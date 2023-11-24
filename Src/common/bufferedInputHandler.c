/**
 * @file bufferedInputHandler.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief handler for processing an array of characters receiver over the API or Console interface
 * @version 0.1
 * @date 2021-12-18
 * 
 * 
 */

#include "bufferedInputHandler.h"
#include "bufferedInputStructs.h"
#include "consoleHandler.h"
#include "apiHandler.h"
#include "consoleBase.h"
#ifndef HARDWARE
#include <stdio.h>
#endif

/**
 * @brief calls onCharacterReception for each element of the input buffer and prints out the answers sequentially
 * *note*: currently the handling procedure is the same for API and Console
 * @param binput the input, can be of type API or Console
 */
void processInputBuffer(BufferedInput binput)
{
	char* returnBfr;
	if (binput->interfaceType == BINPUT_TYPE_CONSOLE)
	{
		while(binput->commBuffer->inputBufferCnt > 0)
		{
			returnBfr = onCharacterReception(binput,binput->commBuffer->inputBuffer[binput->commBuffer->inputBufferCnt-1]);
			binput->commBuffer->inputBufferCnt--;
			appendStringToOutputBuffer((binput->commBuffer),returnBfr);
			//printf(returnBfr);
		}

	} else if (binput->interfaceType == BINPUT_TYPE_API)
	{
		while(binput->commBuffer->inputBufferCnt > 0)
		{
			returnBfr = onByteReception(binput,binput->commBuffer->inputBuffer[binput->commBuffer->inputBufferCnt-1]);
			binput->commBuffer->inputBufferCnt--;
			appendStringToOutputBuffer((binput->commBuffer),returnBfr);
			//printf(returnBfr);
		}
	}
}

void appendToInputBuffer(CommBuffer bfr,uint8_t * data,uint32_t len)
{
	for (uint32_t c=0;c<len;c++)
	{
		bfr->inputBuffer[bfr->inputBufferCnt++]=data[c];
		bfr->inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
	}
}

void appendToInputBufferReverse(CommBuffer bfr,uint8_t * data,uint32_t len)
{
	for (int32_t c=len-1;c>=0;c--)
	{
		bfr->inputBuffer[bfr->inputBufferCnt++]=data[c];
		bfr->inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
	}
}

void appendToOutputBuffer(CommBuffer bfr,uint8_t * data,uint32_t len)
{
	for (uint32_t c=0;c<len;c++)
	{
		bfr->outputBuffer[bfr->outputBufferReadCnt++]=data[c];
		bfr->outputBufferReadCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);
		if(isOutputBufferFull(bfr))
		{
			bfr->bufferConsumer(bfr,1);
		}
	}
}


void appendStringToOutputBuffer(CommBuffer bfr,const char * data)
{
	uint32_t c=0;
	while(data[c]!=0)
	{
		bfr->outputBuffer[bfr->outputBufferReadCnt++]=data[c++];
		bfr->outputBufferReadCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);
		if(isOutputBufferFull(bfr))
		{
			bfr->bufferConsumer(bfr,1);
		}
	}
}


uint32_t getOutputBufferFillLength(CommBuffer bfr)
{
	if (bfr->outputBufferReadCnt >= bfr->outputBufferWriteCnt)
	{
		return bfr->outputBufferReadCnt - bfr->outputBufferWriteCnt;
	}
	else
	{
		return ((1 << OUTPUT_BUFFER_SIZE) - (bfr->outputBufferWriteCnt - bfr->outputBufferReadCnt));
	}
}

void consumeOutputBufferBytes(CommBuffer bfr,uint32_t nbytes)
{
	bfr->outputBufferWriteCnt += nbytes;
	bfr->outputBufferWriteCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);
}

uint8_t isOutputBufferFull(CommBuffer bfr)
{
	uint32_t fl = getOutputBufferFillLength(bfr);
	if (fl >= ((1 << OUTPUT_BUFFER_SIZE)-1))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void getOutputBuffer(CommBuffer bfr,uint32_t* len,uint32_t* offset)
{
	*len = getOutputBufferFillLength(bfr);
	*offset=bfr->outputBufferWriteCnt;
}