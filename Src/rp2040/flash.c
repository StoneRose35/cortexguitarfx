/*
 * flash.c
 *
 *  Created on: 27.10.2021
 *      Author: philipp
 */

#ifdef RP2040_FEATHER

#include <stdint.h>
#include "neopixelDriver.h"
#include "stdlib.h"
#include "flash.h"
#include "romfunc.h"


void unlockFlash()
{

}
uint8_t erasePage(uint16_t pagenr)
{
	flash_range_erase(pagenr << 12,4096,1,' ');
	return 0;
}

uint8_t programHalfword(uint16_t hwrd,uint16_t* addr)
{
	return 0;
}

uint8_t programPage(uint16_t pagenr,uint16_t* data,uint16_t cnt)
{
	ptr fsStart = getFilesystemStart();
	flash_range_program((uint32_t)(fsStart + (pagenr << 12)),(uint8_t*)data,cnt << 1);
	return 0;
}


#endif
