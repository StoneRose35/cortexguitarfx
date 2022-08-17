/**
 * @file consoleHandler.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief a minimalistic implementation of a command shell including a 4 line command history and the standard inline editing
 * capabilities known from bash. Doesn't support autocompletion.
 * @version 0.1
 * @date 2021-12-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdint.h>
#include "consoleHandler.h"
#include <string.h>
#include "taskManager.h"
#include "system.h"

static char currentPath[CONSOLE_MAX_PATHLENGTH];
static uint16_t currentPathCnt = 0;

void addToPath(char * folder)
{
	uint8_t c=0;
	if(currentPathCnt != 1)
	{
		currentPath[currentPathCnt++] = '/';
	}
	while(*((uint8_t*)folder + c)!=0 && *((uint8_t*)folder + c)!=' ' && c<8)
	{
		currentPath[currentPathCnt++] = *(folder + c);
		c++;
	}
	currentPath[currentPathCnt]=0;
}

void removeLastPath()
{
	uint16_t c=currentPathCnt-1;
	while (currentPath[c]!= '/')
	{
		currentPath[c--] = 0;
	}
	if (c==0) // keep the initial slash
	{
		currentPathCnt=1;
	}
	else
	{
		currentPath[c]=0;
		currentPathCnt=c;
	}
}

/**
 * @brief initializes all buffers with zeros
 * 
 * @param console the console data structure to initialize
 */
void initConsole(Console console)
{
	uint16_t cnt;

	for (cnt=0;cnt<COMMAND_BUFFER_SIZE*COMMAND_HISTORY_SIZE;cnt++)
	{
		console->commandBuffer[cnt]=0;
		console->commandBufferShadow[cnt]=0;
	}
	for (cnt=0;cnt<OUT_BUFFER_SIZE;cnt++)
	{
		console->outBfr[cnt]=0;
	}
	console->cmdBfr[0]=0;
	console->cmdBfr[1]=0;
	console->cmdBfr[2]=0;
	console->cbfCnt=0;
	console->cbfIdx=0;
	console->cursor=0;
	console->mode=0;

	for(cnt=0;cnt<CONSOLE_MAX_PATHLENGTH;cnt++)
	{
		currentPath[cnt]=0;
	}
	currentPathCnt = 0;
}

const char * consolePrefix = CONSOLE_PREFIX; //!< command line prefix, every shell line starts with that
const char * cmd_arrow_left = "[D"; //!< special character sequence for "arrow left"
const char * cmd_arrow_right = "[C"; //!< special character sequence for "arrow right"
const char * cmd_arrow_up = "[A"; //!< special character sequence for "arrow up"
const char * cmd_arrow_down = "[B"; //!< special character sequence for "arrow down"

/**
 * @brief defines how to handle a single character input into the command shell. 
 * * Normals Characters: Echos "normal" characters so that we see what we're typing into the console
 * * Arrow Up/Down: browse through command history (if filled) 
 * * Arrow Left/Right: move cursor within the currenty entered command 
 * * Del/Backspace: deletes parts of the currently entered content depending on the cursor position
 * @param binput the input to which the entered character belongs
 * @param charin the character input
 * @return char* what should be written back to the user a.k.a. the output of the command shell
 */
