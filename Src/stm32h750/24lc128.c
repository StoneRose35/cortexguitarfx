#include "drivers/24lc128.h"
#include "drivers/systick.h"
#include "drivers/i2c.h"

void eeprom24s128WritePage(uint32_t address,uint16_t len, uint8_t* data)
{
    eeprom24lc128WaitUntilAvailable();


    masterTransmitExternal((address >> 8) &0xFF,0);
    masterTransmitExternal((address) &0xFF,0);

    for(uint16_t c=0;c<len-1;c++)
    {
        masterTransmitExternal(*(data+c),0);
    }
    masterTransmitExternal(*(data+len-1),1);
}


void eeprom24lc128WriteArray(uint32_t startAdress,uint16_t len, uint8_t* data)
{
    uint16_t addrCnt=startAdress;
    uint16_t dataCnt=0;
    uint16_t remaining=len;
    uint16_t lenToWrite;
    if (getTargetAddressExternal()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddressExternal(EEPROM_24LC128_ADDRESS);
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
        waitSysticks(1);
        remaining -= lenToWrite;
        addrCnt += lenToWrite;
        dataCnt += lenToWrite;
    }
}

void eeprom24lc128ReadArray(uint32_t startAdress,uint16_t len,uint8_t* data)
{

    if (getTargetAddressExternal()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddressExternal(EEPROM_24LC128_ADDRESS);
    }
    eeprom24lc128WaitUntilAvailable();
    masterTransmitExternal((startAdress >> 8) &0xFF,0);
    masterTransmitExternal(startAdress & 0xFF,1); 
    I2CReceiveMultipleExternal(data,len);
}

void eeprom24lc128WaitUntilAvailable()
{
    if (getTargetAddressExternal()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddressExternal(EEPROM_24LC128_ADDRESS);
    }
    while(masterTransmitExternal(0,1)==0) // wait until at least one byte could be sent
    {

    }
}