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

typedef void(*returnHandler)(const char*);

void processInputBuffer(BufferedInput);

/** 
 * returns the offset and the length of the non-consumed part of the output buffer
*/
void getOutputBuffer(CommBuffer bfr,uint32_t* len,uint32_t* offset);


void appendToInputBuffer(CommBuffer bfr,uint8_t * data,uint32_t len);
void appendToInputBufferReverse(CommBuffer bfr,uint8_t * data,uint32_t len);
void appendToOutputBuffer(CommBuffer bfr,uint8_t * data,uint32_t len);
void appendStringToOutputBuffer(CommBuffer bfr,const char * data);
uint32_t getOutputBufferFillLength(CommBuffer bfr);
void consumeOutputBufferBytes(CommBuffer bfr,uint32_t nbytes);
uint8_t isOutputBufferFull(CommBuffer bfr);

#endif /* BUFFEREDINPUTHANDLER_H_ */
