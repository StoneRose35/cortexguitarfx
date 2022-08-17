/*
 * uart.c
 *
 *  Created on: 29.08.2021
 *      Author: philipp
 */

#ifdef RP2040_FEATHER

#include <neopixelDriver.h>
#include "uart.h"
#include "system.h"
#include "consoleHandler.h"
#include "bufferedInputHandler.h"

#include "hardware/regs/addressmap.h"
#include"hardware/regs/uart.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/m0plus.h"
#include "hardware/regs/dma.h"
#include "hardware/rp2040_registers.h"


extern uint32_t task;
extern uint8_t context; // used by printf to decide where a certain information should be output

CommBufferType usbCommBuffer __attribute__((aligned (256)));

CommBufferType btCommBuffer;

// receive interrupt for the usb uart
void isr_uart0_irq20()
{
	if ((*UART_UARTMIS & (1 << UART_UARTMIS_RXMIS_LSB)) == (1 << UART_UARTMIS_RXMIS_LSB)) // reception case
	{
		usbCommBuffer.inputBuffer[usbCommBuffer.inputBufferCnt++]=*UART_UARTDR & 0xFF;
		usbCommBuffer.inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
		task |= (1 << TASK_USB_CONSOLE_RX);
	}
}

// receive interrupt for the bluetooth uart
void isr_uart1_irq21()
{
	if ((*UARTBT_UARTMIS & (1 << UART_UARTMIS_RXMIS_LSB)) == (1 << UART_UARTMIS_RXMIS_LSB)) // reception case
	{
		btCommBuffer.inputBuffer[btCommBuffer.inputBufferCnt++]=*UARTBT_UARTDR & 0xFF;
		btCommBuffer.inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
		task |= (1 << TASK_BT_CONSOLE_RX);
	}
}


/**
 * @brief initiates a dma transfer if the ringbuffer is not empty, empty: readCnt/head = writeCnt/tail
 * reads forward from tail to head
 * @return uint8_t 0 if transmission is ongoing, 1 if terminated
 */
