#include "drivers/24lc128.h"
#include "drivers/systick.h"
#include <stdlib.h>
uint8_t eeprom24s128WritePage(uint32_t address,uint16_t len, uint8_t* data)
{
    uint8_t * sendBfr;
    uint16_t nSent;
    sendBfr = (uint8_t*)malloc(len+2);

    sendBfr[0] = (address >> 8) &0xFF;
    sendBfr[1] = (address) &0xFF;
    for(uint16_t c=0;c<len-1;c++)
    {
        sendBfr[c+2] = *(data+c);
    }
    eeprom24lc128WaitUntilAvailable();
    nSent = I2CsendMultiple(sendBfr,len+2,EEPROM_24LC128_ADDRESS);

    free(sendBfr);

    if (nSent < len)
    {
        return 1;
    }
    return 0;
}


uint8_t eeprom24lc128WriteArray(uint32_t startAdress,uint16_t len, uint8_t* data)
{
    uint16_t addrCnt=startAdress;
    uint16_t dataCnt=0;
    uint16_t remaining=len;
    uint16_t lenToWrite;
    while (remaining > 0)
    {
        if (remaining < EEPROM_24LC128_PAGE_LENGTH)
        {
            lenToWrite = remaining;
            //eeprom24s128WritePage(addrCnt,remaining,data + dataCnt);
        }
        else
        {
            lenToWrite = EEPROM_24LC128_PAGE_LENGTH;
        }
        if ((addrCnt & 0x3F) > ((addrCnt + lenToWrite) & 0x3F)) // crossing page boundary?
        {
            lenToWrite = ((addrCnt + lenToWrite) & 0xFFC0) - addrCnt;
        }
        if (eeprom24s128WritePage(addrCnt,lenToWrite,data + dataCnt)!=0)
        {
            return 1;
        }
        waitSysticks(1);
        remaining -= lenToWrite;
        addrCnt += lenToWrite;
        dataCnt += lenToWrite;
    }
    return 0;
}

uint8_t eeprom24lc128ReadArray(uint32_t startAdress,uint16_t len,uint8_t* data)
{

    uint8_t sendBfr[2];
    eeprom24lc128WaitUntilAvailable();
    sendBfr[0]=(startAdress >> 8) &0xFF;
    sendBfr[1]=startAdress & 0xFF;
    I2CsendMultiple(sendBfr,2,EEPROM_24LC128_ADDRESS);
    if (I2CReceiveMultiple(data,len,EEPROM_24LC128_ADDRESS) != len)
    {
        return 1;
    }
    return 0;
}

void eeprom24lc128WaitUntilAvailable()
{
    uint8_t sendBfr[1]={0};
    while(I2CsendMultiple(sendBfr,1,EEPROM_24LC128_ADDRESS)==0) // wait until at least one byte could be sent
    {

    }
}