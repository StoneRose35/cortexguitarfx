/**
 * @file apiHandler.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief handler for the API or system interface
 * @version 0.1
 * @date 2021-12-18
 * 
 * provides functions to interpret data sent over Uarts without echoing the sent characters and command history
 * intended to use as and API (as the name suggests) with a non-human caller
 * 
 */

#include "system.h"
#include "apiHandler.h"
#include "taskManager.h"
#include "bufferedInputHandler.h"

/**
 * @brief inits the @see Api  structure
 * 
 * @param api the data structure which stores the buffers (input and output)
 */
void initApi(Api api)
{
	api->ibidx = 0;
	for(uint16_t c=0;c<API_INPUT_BUFFER_SIZE;c++)
	{
		api->inputBuffer[c] = 0;
	}
}

/**
 * @brief processes a new byte received
 * 
 * @param binput the input data structure possibly containing characters already received
 * @param charin the new character to handle
 * @return char* 
 * tries to process a command (input buffer content) when the charin is carriage return
 * otherfile put the characted to the input buffer
 * return "1" if the buffer is full and zero (empty string) otherwise
 * 
 */
char* onByteReception(BufferedInput binput,uint8_t charin)
{
	if (charin == 13) // handle command when a newline character has been received
	{
		handleCommand(binput->api->inputBuffer,binput);

		binput->api->ibidx = 0;
		for(uint16_t c=0;c<API_INPUT_BUFFER_SIZE;c++)
		{
			binput->api->inputBuffer[c] = 0;
		}
		binput->api->outputBuffer[0] = 0;
		binput->api->outputBuffer[1] = 0;
	}
	else
	{
		binput->api->inputBuffer[binput->api->ibidx++] = charin;
		if (binput->api->ibidx >= API_INPUT_BUFFER_SIZE)
		{
			binput->api->ibidx = API_INPUT_BUFFER_SIZE-1;
			binput->api->outputBuffer[0] = 0x31;
		}
		else
		{
			binput->api->outputBuffer[0]=0;
		}
	}
	binput->api->outputBuffer[1] = 0;
	return binput->api->outputBuffer;
}