char* onCharacterReception(BufferedInput binput,uint8_t charin)
{
	uint8_t c1=0,c2=1;
	uint16_t obCnt=0;

    clearOutBuffer(binput->console);
	if (charin == 13 && binput->console->mode == 0) // convert \r into \r\n, print a new console line
	{

		// copy the possibly edited command into the first position of the shadow command buffer
		copyCommandBetweenArrays(binput->console->cbfIdx,0,binput->console->commandBuffer,binput->console->commandBufferShadow);

		// push command into history
		for(uint8_t c=COMMAND_HISTORY_SIZE-1;c>0;c--)
		{
			copyCommand(c-1,c,binput->console->commandBufferShadow);
		}

		binput->console->cbfIdx=0;

		handleCommand(binput->console->commandBufferShadow,binput);

		binput->console->outBfr[obCnt++] = 13;
		binput->console->outBfr[obCnt++] = 10;

		while(consolePrefix[c1] != 0)
		{
			binput->console->outBfr[obCnt]=consolePrefix[c1];
			c1++;
			obCnt++;
		}
		c2=0;
		if (currentPath[c2] == '/') // filesystem mounted
		{
			while(currentPath[c2] != 0)
			{
				binput->console->outBfr[obCnt++]=currentPath[c2++];
			}
			binput->console->outBfr[obCnt++] = '$';
		}
		else
		{
			binput->console->outBfr[obCnt++] = '>';
		}
		binput->console->outBfr[obCnt] = 0;

		clearCommandBuffer(binput->console,binput->console->cbfIdx,binput->console->commandBufferShadow);

		// copy shadow into edit buffer
		for (uint16_t cc=0;cc<COMMAND_BUFFER_SIZE*COMMAND_HISTORY_SIZE;cc++)
		{
			*(binput->console->commandBuffer+cc) = *(binput->console->commandBufferShadow+cc);
		}

		binput->console->cursor = 0;

	}



	else if ((charin == 0x7F || charin == 0x8) && binput->console->cbfCnt>0 && binput->console->mode == 0 && binput->console->cursor > 0) // DEL/backspace
	{
        if (binput->console->cursor < binput->console->cbfCnt) // within the command
        {
        	uint8_t swap, backcnt=0;
        	swap=binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor];
        	binput->console->outBfr[obCnt++] = 27;
        	binput->console->outBfr[obCnt++] = 91;
        	binput->console->outBfr[obCnt++] = 68;
        	while (swap != 0)
        	{
        		binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor-1] = swap;
        		binput->console->outBfr[obCnt++] = swap;
        		binput->console->cursor++;
        		backcnt++;
        		swap=binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor];
        	}
        	binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor-1]=0;
        	binput->console->outBfr[obCnt++]=' ';
        	binput->console->cursor++;
        	backcnt++;
        	for (uint8_t q=0;q<backcnt;q++)
        	{
        		binput->console->outBfr[obCnt++] = 27;
        		binput->console->outBfr[obCnt++] = 91;
        		binput->console->outBfr[obCnt++] = 68;
        		binput->console->cursor--;
        	}
        	binput->console->cbfCnt--;
        	binput->console->cursor--;
        }
        else // at the right end (default after typing)
        {
        	binput->console->outBfr[obCnt++] = 0x8;
        	binput->console->outBfr[obCnt++] = 0x20;
        	binput->console->outBfr[obCnt++] = 0x8;
        	binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt--] = 0;
        	binput->console->cursor--;
        }
	}
	else if (charin < 32) // control character received
	{
		binput->console->mode = 1;
		binput->console->cmdBfr[0]=0;
		binput->console->cmdBfr[1]=0;
		binput->console->cmdBfr[2]=0;
	}
	else if (binput->console->mode==1) // first describing character after the control character
	{
		binput->console->cmdBfr[binput->console->mode-1] = charin;
		binput->console->mode++;
	}
	else if (binput->console->mode==2) 		// command mode, arrows behave rougly the same as in bash
	{
		binput->console->cmdBfr[binput->console->mode-1] = charin;
		binput->console->mode = 0;
		if (strcmp(binput->console->cmdBfr,cmd_arrow_left)==0)
		{
			if (binput->console->cursor>0)
			{
				binput->console->cursor--;
				binput->console->outBfr[obCnt++] = 27;
				binput->console->outBfr[obCnt++] = 91;
				binput->console->outBfr[obCnt++] = 68;
			}
		}
		if (strcmp(binput->console->cmdBfr,cmd_arrow_right)==0)
		{
			if (binput->console->cursor<binput->console->cbfCnt)
			{
				binput->console->cursor++;
				binput->console->outBfr[obCnt++] = 27;
				binput->console->outBfr[obCnt++] = 91;
				binput->console->outBfr[obCnt++] = 67;
			}
		}
		if (strcmp(binput->console->cmdBfr,cmd_arrow_up)==0)
		{
			// remove old command
			while (binput->console->cbfCnt>0)
			{
				binput->console->outBfr[obCnt++] = 0x8;
				binput->console->outBfr[obCnt++] = 0x20;
				binput->console->outBfr[obCnt++] = 0x8;
				binput->console->cbfCnt--;
			}
			if (binput->console->cbfIdx < COMMAND_HISTORY_SIZE-1)
			{
				binput->console->cbfIdx++;
				binput->console->cbfCnt=0;
				binput->console->cursor=0;
				while (binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt] != 0)
				{
					binput->console->outBfr[obCnt++] = binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt++];
					binput->console->cursor++;
				}
			}
		}
		if (strcmp(binput->console->cmdBfr,cmd_arrow_down)==0)
		{
			// remove old command
			while (binput->console->cbfCnt>0)
			{
				binput->console->outBfr[obCnt++] = 0x8;
				binput->console->outBfr[obCnt++] = 0x20;
				binput->console->outBfr[obCnt++] = 0x8;
				binput->console->cbfCnt--;
			}
			if (binput->console->cbfIdx >0)
			{
				binput->console->cbfIdx--;
				binput->console->cbfCnt=0;
				binput->console->cursor=0;
				while (binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt] != 0)
				{
					binput->console->outBfr[obCnt++] = binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt++];
					binput->console->cursor++;
				}
			}
		}
	}
	else if (charin < 127 && binput->console->cbfCnt < COMMAND_BUFFER_SIZE-1) // "normal" (non-control) character nor del or special characters entered
	{
		if (binput->console->cursor < binput->console->cbfCnt) // within the command
		{
			char swap, swap2, backcnt=0;
			swap = binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor];
			binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor++]=charin;
			binput->console->outBfr[obCnt++]=charin;
			while (swap != 0)
			{
				swap2 = binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor];
				binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cursor++]= swap;
				binput->console->outBfr[obCnt++]=swap;
				swap=swap2;
				backcnt++;
			}
           for (uint8_t q =0;q<backcnt;q++)
           {
        	   binput->console->outBfr[obCnt++] = 27;
        	   binput->console->outBfr[obCnt++] = 91;
        	   binput->console->outBfr[obCnt++] = 68;
        	   binput->console->cursor--;
           }
           binput->console->cbfCnt++;
		}
		else
		{
			binput->console->commandBuffer[binput->console->cbfIdx*COMMAND_BUFFER_SIZE + binput->console->cbfCnt++] = charin;
			binput->console->outBfr[obCnt++] = charin;
			binput->console->cursor++;
		}
	}
	binput->console->outBfr[obCnt++] = 0;
	return binput->console->outBfr;
}

