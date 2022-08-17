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
			printf(returnBfr);
		}

	} else if (binput->interfaceType == BINPUT_TYPE_API)
	{
		while(binput->commBuffer->inputBufferCnt > 0)
		{
			returnBfr = onByteReception(binput,binput->commBuffer->inputBuffer[binput->commBuffer->inputBufferCnt-1]);
			binput->commBuffer->inputBufferCnt--;
			printf(returnBfr);
		}
	}

}
