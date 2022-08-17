

#include "systemChoice.h"
#include "taskManager.h"
#include "system.h"
#include "memoryAccess.h"
#include "flash.h"
#ifdef HARDWARE
#include "uart.h"
#include "systick.h"
register void* address __asm__("sp");
#else
#include <stdio.h>
void* address;
#endif
#include "stringFunctions.h"
#include "interpolators.h"
#include "taskManagerUtils.h"
#include <bufferedInputHandler.h>
#include "bluetoothATConfig.h"
#include "neopixelDriver.h"
#include "neopixelCommands.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

/**
 * @brief array of 16 standard colors used for the color command
 * 
 */
RGB colors[] = {
		{.r=0,.g=0,.b=0}, // background
		{.r=255,.g=255,.b=255}, // foreground
		{.r=250,.g=60,.b=60}, // 2 red
		{.r=0,.g=220,.b=0}, // 3 green

		{.r=30,.g=60,.b=255}, // 4 dark bue
		{.r=0,.g=200,.b=200}, // 5 light blue
		{.r=240,.g=0,.b=130}, // 6 magenta
		{.r=230,.g=220,.b=50}, // 7 yellow

		{.r=240,.g=130,.b=40}, // 8 orange
		{.r=160,.g=0,.b=200}, // 9 purple
		{.r=160,.g=230,.b=50}, // 10 yellow/green
		{.r=0,.g=160,.b=255}, // 11 medium blue

		{.r=230,.g=175,.b=45}, // 12 dark yellow
		{.r=0,.g=210,.b=140}, // 13 aqua
		{.r=130,.g=0,.b=220}, // 14 dark purple
		{.r=170,.g=170,.b=170} // 15 gray
};

/**
 * @brief command name of the color commands
 * 
 */
const char * colorCommands[N_COLOR_COMMANDS] = {
		"BACKGROUND",
		"FOREGROUND",
		"RED",
		"GREEN",
		"DARKBLUE",
		"LIGHTBLUE",
		"MAGENTA",
		"YELLOW",
		"ORANGE",
		"PURPLE",
		"YELLOWGREEN",
		"MEDIUMBLUE",
		"DARKYELLOW",
		"AQUA",
		"DARKPURPLE",
		"GRAY"};

/**
 * @brief implementation of the command which sets the color of a set of neopixel colors to a preset color
 * 
 * @param cmd the command itself, Example AQUA(0-4,10)
 * @param context the RGBStream containing the structured color information
 */
void colorCommand(char * cmd,void * context)
{
	char bracketContent[128];
	uint8_t nLamps;
	uint8_t lampnrs[N_LAMPS];
	uint8_t has_errors = 0;

	uint8_t cnt=0,idx=0xFF;
	RGBStream* lamps =(RGBStream*)context;

	for (cnt=0;cnt<N_COLOR_COMMANDS;cnt++)
	{
		if (startsWith(cmd,(char*)colorCommands[cnt]) > 0)
		{
			idx = cnt;
		}
	}
	getBracketContent(cmd,bracketContent);
	if (bracketContent != 0 && idx != 0xFF)
	{
		nLamps = expandLampDescription(bracketContent,lampnrs);
		for (uint8_t c=0;c<nLamps;c++)
		{
			if (checkLampRange(*(lampnrs+c),&has_errors) == 1)
			{
				handleRgbStruct((colors+idx),*(lampnrs+c),lamps);
			}
		}
	}
	else
	{
		printf("ERROR: no content within brackets \"()\" found\r\n");
	}
}

/**
 * @brief set a set of neopixel to a custom rgb value
 * @param cmd The command including argument, Example RGB(110,0,110,1,3,5-7) set the neopixels 1,3,5,6,7
 * to a dim violet.
 * @param context the RGBStream containing the structured color information
 */
