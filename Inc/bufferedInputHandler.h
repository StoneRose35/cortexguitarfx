/*
 * byteInterfaceHandler.h
 *
 *  Created on: 02.11.2021
 *      Author: philipp
 */

#ifndef BUFFEREDINPUTHANDLER_H_
#define BUFFEREDINPUTHANDLER_H_

#include <stdint.h>
#include "bufferedInputStructs.h"
#include "uart.h"


#define BINPUT_TYPE_CONSOLE 0
#define BINPUT_TYPE_API 1



void processInputBuffer(BufferedInput);

#endif /* BUFFEREDINPUTHANDLER_H_ */
