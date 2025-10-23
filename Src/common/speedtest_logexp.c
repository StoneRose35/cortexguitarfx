#include "systick.h"
#include "stringFunctions.h"
#include <stdint.h>
#include "fastExpLog.h"
#include "ln.h"
#include "uart.h"
/*
#define SPEEDTEST_N_ITERATION_FOR_LOG 100000
#define SPEEDTEST_FOR_LOG_INCREMENT 0.01f
#define SPEEDTEST_FOR_LIN_INCREMENT 0.1f
float testSpeed()
{
    char charBfr[256];
    char ncBfr[32];
    uint16_t clen;
    float xval;
    float xInterm;
    float unusedSum=0.0f;
    volatile float unusedResult; 
    xval = 0.0f;
    uint32_t tStart,tEnd;

    initUart(115200);

    while (xval < 1.0f)
    {
        xval += SPEEDTEST_FOR_LOG_INCREMENT;
        xInterm = xval;
        tStart = getTickValue();
        for (uint32_t c=0;c<SPEEDTEST_N_ITERATION_FOR_LOG;c++)
        {
            unusedSum += fastlog(xInterm);
        }
        tEnd = getTickValue();
        unusedResult = unusedSum;
        copyToString(charBfr,"fastlog for ");
        UInt16ToChar((uint16_t)(xval/SPEEDTEST_FOR_LOG_INCREMENT),ncBfr);
        appendToString(charBfr,ncBfr);
        appendToString(charBfr,", ticks used ");
        UInt32ToChar(tEnd-tStart,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n");
        sendBlocking((uint8_t*)charBfr,clen);

        tStart = getTickValue();
        for (uint32_t c=0;c<SPEEDTEST_N_ITERATION_FOR_LOG;c++)
        {
            unusedSum += toDb(xInterm);
        }
        tEnd = getTickValue();
        unusedResult = unusedSum;
        copyToString(charBfr,"toDb for ");
        UInt16ToChar((uint16_t)(xval/SPEEDTEST_FOR_LOG_INCREMENT),ncBfr);
        appendToString(charBfr,ncBfr);
        appendToString(charBfr,", ticks used ");
        UInt32ToChar(tEnd-tStart,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n\r\n");
        sendBlocking((uint8_t*)charBfr,clen);
    }

    xval = -120.0f;
    while (xval < 0.0f)
    {
        xval += SPEEDTEST_FOR_LIN_INCREMENT;
        xInterm = xval;
        tStart = getTickValue();
        for (uint32_t c=0;c<SPEEDTEST_N_ITERATION_FOR_LOG;c++)
        {
            unusedSum += fastexp(xInterm);
        }
        tEnd = getTickValue();
        unusedResult = unusedSum;
        copyToString(charBfr,"fastexp for ");
        Int16ToChar((int16_t)(xval/SPEEDTEST_FOR_LIN_INCREMENT),ncBfr);
        appendToString(charBfr,ncBfr);
        appendToString(charBfr,", ticks used ");
        UInt32ToChar(tEnd-tStart,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n");
        sendBlocking((uint8_t*)charBfr,clen);

        tStart = getTickValue();
        for (uint32_t c=0;c<SPEEDTEST_N_ITERATION_FOR_LOG;c++)
        {
            unusedSum += toLin(xInterm);
        }
        tEnd = getTickValue();
        unusedResult = unusedSum;
        copyToString(charBfr,"toLin for ");
        Int16ToChar((int16_t)(xval/SPEEDTEST_FOR_LIN_INCREMENT),ncBfr);
        appendToString(charBfr,ncBfr);
        appendToString(charBfr,", ticks used ");
        UInt32ToChar(tEnd-tStart,ncBfr);
        appendToString(charBfr,ncBfr);
        clen = appendToString(charBfr,"\r\n\r\n");
        sendBlocking((uint8_t*)charBfr,clen);
    }
    return unusedResult;
}
*/