/**
 * @brief empties the command buffer holding the currenty command being typed in
 * 
 * @param console this console's cbfCnt is reset
 * @param idx position within the command history
 * @param cmdBfr the command holding the entire command history
 */
void clearCommandBuffer(Console console,uint8_t idx,char* cmdBfr)
{
	for(uint16_t cnt=0; cnt<COMMAND_BUFFER_SIZE; cnt++)
	{
		*(cmdBfr + idx*COMMAND_BUFFER_SIZE + cnt) = 0;
	}
	console->cbfCnt=0;
}

/**
 * @brief clear the entire output buffer 
 * 
 * @param console 
 */
void clearOutBuffer(Console console)
{
	for(uint16_t cnt=0; cnt<OUT_BUFFER_SIZE; cnt++)
	{
		console->outBfr[cnt] = 0;
	}
}


/**
 * @brief copies a command within the command Buffer
 * 
 * @param idxSrc the byte index of the source command position
 * @param idxTarget the byte index of the target position
 * @param cmdBfr the command buffer which should be manipulated
 */
void copyCommand(uint8_t idxSrc,uint8_t idxTarget,char* cmdBfr)
{
	for (uint16_t c=0;c<COMMAND_BUFFER_SIZE;c++)
	{
		*(cmdBfr + idxTarget*COMMAND_BUFFER_SIZE + c) = *(cmdBfr + idxSrc*COMMAND_BUFFER_SIZE + c);
	}
}

/**
 * @brief copies a command from one command buffer to another
 * 
 * @param idxSrc the byte index of the source command within cmdBfrSrc
 * @param idxTarget the byte index of the target command within cmdBfrTarget
 * @param cmdBfrSrc the command buffer from which the command should be copied
 * @param cmdBfrTarget the command buffer to the the command should be copied to
 */
void copyCommandBetweenArrays(uint8_t idxSrc,uint8_t idxTarget,char* cmdBfrSrc,char* cmdBfrTarget)
{
	for (uint16_t c=0;c<COMMAND_BUFFER_SIZE;c++)
	{
		*(cmdBfrTarget + idxTarget*COMMAND_BUFFER_SIZE + c) = *(cmdBfrSrc + idxSrc*COMMAND_BUFFER_SIZE + c);
	}
}

