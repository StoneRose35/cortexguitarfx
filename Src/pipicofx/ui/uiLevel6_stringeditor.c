#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "oled_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static volatile uint8_t editPos=0;
static volatile uint8_t editState=0;
static volatile uint8_t maxStringLength=8;
static char * stringBkp;

/*

  ---------------------------------
  |  _                            |
  |  StringToEdit                 |
  |  -                            |
  |                               |
  |  OK                 Cancel    | 
  |                               |
  ---------------------------------

*/

static void create(PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    char * stringBfr = (char*)data->data;
    const GFXfont * font = getGFXFont(FREEMONO9PT7B);
    clearImage(imgBuffer);
    drawText(2,2+11,stringBfr,imgBuffer,font);
    
    maxStringLength = 0;
    while(*(stringBfr + maxStringLength) != 0)
    {
        maxStringLength++;
    }

    stringBkp = malloc(maxStringLength+1);
    for(uint8_t c=0;c<maxStringLength;c++)
    {
        *(stringBkp+c)=*(stringBfr+c);
    }
    *(stringBkp+maxStringLength) = 0;
    
    if (editPos < 254)
    {
        drawLine(editPos*11,1,(editPos+1)*11,1,imgBuffer);
        drawLine(editPos*11,2+font->yAdvance,(editPos+1)*11,2+font->yAdvance,imgBuffer);
    }
    else if (editPos == 254) // OK
    {
        drawLine(10,53-12,32,53-12,imgBuffer);
        drawLine(10,53-11+font->yAdvance,32,53-11+font->yAdvance,imgBuffer);
    }
    else // Cancel
    {
        drawLine(40,53-12,106,53-12,imgBuffer);
        drawLine(40,53-11+font->yAdvance,106,53-11+font->yAdvance,imgBuffer);
    }

    drawText (10,53,"OK",imgBuffer,font);
    drawText(40,53,"Cancel",imgBuffer,font);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    OledwriteFramebufferAsync(imgBuffer->data);
}


static void enterCallback(PiPicoFxUiType*data) 
{
    BwImageType* imgBuffer = getImageBuffer();
    const GFXfont * font = getGFXFont(FREEMONO9PT7B);
    if (editState == 0)
    {
        editState = 1;
        if (editPos < 254)
        {
            drawLine(editPos*11,0,(editPos+1)*11,0,imgBuffer);
            drawLine(editPos*11,3+font->yAdvance,(editPos+1)*11,3+font->yAdvance,imgBuffer);
        }
        if (uiStackCurrent(data) != 0xFF)
        {
            uiStackPush(data,0xFF);
        }
    }
}

static void exitCallback(PiPicoFxUiType*data)
{    
    BwImageType* imgBuffer = getImageBuffer();
    const GFXfont * font = getGFXFont(FREEMONO9PT7B);
    if (editState == 1)
    {
        editState = 0;
        if (editPos < 254)
        {
            clearLine(editPos*11,0,(editPos+1)*11,0,imgBuffer);
            clearLine(editPos*11,3+font->yAdvance,(editPos+1)*11,3+font->yAdvance,imgBuffer);
        }
    }
    else
    {
        if (editPos == 254) // OK
        {
            uiStackPop(data);
            free(stringBkp);
        }
        else if (editPos == 255) // Cancel
        {
            uiStackPop(data);
            for(uint8_t c=0;c<maxStringLength;c++)
            {
                *((char*)data->data + c) = *(stringBkp+ c);
            }
            free(stringBkp);
        }
    }
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    int16_t newPos;
    int8_t newChar;
    BwImageType* imgBuffer = getImageBuffer();
    const GFXfont * font = getGFXFont(FREEMONO9PT7B);
    if (editState == 0) // change letter / OK / Cancel button
    {
        newPos = editPos + encoderDelta;
        if (newPos < 0)
        {
            newPos = 0;
        }
        else if (newPos > maxStringLength-1 && editPos < 254)
        {
            newPos = 254;
        }
        else if (editPos == 254 && newPos > editPos)
        {
            newPos = 255;
        }
        else if (editPos == 254 && newPos < editPos)
        {
            newPos = maxStringLength-1;
        }
        else if (editPos == 255 && encoderDelta < 0)
        {
            newPos = 254;
        }
        else if (editPos == 255 && encoderDelta > 0)
        {
            newPos = 255;
        }

        if (editPos < 254)
        {
            clearLine(editPos*11,1,(editPos+1)*11,1,imgBuffer);
            clearLine(editPos*11,2+font->yAdvance,(editPos+1)*11,2+font->yAdvance,imgBuffer);
        }
        else if (editPos == 254) // OK
        {
            clearLine(10,53-12,32,53-12,imgBuffer);
            clearLine(10,53-11+font->yAdvance,32,53-11+font->yAdvance,imgBuffer);
        }
        else // Cancel
        {
            clearLine(40,53-12,106,53-12,imgBuffer);
            clearLine(40,53-11+font->yAdvance,106,53-11+font->yAdvance,imgBuffer);
        }

        if (newPos < 254)
        {
            drawLine(newPos*11,1,(newPos+1)*11,1,imgBuffer);
            drawLine(newPos*11,2+font->yAdvance,(newPos+1)*11,2+font->yAdvance,imgBuffer);
        }
        else if (newPos == 254) // OK
        {
            drawLine(10,53-12,32,53-12,imgBuffer);
            drawLine(10,53-11+font->yAdvance,32,53-11+font->yAdvance,imgBuffer);
        }
        else // Cancel
        {
            drawLine(40,53-12,106,53-12,imgBuffer);
            drawLine(40,53-11+font->yAdvance,106,53-11+font->yAdvance,imgBuffer);
        }        
        editPos = newPos;
    }
    else
    {
        if(editPos < 254)
        {
            newChar = *((char*)data->data + editPos) + encoderDelta;
            if(newChar > 126)
            {
                newChar=126;
            } 
            else if (newChar < 0)
            {
                newChar = 126;
            }
            else if (newChar < 32)
            {
                newChar = 32;
            }
            clearSquare(0,2,127,2+font->yAdvance,imgBuffer);
            *((char*)data->data + editPos)=newChar;
            drawText(2,2+11,(char*)data->data,imgBuffer,font);
        }
    }
}


void enterLevel6(PiPicoFxUiType*data)
{
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    //registerKnob0Callback(&knob0Callback);
    //registerKnob1Callback(&knob1Callback);
    //registerKnob2Callback(&knob2Callback);
    //registerStompswitch1PressedCallback(&stompswitch1Callback);
    //registerStompswitch2PressedCallback(&stompswitch2Callback);
    //registerStompswitch3PressedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

