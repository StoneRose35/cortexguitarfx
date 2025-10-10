#include "uart.h"
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"



void writeandCheck(uint32_t testVal,uint16_t readTimes)
{
    char charBfr[256];
    char ncBfr[32];
    uint16_t clen;
    uint32_t val;
    uint32_t * memPtr = (uint32_t*)0xC0001000;
    clen = copyToString(charBfr,"Initializing SDRAM with ");
    UInt32ToChar(testVal,ncBfr);
    appendToString(charBfr,ncBfr);
    clen = appendToString(charBfr," \r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint32_t c=0;c<DELAY_LINE_SDRAM_LENGTH;c++)
    {
        *(memPtr + c) = testVal;
    }
    clen = copyToString(charBfr,"Reading SDRAM ");
    UInt16ToChar(readTimes,ncBfr);
    appendToString(charBfr,ncBfr);
    clen = appendToString(charBfr," Times\r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint16_t c=0;c<readTimes;c++)
    {
        copyToString(charBfr," Iteration ");
        UInt16ToChar(c,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n");
        sendBlocking((uint8_t*)charBfr,clen);
        for (uint32_t cc=0;cc<DELAY_LINE_SDRAM_LENGTH;cc++)
        {
            val = *(memPtr + cc);
            if (val != testVal)
            {
                copyToString(charBfr,"expected ");
                UInt32ToChar(testVal,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr," at ");
                UInt32ToHex(cc,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr,", got ");
                UInt32ToChar(val,ncBfr);
                appendToString(charBfr,ncBfr);
                clen = appendToString(charBfr,"\r\n");
                sendBlocking((uint8_t*)charBfr,clen);
            }
        }
    }
}

void writeandCheckAscending(uint16_t readTimes)
{
    char charBfr[256];
    char ncBfr[32];
    uint16_t clen;
    uint32_t val;
    uint32_t * memPtr = (uint32_t*)0xC0001000;
    clen = copyToString(charBfr,"write and read ascending sequence");
    clen = appendToString(charBfr," \r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint32_t c=0;c<DELAY_LINE_SDRAM_LENGTH;c++)
    {
        *(memPtr + c) = c;
    }
    clen = copyToString(charBfr,"Reading SDRAM ");
    UInt16ToChar(readTimes,ncBfr);
    appendToString(charBfr,ncBfr);
    clen = appendToString(charBfr," Times\r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint16_t c=0;c<readTimes;c++)
    {
        copyToString(charBfr," Iteration ");
        UInt16ToChar(c,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n");
        sendBlocking((uint8_t*)charBfr,clen);
        for (uint32_t cc=0;cc<DELAY_LINE_SDRAM_LENGTH;cc++)
        {
            val = *(memPtr + cc);
            if (val != cc)
            {
                copyToString(charBfr,"expected ");
                UInt32ToChar(cc,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr," at ");
                UInt32ToHex(cc,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr,", got ");
                UInt32ToChar(val,ncBfr);
                appendToString(charBfr,ncBfr);
                clen = appendToString(charBfr,"\r\n");
                sendBlocking((uint8_t*)charBfr,clen);
            }
        }
    }
}

void writeandCheckDescending(uint16_t readTimes)
{
    char charBfr[256];
    char ncBfr[32];
    uint16_t clen;
    uint32_t val;
    uint32_t * memPtr = (uint32_t*)0xC0001000;
    clen = copyToString(charBfr,"write and read descending sequence");
    clen = appendToString(charBfr," \r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint32_t c=0;c<DELAY_LINE_SDRAM_LENGTH;c++)
    {
        *(memPtr + c) = 0xFFFFFFFF - c;
    }
    clen = copyToString(charBfr,"Reading SDRAM ");
    UInt16ToChar(readTimes,ncBfr);
    appendToString(charBfr,ncBfr);
    clen = appendToString(charBfr," Times\r\n");
    sendBlocking((uint8_t*)charBfr,clen);
    for (uint16_t c=0;c<readTimes;c++)
    {
        copyToString(charBfr," Iteration ");
        UInt16ToChar(c,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n");
        sendBlocking((uint8_t*)charBfr,clen);
        for (uint32_t cc=0;cc<DELAY_LINE_SDRAM_LENGTH;cc++)
        {
            val = *(memPtr + cc);
            if (val != (0xFFFFFFFF - cc))
            {
                copyToString(charBfr,"expected ");
                UInt32ToChar(0xFFFFFFFF - cc,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr," at ");
                UInt32ToHex(cc,ncBfr);
                appendToString(charBfr,ncBfr);
                appendToString(charBfr,", got ");
                UInt32ToChar(val,ncBfr);
                appendToString(charBfr,ncBfr);
                clen = appendToString(charBfr,"\r\n");
                sendBlocking((uint8_t*)charBfr,clen);
            }
        }
    }
}

void checkSDRAM()
{

    initUart(115200);

    writeandCheck(0,16);
    writeandCheck(0xFFFFFFFF,16);
    writeandCheck(0x55555555,16);
    writeandCheck(0xAAAAAAAA,16);
    writeandCheckAscending(16); 
    writeandCheckDescending(16); 
}