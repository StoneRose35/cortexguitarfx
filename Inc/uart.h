/*
 * uart.h
 *
 *  Created on: 29.08.2021
 *      Author: philipp
 */

#include "systemChoice.h"

#ifdef HARDWARE
#ifndef UART_H_
#define UART_H_


#include <stdint.h>
#include "hardware/regs/uart.h"
#include "hardware/regs/addressmap.h"

#define BAUD_RATE 57600


#ifdef STM32
typedef volatile uint32_t reg;
typedef struct
{
	reg CR1;
	reg CR2;
	reg CR3;
	reg BRR;
	reg GTPR;
	reg RTOR;
	reg RQR;
	reg ISR;
	reg ICR;
	reg RDR;
	reg TDR;
} UartTypeDef;

#define TXE (7)
#define TC (6)
#define RXNE (5)

#define UART1 ((UartTypeDef*)0x40013800UL)
#define UART2 ((UartTypeDef*)0x40004400UL)
#endif

/**
 * @brief initialize the main uart with a given baud rate
 * 
 * @param baudrate allowed values: 2400, 4800, 9600 19200,28800,57600, 115200
 */
void initUart(uint16_t baudrate);

/**
 * @brief initialize the uart meanth to have a bluetooth interface attached
 * 
 * @param baudrate allowed values: 2400, 4800, 9600 19200,28800,57600, 115200
 */
void initBTUart(uint16_t baudrate);

/**
 * @brief send's one or multiple characters from out output buffer 
 * doesn't wait until the transmission has terminated
 * depending on the hardware one can fill a FIFO or use DMA to send the output buffer
 * the calling duration must be independent of the baud rate
 *  * 
 * @return uint8_t 1 if transmission has terminated (buffer emptied), 0 if buffer is not empty
 */
uint8_t sendCharAsyncUsb();
uint8_t sendCharAsyncBt();


#endif /* UART_H_ */

#endif
