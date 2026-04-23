#include "stm32h750/stm32h750xx.h"
#include "drivers/24lc128.h"
#include "drivers/systick.h"
#include "drivers/i2c.h"
#include "stdlib.h"
#include "memoryRegions.h"

__QSPI_CODE
uint8_t eeprom24s128WritePage(uint32_t address,uint16_t len, uint8_t* data)
{
    uint8_t * sendBfr;
    uint16_t nSent;
    sendBfr = (uint8_t*)malloc(len+2);

    sendBfr[0] = (address >> 8) &0xFF;
    sendBfr[1] = (address) &0xFF;
    for(uint16_t c=0;c<len;c++)
    {
        sendBfr[c+2] = *(data+c);
    }
    eeprom24lc128WaitUntilAvailable();
    nSent = I2CsendMultipleExternal(sendBfr,len+2);


    free(sendBfr);

    if (nSent < len)
    {
        return 1;
    }
    return 0;
}

__QSPI_CODE
uint8_t eeprom24lc128WriteArray(uint32_t startAdress,uint16_t len, uint8_t* data)
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

__QSPI_CODE
uint8_t eeprom24lc128ReadArray(uint32_t startAdress,uint16_t len,uint8_t* data)
{
    uint8_t sendBfr[2];
    if (getTargetAddressExternal()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddressExternal(EEPROM_24LC128_ADDRESS);
    }
    eeprom24lc128WaitUntilAvailable();
    sendBfr[0]=(startAdress >> 8) &0xFF;
    sendBfr[1]=startAdress & 0xFF;
    I2CsendMultipleExternal(sendBfr,2);
    if (I2CReceiveMultipleExternal(data,len) != len)
    {
        return 1;
    }
    return 0;
}

__QSPI_CODE
void eeprom24lc128WaitUntilAvailable()
{
    uint8_t sendBfr[1]={0};
    if (getTargetAddressExternal()!=EEPROM_24LC128_ADDRESS)
    {
        setTargetAddressExternal(EEPROM_24LC128_ADDRESS);
    }
    while(I2CsendMultipleExternal(sendBfr,1)==0) // wait until at least one byte could be sent
    {

    }
}
