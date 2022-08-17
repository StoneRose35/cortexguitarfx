/**
 * @file intFunctions.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief contains function operating on integer types
 * @version 0.1
 * @date 2021-12-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "intFunctions.h"
#include "system.h"

/**
 * @brief compares to bytes interpreted as unsigned integers
 * 
 * @param a the first integer 
 * @param b  the second integer
 * @return a - b as signed integer
 */
int compareUint8(const void* a,const void* b)
{
	return (int)(*(uint8_t*)a - *(uint8_t*)b);
}
