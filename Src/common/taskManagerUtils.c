/**
 * @file taskManagerUtils.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief utility functions for the task manager
 * @version 0.1
 * @date 2021-12-23
 * 
 * 
 */

#include "systemChoice.h"
#include "commands/taskManagerUtils.h"
#include "intFunctions.h"
#include <string.h>
#include <stdlib.h>

#ifndef HARDWARE
#include <stdio.h>
#else
#include "consoleBase.h"
#endif

/**
 * @brief null-save string to integer conversion
 * 
 * @param str the string to convert
 * @param has_errors_ptr pointer to an error flag, is set one if and error happened
 * @return the converted string, 0 in case of an error
 */
uint8_t tryToUInt8(char * str,uint8_t* has_errors_ptr)
{
	if (str != 0)
	{
		return toUInt8(str);
	}
	else
	{
		printf("ERROR: tryToUInt8 got a NullPointer\n");
		*has_errors_ptr = 1;
		return 0;
	}
}

/**
 * @brief null-save string to integer conversion
 * 
 * @param str the string to convert
 * @param has_errors_ptr pointer to an error flag, is set one if and error happened
 * @return the converted string, 0 in case of an error
 */
int16_t tryToInt16(char * str,uint8_t* has_errors_ptr)
{
	if (str != 0)
	{
		return toInt16(str);
	}
	else
	{
		printf("ERROR: tryToInt16 got a NullPointer\n");
		*has_errors_ptr = 1;
		return 0;
	}
}

/**
 * @brief null-save string to integer conversion
 * 
 * @param str the string to convert
 * @param has_errors_ptr pointer to an error flag, is set one if and error happened
 * @return the converted string, 0 in case of an error
 */
uint32_t tryToUInt32(char * str,uint8_t* has_errors_ptr)
{
	if (str != 0)
	{
		return toUInt32(str);
	}
	else
	{
		printf("ERROR: tryToInt16 got a NullPointer\n");
		*has_errors_ptr = 1;
		return 0;
	}
}


/**
 * @brief converts a neopixel description containing ranges and individual indexes into a set of distinct and value neopixel indexes
 * 
 * The description does not need to define unique indexes, for example "1-6,3,8" would expand int o 1,2,3,4,5,6,8 without an error 
 * Avoid using range definition for when inerfacing via API since range expansion can be computationally intense and and a list 
 * of indexes can be generated easily from an API caller.
 * @param description the description, example: "1,2,6-9,10"
 * @param res the resulting array, is initialized within the function
 * @return the length of the expanded array
 */
uint8_t expandLampDescription(char * description,uint8_t * res)
{
	char * arrayElement;
	uint8_t * rangePtr;
	uint8_t rlength=0;
	uint8_t nlamps=0;
	uint8_t swapval;
	stripWhitespaces(description);
	arrayElement = strtok(description,",");
	//uint8_t lampsnrs[N_LAMPS];
	void* isInArray;
	while (arrayElement != NULL)
	{
		rlength = expandRange(arrayElement,&rangePtr);
		// insert into array
		for(uint8_t c=0;c<rlength;c++)
		{
			isInArray = bsearch((void*)(rangePtr+c),res,nlamps,sizeof(uint8_t),compareUint8);
			if (isInArray==NULL)
			{
				res[nlamps++]=rangePtr[c];
				for(uint8_t c2=nlamps-1;c2>0;c2--)
				{
					if(res[c2-1] > res[c2])
					{
						swapval = res[c2-1];
						res[c2-1] = res[c2];
						res[c2] = swapval;
					}
				}
			}
		}
		free(rangePtr);
		arrayElement = strtok(NULL,",");
	}
	//*res=lampsnrs;
	return nlamps;
}



