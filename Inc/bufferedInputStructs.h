/**
 * @file bufferedInputStructs.h
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief functions a data structure for handling the command line (CLI) and and AP interface (API)
 * @version 0.1
 * @date 2021-12-22
 * This header defines the "power" of the interface in terms of maximum command length possible and number of commands stored
 * adjust these values for future applications depending on the RAM available and application needs
 * @copyright Copyright (c) 2021
 * 
 */
#include "systemChoice.h"

#ifndef BUFFEREDINPUTSTRUCTS_H_
#define BUFFEREDINPUTSTRUCTS_H_

#ifndef HARDWARE
#include <stdint.h>
#endif

#define COMMAND_BUFFER_SIZE 48 //!< the size of the input command buffer thus the maximum command length. The byte size of the input buffer is COMMAND_BUFFER_SIZE*COMMAND_HISTORY_SIZE
#define COMMAND_HISTORY_SIZE 4 //!< the maximum number of commands remembered
#define OUT_BUFFER_SIZE 256 //!< the size of the command line output buffer, holding the characters which should be printed back at once after a character is entered
                            
#define API_INPUT_BUFFER_SIZE 256 //!< the size of the input buffer of the api, adjust to the maximum command length
#define API_OUTPUT_BUFFER_SIZE 8 //!< size of the output api buffer

#define INPUT_BUFFER_SIZE 8 //!< the size of the input buffer of the api, adjust to the maximum command length, is filled by the uart eception interrupt handlers and emptied by onCharReception
#define OUTPUT_BUFFER_SIZE 8 //!< size of the output buffer defined as a power of two, filled by printf and emptied by sendCharAsyncUsb or sendCharAsyncBt 

/**
 * @brief structre containing the hardware buffer for the API
 * 
 */
typedef struct {

	char inputBuffer[API_INPUT_BUFFER_SIZE]; //!< byte input buffer, is filled by the uart reception interrupt handlers and emptied by onByteReception
	char outputBuffer[API_OUTPUT_BUFFER_SIZE]; //!< byte output buffer, filled by printf and emptied by sendCharAsyncUsb or sendCharAsyncBt 
	uint16_t ibidx; //!< the current fill level of the input buffer
} ApiType;

/**
 * @brief pointer to ApiType, defined for convenience
 * 
 */
typedef ApiType* Api;


/**
 * @brief data structure for the CLI
 * contains two buffers for the command lines entered providing the command history functionality
 * plus one small buffer of handling two-byte commands, such as the arrow keys
 */
typedef struct {
	char commandBuffer[COMMAND_BUFFER_SIZE*COMMAND_HISTORY_SIZE]; //!< the main command buffer
	char commandBufferShadow[COMMAND_BUFFER_SIZE*COMMAND_HISTORY_SIZE]; //!< a shadow command buffer allowing to edit commands in history which are reverted to their old state once a new command is executed
	char outBfr[OUT_BUFFER_SIZE]; //!< the buffer containing the cursor commands and characters echoed of the command line
	char cmdBfr[3]; //!< buffer for handling two-byte commands (arrow keys) in a non-blocking fashion
	uint8_t cbfCnt; //!< counter for the number of bytes entered at a certain certain index
	uint8_t cbfIdx; //!< hold the current index within the command buffer, can go from 0 to COMMAND_HISTORY_SIZE - 1 
	uint8_t cursor; //!< the position of the cursor
	uint8_t mode; //!< state flag for handling multi-byte keystrokes (such as the arrow keys)
} ConsoleType;

/**
 * @brief pointer to ConsoleType, defined for convenience
 * 
 */
typedef ConsoleType* Console;

/**
 * @brief data structure for a byte-oriented hardware communication interface (typically UART)
 * 
 */
typedef struct
{
	char outputBuffer[(1 << OUTPUT_BUFFER_SIZE)]; //!< output buffer, filled by the system, emptied by calling sendCharAsync
	char inputBuffer[INPUT_BUFFER_SIZE]; //!< input buffer, filled by the interrupt handler, emptied by calling processInputBuffer
	uint32_t outputBufferReadCnt; //!< counts how many bytes have been read by the output buffer, is increased by the system
	uint32_t outputBufferWriteCnt; //!< counts how many bytes have been written out the the interface, is increased by sendCharAsync
	uint8_t inputBufferCnt; //!< counts the number of bytes in the input buffer
} CommBufferType;

/**
 * @brief pointer to CommBufferType, defined fro convenience
 * 
 */
typedef CommBufferType* CommBuffer;

/**
 * @brief data structure for an entire communication interface
 * Allows mode switching from Console/CLI to system/API
 */
typedef struct
{
	uint8_t interfaceType; //!< defines the interface type active, can be either BINPUT_TYPE_CONSOLE or BINPUT_TYPE_API
	CommBuffer commBuffer; //!< hardware communicatiion buffer
	Console console; //!< data structure for console/CLI, can be null of only the API is used
	Api api; //!< data structure for the API, can be null if only the CLI is used
} BufferedInputType;

/**
 * @brief pointer to BufferedInputType, defined for convenience
 * 
 */
typedef BufferedInputType* BufferedInput;

#endif /* BUFFEREDINPUTSTRUCTS_H_ */
