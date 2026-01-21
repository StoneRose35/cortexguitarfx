#include "flash.h"
#include "stm32h750/stm32h750xx.h"
#include "memoryRegions.h"



__RAMFUNC
uint8_t unlockFlash()
{
 
    if ((FLASH->CR1 & (1 << FLASH_CR_LOCK_Pos))!=0)
    {
        FLASH->KEYR1 = 0x45670123;
        FLASH->KEYR1 = 0xCDEF89AB; 
    } 
    if  ((FLASH->CR1 & (1 << FLASH_CR_LOCK_Pos))!=0)
    {
        return 1;
    }
    return 0;
}

__RAMFUNC
uint8_t eraseSector()
{
     uint8_t res;
     uint32_t reg;
    if ((FLASH->CR1  & (1 << FLASH_CR_LOCK_Pos))!= 0)
    {
        res = unlockFlash();
        if (res != 0)
        {
            return res;
        }
    }
    reg = FLASH->CR1;
    reg &= ~((FLASH_CR_SER) | (FLASH_CR_SNB));
    reg |= (1 << FLASH_CR_SER_Pos) | (0 << FLASH_CR_SNB_Pos);
    FLASH->CR1 = reg;
    FLASH->CR1 |= (1 << FLASH_CR_START_Pos);
    while ((FLASH->SR1 & (1 << FLASH_SR_QW_Pos))!=0);
    return 0;
}

/*
    data: array of input data, always 32 bytes long
    address: relative to start of Bank 1, Sector 0
*/
__RAMFUNC
uint8_t writeFlashWord(uint8_t*data,uint32_t address)
{
    uint8_t res;
    if ((address & 0x1F) != 0) // address is not 32 byte aligned
    {
        return 2;
    }
    if ((FLASH->CR1  & (1 << FLASH_CR_LOCK_Pos))!= 0 || ((FLASH->CR1 & (1 << FLASH_CR_PG_Pos))==0))
    {
        res = unlockFlash();
        if (res != 0)
        {
            return res;
        }
        FLASH->CR1 |= (1 << FLASH_CR_PG_Pos);       
    }
    while ((FLASH->SR1 & (1 << FLASH_SR_BSY_Pos))!= 0);
    for (uint8_t c=0;c<32;c++)
    {
        *((volatile uint8_t*)FLASH_BANK1_BASE + c + address) = *(data + c);  
    }
    if ((FLASH->SR1 & (1 << FLASH_SR_QW_Pos))==0)
    {
        return 1;
    }
    while ((FLASH->SR1 & (1 << FLASH_SR_QW_Pos))!=0);
    return 0;
}