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
