
#include "displayCommands.h"
#include "spi_sdcard_display.h"
#include "charDisplay.h"
#include "uart.h"
#include "stringFunctions.h"
#include "taskManagerUtils.h" 
#include <string.h>
#include "consoleBase.h"
static uint8_t ptrX=0;
static uint8_t ptrY=0;

void initDisplayCommand(char * cmd,void* context)
{
    printf("Initializing Display .. ");
    initDisplay(); 
    printf("Done \r\n");
}

void setBacklightCommand(char * cmd,void* context)
{
    char bcontent[8];
    uint8_t brightnessval, errorflag = 0;
    getBracketContent(cmd,bcontent);
    brightnessval = tryToUInt8(bcontent,&errorflag);
    if(errorflag == 0)
    {
        setBacklight(brightnessval);
    }
    else{
        printf("\r\nERROR: brightness value must be an integer from 0 to 255\r\n");
    }
}

void setCursorCommand(char * cmd,void* context)
{
    char bcontent[8];
    char nrbfr[4];
    uint8_t errorcodes=0;
    char *nrbfrPtr = nrbfr; 
    uint8_t posh,posv;
    getBracketContent(cmd,bcontent);
    nrbfrPtr = strtok(bcontent,",");
    posh = tryToUInt8(nrbfrPtr,&errorcodes);
    if (errorcodes != 0)
    {
        printf("\r\nERROR: couild not read first position\r\n");
        return;
    }
    if(posh > 19)
    {
        printf("\r\nERROR: horizontal position mus be within 0 to 19\r\n");
        return;
    }
    nrbfrPtr = strtok(0,",");
    posv = tryToUInt8(nrbfrPtr,&errorcodes);
    if (errorcodes != 0)
    {
        printf("\r\nERROR: couild not read sencond position\r\n");
        return;
    }
    if(posh > 15)
    {
        printf("\r\nERROR: vertical position mus be within 0 to 15\r\n");
        return;
    }
    ptrX = posh;
    ptrY=posv;
}

void writeStringCommand(char * cmd,void * context)
{
    uint16_t cmdCnt=0,c2=0;
    uint8_t firstBracketPos=0;
    while(*(cmd + cmdCnt) != 0)
    {
        if(*(cmd+cmdCnt) == '(' && firstBracketPos==0)
        {
            firstBracketPos = cmdCnt;
        }
        cmdCnt++;
    }
    while(c2 < cmdCnt - firstBracketPos-2)
    {
        *(cmd + c2) = *(cmd + c2 + firstBracketPos + 1);
        c2++;
    }
    *(cmd + c2) = 0;
    writeString(cmd,ptrX,ptrY);
}