void rgbCommand(char* cmd,void* context)
{
	char * clr;
	const char comma[2]=",";
	uint8_t r,g,b;
	uint8_t has_errors = 0;
	char bracketContent[128];
	uint8_t lampnrs[N_LAMPS];
	uint8_t nLamps;
	RGBStream* lamps =(RGBStream*)context;
	getBracketContent(cmd,bracketContent);
	if (bracketContent != 0)
	{
		clr = strtok(bracketContent,comma);
		r = tryToUInt8(clr,&has_errors);
		clr = strtok(0,comma);
		g = tryToUInt8(clr,&has_errors);
		clr = strtok(0,comma);
		b = tryToUInt8(clr,&has_errors);
		if (has_errors == 1)
		{
			printf("ERROR: invalid number of arguments\r\n");
		}
		clr = strtok(0,"");

		if (clr != 0 && has_errors == 0)
		{
			nLamps = expandLampDescription(clr,lampnrs);
			for (uint8_t c=0;c<nLamps;c++)
			{
				if (checkLampRange(*(lampnrs+c),&has_errors)==1)
				{
					handleRgb(r,g,b,*(lampnrs+c),lamps);
				}
			}
		}
		else
		{
			printf("ERROR: invalid number of arguments\r\n");
		}
	}
	else
	{
		printf("ERROR: no content within brackets \"()\" found\r\n");
	}
}


/**
 * @brief Starts the neopixel animation, if possible
 * 
 * @param cmd the command, doesn't taken any arguments, therefore unused here.
 * @param context the Tasks objects
 */
void startCommand(char * cmd,void* context)
{
	uint8_t retcode;
	Tasks interpolators=(Tasks)context;
	retcode = startInterpolators(interpolators);
	if (retcode > 0)
	{
		printf("Error while starting the animation\r\n");
	}
}

/**
 * @brief Stops the neopixel animation, if possible
 * 
 * @param cmd the command, doesn't taken any arguments, therefore unused here.
 * @param context the Tasks objects
 */
void stopCommand(char * cmd,void* context)
{
	uint8_t retcode;
	Tasks interpolators=(Tasks)context;
	retcode = stopInterpolators(interpolators);
	if (retcode > 0)
	{
		printf("Error while stopping the animation\r\n");
	}
}

void printColorCommands()
{
    for(uint8_t c=0;c<N_COLOR_COMMANDS;c++)
	{
		printf(" * ");
		printf(colorCommands[c]);
		printf("(<LedNumbers>)\r\n");
	}
}

/**
 * @brief defines an interpolator/color sequence for a given neopixel 255 means every neopixel. This function doesn't take ranges
 * as an argument to select a set of neopixels.
 * 
 * @param cmd the command itself including arguments.
 * @param context the Tasks data structures holding all individual neopixel Task data structures 
 */
void interpCommand(char * cmd,void* context)
{
	char * var;
	uint8_t lampnr;
	uint8_t nsteps;
	uint8_t repeating, has_errors = 0, retval;
	char bracketContent[128];
	char nrbfr[4];
	Tasks interpolators=(Tasks)context;
	getBracketContent(cmd,bracketContent);
	if (bracketContent != 0)
	{
		var = strtok(bracketContent,",");
		lampnr = tryToUInt8(var,&has_errors);
		if (has_errors == 0)
		{
			checkLampRange(lampnr,&has_errors);
		}
		var = strtok(0,",");
		nsteps = tryToUInt8(var,&has_errors);
		var = strtok(0,",");
		repeating = tryToUInt8(var,&has_errors);
		if (repeating > 1)
		{
			printf("ERROR: repeating has to be either 0 or 1\r\n");
			has_errors = 1;
		}
		if (repeating == 1 && nsteps == 1)
		{
			printf("ERROR: a repeating color sequence with only one color\r\n");
			printf("       is meaningless, use RGB for setting a constant color\r\n");
			printf("       or set <repeating> to 0\r\n");
			has_errors = 1;
		}
		if (has_errors == 0)
		{
			if (lampnr < 0xFF)
			{
				retval = setLampInterpolator(interpolators,lampnr,nsteps,repeating);
			}
			else
			{
				retval=0;
				for (uint8_t cc=0;cc<N_LAMPS;cc++)
				{
					retval += setLampInterpolator(interpolators,cc,nsteps,repeating);
				}
			}
			if (retval != 0)
			{
				printf("ERROR: returned error code ");
				UInt8ToChar(retval, nrbfr);
				printf(nrbfr);
				printf(" from setLampInterpolator\r\n");
			}
		}
	}
	else
	{
		printf("ERROR: no content within brackets \"()\" found\r\n");
	}
}

