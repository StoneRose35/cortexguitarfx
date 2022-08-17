/**
 * @file memoryAccess.c
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief provides functions to access persistent storage
 * @version 0.1
 * @date 2021-12-23
 * 
 * 
 */


#include <stdint.h>

#include <stdlib.h>
#include "flash.h"
#include "memoryAccess.h"

/**
 * @brief save size bytes at the beginning of the persistent storage sector
 * 
 * @param data halfword array to save
 * @param size length of the array in halfwords
 * @return uint16_t 0 if exited without error, >0 otherwise
 */
uint16_t saveHeader(uint16_t * data,uint32_t size)
{
	uint16_t retcode=0;
	uint16_t pageBfr[FLASH_PAGE_SIZE >> 1];
	for(uint16_t c=0;c<FLASH_PAGE_SIZE>>1; c++)
	{
		pageBfr[c] = *((uint16_t*)getFilesystemStart() + c);
	}

	erasePage(0);

	for (uint32_t c2=0;c2<size;c2++)
	{
		retcode += programHalfword(*(data+(c2>>1)),(uint16_t*)getFilesystemStart()+c2);
	}
	for (uint32_t c3=0;c3<((FLASH_PAGE_SIZE>>1)-size);c3++)
	{
		retcode += programHalfword(*(pageBfr+c3+size),(uint16_t*)getFilesystemStart()+size+c3);
	}
	return retcode;
}

/**
 * @brief saves data at a given position in the persistent storage section
 * 
 * @param data halfword array to save
 * @param size length of the array in halfwords
 * @param offset start address in bytes
 * @return uint16_t 0 if exited successfully, >0 otherwise
 */
uint16_t saveData(uint16_t * data,uint32_t size,uint32_t offset)
{
	uint8_t sizeInPages = (uint8_t)(size >> FLASH_PAGE_SIZE_BIT);
	if ((size & (FLASH_PAGE_SIZE-1))!=0)
	{
		sizeInPages++;
	}
	sizeInPages <<=  1;
	uint32_t dataCnt=0;
	uint32_t flashCnt=0;
	uint32_t pageOffset;
	uint16_t pageBuffer[FLASH_PAGE_SIZE>>1];
	uint16_t c=0;

	ptr fsStart = getFilesystemStart();
	uint16_t firstpage = offset >> FLASH_PAGE_SIZE_BIT;

	while(dataCnt<size)
	{
		if(c==0)
		{
			pageOffset = (offset - ((offset >> FLASH_PAGE_SIZE_BIT)<<FLASH_PAGE_SIZE_BIT)) >> 1;
		}
		else
		{
			pageOffset=0;
		}
		for (uint16_t c2=0;c2<FLASH_PAGE_SIZE>>1;c2++)
		{
			if(dataCnt < size && c2 >= pageOffset)
			{
				pageBuffer[c2] = *(data+dataCnt++);
			}
			else if (dataCnt < size && c2 < pageOffset)
			{
				pageBuffer[c2] = *((uint16_t*)fsStart + flashCnt++);
			}
			else
			{
				pageBuffer[c2] = *((uint16_t*)fsStart + size+ (offset>>1) + flashCnt++);
			}
		}
		erasePage(firstpage + c);
		programPage(firstpage + c,pageBuffer,FLASH_PAGE_SIZE>>1);
		c++;
	}

	return 0;
}
