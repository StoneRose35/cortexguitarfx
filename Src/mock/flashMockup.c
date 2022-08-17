#include "systemChoice.h"

#ifndef HARDWARE

#include "flash.h"

void unlockFlash()
{

}

uint8_t erasePage(uint16_t pagenr)
{
	for(uint16_t c=0;c<FLASH_PAGE_SIZE>>1;c++)
	{
		*((uint16_t*)fakeflash+c+ (FLASH_PAGE_SIZE>>1)*pagenr)=0xFFFF;
	}
	return 0;
}
uint8_t programHalfword(uint16_t hwrd,uint16_t* addr)
{
	*addr&=hwrd;
	return 0;
}

uint8_t programPage(uint16_t pagenr,uint16_t* data,uint16_t cnt)
{
	for(uint16_t c=0;c<FLASH_PAGE_SIZE>>1;c++)
	{
		*((uint16_t*)fakeflash + c + (FLASH_PAGE_SIZE >> 1)*pagenr) &= *(data +c);
	}
	return 0;
}


#endif