/**
 * @brief defines a certain step for a color sequence/interpolator
 * 
 * @param cmd the command itself including arguments.
 * @param context the Tasks data structures holding all individual neopixel Task data structures 
 */
void istepCommand(char * cmd,void* context)
{
	char * var;
	uint8_t r,g,b;
	uint32_t frames;
	uint8_t interpolation;
	uint8_t lampnr;
	uint8_t step, retval;
	uint8_t has_errors = 0;
	char bracketContent[128];
	char nrbfr[4];
	getBracketContent(cmd,bracketContent);
	Tasks interpolators=(Tasks)context;
	if (bracketContent != 0)
	{
		var = strtok(bracketContent,",");
		r = tryToUInt8(var,&has_errors);
		var = strtok(0,",");
		g = tryToUInt8(var,&has_errors);
		var = strtok(0,",");
		b = tryToUInt8(var,&has_errors);
		var = strtok(0,",");
		frames = tryToUInt32(var,&has_errors);
		var = strtok(0,",");
		interpolation = tryToUInt8(var,&has_errors);

		if (interpolation > 1)
		{
			printf("ERROR: interpolation must be either 0 for constant or 1 for linear\r\n");
			has_errors = 1;
		}

		var = strtok(0,",");
		lampnr = tryToUInt8(var,&has_errors);
		checkLampRange(lampnr,&has_errors);
		var = strtok(0,",");
		step = tryToUInt8(var,&has_errors);

		if (has_errors == 0)
		{
			if (lampnr < 0xFF)
			{
				retval = setColorFramesInterpolation(interpolators,r,g,b,frames,interpolation,lampnr,step);
			}
			else
			{
				retval=0;
				for(uint8_t cc=0;cc<N_LAMPS;cc++)
				{
					retval += setColorFramesInterpolation(interpolators,r,g,b,frames,interpolation,cc,step);
				}
			}
			if (retval != 0)
			{
				printf("ERROR: returned error code ");
				UInt8ToChar(retval, nrbfr);
				printf(nrbfr);
				printf(" from setColorFramesInterpolation\r\n");
			}
		}
	}
	else
	{
		printf("ERROR: no content within brackets \"()\" found\r\n");
	}

}

/**
 * @brief shows the state of all color sequences including their progression
 * 
 * @param cmd the command, doesn't taken any arguments, therefore unused here.
 * @param context the Tasks data structure
 */
void desciCommand(char * cmd,void* context)
{
	char nrbfr[8];
	Tasks interpolators=(Tasks)context;
	printf("\r\n");
	for(uint8_t c=0;c<interpolators->taskArrayLength;c++)
	{
		if (interpolators->taskArray[c].steps != 0)
		{
			printf("Color-Sequence ");
			UInt8ToChar(c,nrbfr);
			printf(nrbfr);
			printf("\r\n");
			printf("   Steps: ");
			UInt8ToChar(interpolators->taskArray[c].Nsteps,nrbfr);
			printf(nrbfr);
			printf(", Lamp: ");
			UInt8ToChar(interpolators->taskArray[c].lamp_nr,nrbfr);
			printf(nrbfr);
			if((interpolators->taskArray[c].state & 0x4) == 0x4)
			{
				printf("\r\n   Mode: repeating");
			}
			else
			{
				printf("\r\n   Mode: one-shot");
			}
			printf(", State: ");
			if((interpolators->taskArray[c].state & 0x2) == 0x2)
			{
				printf("running");
			}
			else
			{
				printf("stopped/starting");
			}
			printf(", Progression: ");
			toPercentChar(getProgression(interpolators->taskArray+c),nrbfr);
			printf(nrbfr);
			printf("%\r\n");
		}
	}
}

/**
 * @brief frees a color sequence / interpolator data structures
 * 
 * returns error messages if no destroyable interpolators have been found
 * @param cmd the command including arguments.
 * @param context the Tasks data structure.
 */
