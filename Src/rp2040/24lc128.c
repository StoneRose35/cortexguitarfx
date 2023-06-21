#include "24lc128.h"
#include "systick.h"

void eeprom24s128WritePage(uint32_t address,uint16_t len, uint8_t* data)
{
    uint8_t i2c_resp=0;
    i2c_resp += masterTransmit((address >> 8) &0xFF,0);
    i2c_resp += masterTransmit((address) &0xFF,0);
    while(i2c_resp > 0) // 
    {
        i2c_resp = 0;
        i2c_resp += masterTransmit((address >> 8) &0xFF,0);
        i2c_resp += masterTransmit((address) &0xFF,0);
    }

    for(uint16_t c=0;c<len-1;c++)
    {
        masterTransmit(*(data+c),0);
    }
    masterTransmit(*(data+len-1),1);
}


void eeprom24lc128WriteArray(uint32_t startAdress,uint16_t len, uint8_t* data)
{
    uint16_t addrCnt=startAdress;
    uint16_t dataCnt=0;
    uint16_t remaining=len;
    uint16_t lenToWrite;
    if (getTargetAddress()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddress(EEPROM_24LC128_ADDRESS);
    }
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
        eeprom24s128WritePage(addrCnt,lenToWrite,data + dataCnt);
        remaining -= lenToWrite;
        addrCnt += lenToWrite;
        dataCnt += lenToWrite;
    }
}

void eeprom24lc128ReadArray(uint32_t startAdress,uint16_t len,uint8_t* data)
{
    uint8_t i2c_resp=0;
    if (getTargetAddress()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddress(EEPROM_24LC128_ADDRESS);
    }
    i2c_resp += masterTransmit((startAdress >> 8) &0xFF,0);
    i2c_resp += masterTransmit(startAdress & 0xFF,1);   
    while (i2c_resp > 0)
    {
        i2c_resp = 0;
        i2c_resp += masterTransmit((startAdress >> 8) &0xFF,0);
        i2c_resp += masterTransmit(startAdress & 0xFF,1); 
    }

    for(uint16_t c=0;c<len-1;c++)
    {
        *(data + c) =masterReceive(0);
    }
    *(data + len -1) = masterReceive(1);
}