uint8_t sendCharAsyncUsb()
{
	if (usbCommBuffer.outputBufferWriteCnt != usbCommBuffer.outputBufferReadCnt 
	&& (*DMA_CH1_CTRL_TRIG & (1 << DMA_CH1_CTRL_TRIG_BUSY_LSB)) != (1 << DMA_CH1_CTRL_TRIG_BUSY_LSB))//((*UART_UARTFR & (1 << UART_UARTFR_BUSY_LSB)) == 0))
	{
		*DMA_CH1_READ_ADDR = (uint32_t)(usbCommBuffer.outputBuffer+usbCommBuffer.outputBufferWriteCnt);
		if (usbCommBuffer.outputBufferReadCnt > usbCommBuffer.outputBufferWriteCnt)
		{
			*DMA_CH1_TRANS_COUNT = usbCommBuffer.outputBufferReadCnt - usbCommBuffer.outputBufferWriteCnt;
		}
		else
		{
			*DMA_CH1_TRANS_COUNT = ((1 << OUTPUT_BUFFER_SIZE) - (usbCommBuffer.outputBufferWriteCnt - usbCommBuffer.outputBufferReadCnt));
		}
		*DMA_CH1_CTRL_TRIG |= (1 << DMA_CH1_CTRL_TRIG_EN_LSB);
		usbCommBuffer.outputBufferWriteCnt = usbCommBuffer.outputBufferReadCnt;
		return 0;
	}
	else if ((*DMA_CH1_CTRL_TRIG & (1 << DMA_CH1_CTRL_TRIG_BUSY_LSB)) != (1 << DMA_CH1_CTRL_TRIG_BUSY_LSB))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t sendCharAsyncBt()
{
	if (btCommBuffer.outputBufferWriteCnt < btCommBuffer.outputBufferReadCnt && ((*UARTBT_UARTRIS & (1 << UART_UARTRIS_TXRIS_LSB)) == (1 << UART_UARTRIS_TXRIS_LSB)))
	{
		*UARTBT_UARTDR = *(btCommBuffer.outputBuffer+btCommBuffer.outputBufferWriteCnt);
		btCommBuffer.outputBufferWriteCnt++;
	}
	if (btCommBuffer.outputBufferWriteCnt == btCommBuffer.outputBufferReadCnt)
	{
		btCommBuffer.outputBufferWriteCnt=0;
		btCommBuffer.outputBufferReadCnt=0;
		return 1;
	}
	else
	{
		return 0;
	}
}


/* USB Uart, used for serial communication over usb
 * 
 * */
void initUart(uint16_t baudrate)
{
	// power on uart, first switch it off explicitely
	*RESETS |= (1 << RESETS_RESET_UART0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_UART0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_UART0_LSB)) == 0);


	// wire up the GPIO to UART
	*UART_RX_PIN_CNTR = 2;
	*UART_TX_PIN_CNTR = 2;

	// enable interrupt on receive
	*UART_UARTIMSC |= (1 << UART_UARTIMSC_RXIM_LSB);

	// clock is 132MHz / (16*57600)
	// resulting in 143.2291666, becoming 143 and floor(0.2291666*64)=14

	// set the baud rate
	*UART_UARTIBRD = (uint32_t)(F_SYS/(16*BAUD_RATE)); // 130 with F_SYS at 120 MHz
	*UART_UARTFBRD = (uint32_t)(64*(F_SYS/(16.0*BAUD_RATE) - (uint32_t)(F_SYS/(16.0*BAUD_RATE))) + 0.5); // 13 with F_SYS at 120 MHz

	// set word length to 8 bits
	*UART_UARTLCR_H |= (3 << UART_UARTLCR_H_WLEN_LSB);

	// enable receive and transmit
	*UART_UARTCR |= (1 << UART_UARTCR_RXE_LSB) | (1 << UART_UARTCR_TXE_LSB) | (1 << UART_UARTCR_UARTEN_LSB);

	// enable interrupts in the nvic
	*NVIC_ISER = (1 << 20);

	// enable trasmit dma 
	*UART_UARTDMACR = (1 << UART_UARTDMACR_TXDMAE_LSB);

	// setup dma, write to the uart data register
	*DMA_CH1_WRITE_ADDR = (uint32_t)UART_UARTDR;

	// increase the read address, set data size to 8 bit, data request to uart0 tx
	// set ring size to output buffer size, enable ring buffer for read addresses
	*DMA_CH1_CTRL_TRIG |= (1 << DMA_CH1_CTRL_TRIG_INCR_READ_LSB) 
						| (20 << DMA_CH1_CTRL_TRIG_TREQ_SEL_LSB) 
						| (0 << DMA_CH1_CTRL_TRIG_DATA_SIZE_LSB)
						| (OUTPUT_BUFFER_SIZE << DMA_CH1_CTRL_TRIG_RING_SIZE_LSB)
						| (0 << DMA_CH1_CTRL_TRIG_RING_SEL_LSB)
						;

	// enable interrupt of channel 1 to inte0
	*DMA_INTE0 |= (1 << 1);

}

/*
 * Uart connected to the bluetooth module
 * 
 * */
void initBTUart(uint16_t baudrate)
{
	// power on uart
	*RESETS |= (1 << RESETS_RESET_UART1_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_UART1_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_UART1_LSB)) == 0)


	// wire up the GPIO to UART
	*UARTBT_RX_PIN_CNTR = 2;
	*UARTBT_TX_PIN_CNTR = 2;

	// enable interrupt on receive
	*UARTBT_UARTIMSC |= (1 << UART_UARTIMSC_RXIM_LSB);

	// set the baud rate
	*UARTBT_UARTIBRD = (uint32_t)(F_SYS/(16*BAUD_RATE)); // 130 with F_SYS at 120 MHz
	*UARTBT_UARTFBRD = (uint32_t)(64*(F_SYS/(16.0*BAUD_RATE) - (uint32_t)(F_SYS/(16.0*BAUD_RATE))) + 0.5); // 13 with F_SYS at 120 MHz

	// set word length to 8 bits
	*UARTBT_UARTLCR_H |= (3 << UART_UARTLCR_H_WLEN_LSB);

	// enable receive and transmit
	*UARTBT_UARTCR |= (1 << UART_UARTCR_RXE_LSB) | (1 << UART_UARTCR_TXE_LSB) | (1 << UART_UARTCR_UARTEN_LSB);

	// enable interrupts in the nvic
	*NVIC_ISER = (1 << 21);
}


#endif
