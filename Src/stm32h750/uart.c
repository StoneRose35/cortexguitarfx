#include "stdint.h"
#include "uart.h"
#include "stm32h750/stm32h750xx.h"
#include "bufferedInputHandler.h"
#include "system.h"

#define APB1_SPEED 45000000


extern uint32_t task;
extern uint8_t context; // used by printf to decide where a certain information should be output

CommBufferType usbCommBuffer __attribute__((aligned (256)));

CommBufferType btCommBuffer;

// irq for uart reception, "USB"-port
void USART3_IRQHandler()
{
    if ((USART3->SR & (1 <<USART_SR_RXNE_Pos))== (1 <<USART_SR_RXNE_Pos))
    {
		usbCommBuffer.inputBuffer[usbCommBuffer.inputBufferCnt++]=USART3->DR& 0xFF;
		usbCommBuffer.inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
		task |= (1 << TASK_USB_CONSOLE_RX);
    }
}

// irq for uart reception, optional Bluetooth port

//setup
void initUart(uint16_t baudrate)
{
    uint32_t divider;
    uint32_t regbfr;
    uint32_t baudrateWord;
    baudrateWord=baudrate;
    // enable clock
    RCC->APB1ENR |= (1 << RCC_APB1ENR_USART3EN_Pos);

    // define baudrate
    divider = APB1_SPEED/baudrateWord; 
    USART3->BRR = divider & 0xFFFF;

    // enable usart, receiver and transmitter and receiver not empty interrupt
    USART3->CR1 = (1 << USART_CR1_UE_Pos) | (1 << USART_CR1_TE_Pos) | (1 << USART_CR1_RE_Pos) | (1 << USART_CR1_RXNEIE_Pos);

    // enable the usart3 interrupt
    __NVIC_EnableIRQ(USART3_IRQn);

    // wire up pd8 (uart3 tx) and pd9 (uart3 rx)
    RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIODEN_Pos);
    regbfr = GPIOD->MODER;
    regbfr &= ~((3 << (8*2)) | (3 << (9*2)));
    regbfr |= (2 << (8*2)) | (2 << (9*2));
    GPIOD->MODER = regbfr;

    GPIOD->PUPDR &= ~((3 << (8*2)) | (3 << (9*2)));
    regbfr = GPIOD->AFR[1];
    regbfr &= ~((0xF << 0) | (0xF << 4));
    regbfr |= ((7 << 0) | (7 << 4));
    GPIOD->AFR[1] = regbfr; // define alternate funtion 7 for pin 8 and 9 
}

void initBTUart(uint16_t baudrate)
{

}

/**
 * @brief send's one or multiple characters from out output buffer 
 * doesn't wait until the transmission has terminated
 * depending on the hardware one can fill a FIFO or use DMA to send the output buffer
 * the calling duration must be independent of the baud rate
 *  * 
 * @return uint8_t 1 if transmission has terminated (buffer emptied), 0 if buffer is not empty
 */
uint8_t sendCharAsyncUsb()
{
	if (usbCommBuffer.outputBufferWriteCnt != usbCommBuffer.outputBufferReadCnt && ((USART3->SR & USART_SR_TXE)== USART_SR_TXE))
	{
		USART3->DR = *(usbCommBuffer.outputBuffer+usbCommBuffer.outputBufferWriteCnt);
		usbCommBuffer.outputBufferWriteCnt++;
        usbCommBuffer.outputBufferWriteCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);
        return 0;
	}
	if (usbCommBuffer.outputBufferWriteCnt == usbCommBuffer.outputBufferReadCnt)
	{
		return 1;
	}
    return 0;
}

uint8_t sendCharAsyncBt()
{
    return 1;
}