void destroyCommand(char * cmd,void* context)
{
	uint8_t has_errors = 0;
	uint8_t lampnr;
	uint8_t lampidx;
	char bracketContent[128];
	getBracketContent(cmd,bracketContent);
	Tasks interpolators=(Tasks)context;
;	if (bracketContent != 0)
	{
		lampnr = tryToUInt8(bracketContent,&has_errors);
		if (checkLampRange(lampnr,&has_errors) != 0)
		{
			if(lampnr < 0xFF)
			{
				lampidx = getLampIndex(interpolators,lampnr);
				if (lampidx != 0xFF)
				{
					destroyTask(interpolators->taskArray+lampidx);
				}
				else
				{
					printf("ERROR: no interpolator found to destroy\r\n");
				}
			}
			else
			{
				for(uint8_t cc=0;cc<N_LAMPS;cc++)
				{
					lampidx = getLampIndex(interpolators,cc);
					if (lampidx != 0xFF)
					{
						destroyTask(interpolators->taskArray+lampidx);
					}
					else
					{
						printf("ERROR: no interpolator found to destroy\r\n");
					}
				}
			}
		}
	}
}

/**
 * @brief persistently stores a set of interpolators / color sequences
 * return a message indicating how many bytes have been saved and is saving has been successful.
 * @param cmd the command, doesn't taken any arguments, therefore unused here.
 * @param context the Tasks data structure.
 */
void saveCommand(char * cmd,void* context)
{
	uint16_t retcode = 0;
	uint32_t streamsize;
	char nrbfr[8];

	Tasks interpolators=(Tasks)context;
	uint8_t * streamout;
	streamsize = toStream(interpolators,&streamout);
	retcode = saveData((uint16_t*)streamout,streamsize,FLASH_HEADER_SIZE);
	free(streamout);
	printf("\r\nsaved ");
	UInt32ToChar(streamsize,nrbfr);
	printf(nrbfr);
	printf(" bytes to flash, returned code: ");
	UInt16ToChar(retcode,nrbfr);
	printf(nrbfr);
	printf("\r\n");
}

/**
 * @brief load a set of interpolators from the persistent storage
 * 
 * @param cmd the command, doesn't taken any arguments, therefore unused here.
 * @param context the Tasks data structure.
 */
void loadCommand(char * cmd,void* context)
{
	Tasks interpolators=(Tasks)context;
	for (uint8_t c=0;c<interpolators->taskArrayLength;c++)
	{
		destroyTask(interpolators->taskArray+c);
	}
	fromStream((uint16_t*)getFilesystemStart()+FLASH_HEADER_SIZE,interpolators);
}


/**
 * @brief switches the neopixel engine off or on
 * 
 * @param cmd takes either 0 or 1 as argument,0: Off, 1: ON
 * @param context unused
 */
void npEngineCommand(char * cmd,void*context)
{
	uint8_t error_flag=0;
	uint8_t engineflag;
	char bracketContent[4];
	getBracketContent(cmd,bracketContent);
	engineflag = tryToUInt8(bracketContent,&error_flag);
	if (error_flag == 0)
	{
		if(engineflag == 0 || engineflag== 1)
		{
			setEngineState(engineflag);
		}
		else
		{
			printf("Argument must be either 0: Off or 1: On\r\n");
		}
	}
	else
	{
		printf("Couldn't parse arguments\r\n");
	}
}


/**
 * @brief updates the color along a hue shift with the phase going from 0 to 1535
 * @param color the color data to update
 * @param phase the phase value going from 0x0 to 0x600, the hue shifts starts/ends with red
 * */
void colorUpdate(RGB * color,uint32_t phase)
{
	if (phase < 0x100)
	{
		color->r = 0xFF;
		color->g = phase & 0xFF;
		color->b = 0x00;
	}
	else if (phase < 0x200)
	{
		color->r = 0x1FF - phase;
		color->g = 0xFF;
		color->b = 0x00;
	}
	else if (phase < 0x300)
	{
		color->r = 0x00;
		color->g = 0xFF;
		color->b = phase - 0x200;
	}
	else if (phase < 0x400)
	{
		color->r = 0x00;
		color->g = 0x3FF - phase;
		color->b = 0xFF;
	}
	else if (phase < 0x500)
	{
		color->r = phase - 0x400;
		color->g = 0x00;
		color->b = 0xFF;
	}
	else
	{
		color->r = 0xFF;
		color->g = 0x00;
		color->b = 0x5FF - phase;
	}
}