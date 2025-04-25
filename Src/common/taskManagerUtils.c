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
#include "taskManagerUtils.h"
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




