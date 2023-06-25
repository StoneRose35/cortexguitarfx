#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

#include "cs4270_audio_codec.h"
#include "images/toggleswitch_on.h"
#include "images/toggleswitch_off.h"

static volatile uint8_t paramSelected=0;
static volatile uint8_t subLevel;

static void create(PiPicoFxUiType*data)
{
    char strbfr[16];
    uint8_t regbfr;
    float fValue, fMaxValue, fMinValue, px, py, cx, cy;
    uint16_t currentVolume;
    const GFXfont * font = getGFXFont(FREESANSBOLD9PT7B);
    BwImageType* img = getImageBuffer();
    *(strbfr) = 0;  
    clearImage(img);
    appendToString(strbfr,"Settings");
    drawText(2,14,strbfr,img,font);

    regbfr = 0; //cs4270GetInputState();
    if (regbfr & 0x1)
    {
        drawImage(16,20,&toggleswitch_on_streamimg,img);
    }
    else
    {
        drawImage(16,20,&toggleswitch_off_streamimg,img);
    }

    if (regbfr & 0x2)
    {
        drawImage(47,20,&toggleswitch_on_streamimg,img);
    }
    else
    {
        drawImage(47,20,&toggleswitch_off_streamimg,img);
    } 

    *(strbfr) = 0;  
    appendToString(strbfr,"Mic");
    drawText(9,55,strbfr,img,(void*)0);

    *(strbfr) = 0;  
    appendToString(strbfr,"Instr");
    drawText(35,55,strbfr,img,(void*)0);

    *(strbfr) = 0;  
    appendToString(strbfr,"Vol");
    drawText(87,55,strbfr,img,(void*)0);

    currentVolume = 0xFFFF; //cs4270GetOutputVolume();
    drawOval(10.f,10.f,100.f,32.f,img);
    clearOval(8.f,8.f,100.f,32.f,img);

    fValue = int2float((int32_t)(currentVolume & 0xFF));
    fMaxValue = int2float((int32_t)(1 << 8));
    fMinValue = int2float((int32_t)0);
    fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
    px = 100.0f - fsin(fValue)*9.0f;
    py = 32.0f + fcos(fValue)*9.0f;
    cx = 100.0f;
    cy = 32.0f;
    drawLine(cx,cy,px,py,img);

    switch (paramSelected)
    {
        case 0:
            drawHorizontal(18,14,26,img);
            drawHorizontal(46,14,26,img);
            drawVertical(14,18,46,img);
            drawVertical(26,18,46,img);
            break;
        case 1:
            drawHorizontal(18,45,45+12,img);
            drawHorizontal(46,45,45+12,img);
            drawVertical(45,18,46,img);
            drawVertical(45+12,18,46,img);
            break;
        case 2:
            drawHorizontal(21,89,111,img);
            drawHorizontal(43,89,111,img);
            drawVertical(89,21,43,img);
            drawVertical(111,21,43,img);
            break;
    }

}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    ssd1306writeFramebufferAsync(imgBuffer->data);
}


static void enterCallback(PiPicoFxUiType*data) 
{
    BwImageType* img = getImageBuffer();
    if(subLevel==0)
    {
        uiStackPush(data,0xFF);
        subLevel++;
        switch (paramSelected)
        {
            case 0:
                drawHorizontal(18-1,14-1,26+1,img);
                drawHorizontal(46+1,14-1,26+1,img);
                drawVertical(14-1,18-1,46+1,img);
                drawVertical(26+1,18-1,46+1,img);
                break;
            case 1:
                drawHorizontal(18-1,45-1,45+12+1,img);
                drawHorizontal(46+1,45-1,45+12+1,img);
                drawVertical(45-1,18-1,46+1,img);
                drawVertical(45+12+1,18-1,46+1,img);
                break;
            case 2:
                drawHorizontal(21-1,89-1,111+1,img);
                drawHorizontal(43+1,89-1,111+1,img);
                drawVertical(89-1,21-1,43+1,img);
                drawVertical(111+1,21-1,43+1,img);
                break;
        }
    }
}

static void exitCallback(PiPicoFxUiType*data)
{
    BwImageType* img = getImageBuffer();
    if (subLevel==1)
    {
        subLevel--;
        switch (paramSelected)
        {
            case 0:
                clearHorizontal(18-1,14-1,26+1,img);
                clearHorizontal(46+1,14-1,26+1,img);
                clearVertical(14-1,18-1,46+1,img);
                clearVertical(26+1,18-1,46+1,img);
                break;
            case 1:
                clearHorizontal(18-1,45-1,45+12+1,img);
                clearHorizontal(46+1,45-1,45+12+1,img);
                clearVertical(45-1,18-1,46+1,img);
                clearVertical(45+12+1,18-1,46+1,img);
                break;
            case 2:
                clearHorizontal(21-1,89-1,111+1,img);
                clearHorizontal(43+1,89-1,111+1,img);
                clearVertical(89-1,21-1,43+1,img);
                clearVertical(111+1,21-1,43+1,img);
                break;
        }
    }
    else
    {
        if(uiStackCurrent(data)==0xFF)
        {
            uiStackPop(data);
        }
    }
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    uint8_t regbfr;
    uint16_t currentVolume;
    float fValue, fMaxValue, fMinValue, px, py, cx, cy;
    BwImageType* img = getImageBuffer();
    if (subLevel==0) // choose parameter to edit
    {
        switch (paramSelected)
        {
            case 0:
                clearHorizontal(18,14,26,img);
                clearHorizontal(46,14,26,img);
                clearVertical(14,18,46,img);
                clearVertical(26,18,46,img);
                break;
            case 1:
                clearHorizontal(18,45,45+12,img);
                clearHorizontal(46,45,45+12,img);
                clearVertical(45,18,46,img);
                clearVertical(45+12,18,46,img);
                break;
            case 2:
                clearHorizontal(21,89,111,img);
                clearHorizontal(43,89,111,img);
                clearVertical(89,21,43,img);
                clearVertical(111,21,43,img);
                break;
        }
        if (encoderDelta > 0)
        {
            paramSelected++;
            if (paramSelected > 2)
            {
                paramSelected = 2;
            }
        }
        else
        {
            paramSelected--;
            if (paramSelected > 2)
            {
                paramSelected = 0;
            }
        }
        switch (paramSelected)
        {
            case 0:
                drawHorizontal(18,14,26,img);
                drawHorizontal(46,14,26,img);
                drawVertical(14,18,46,img);
                drawVertical(26,18,46,img);
                break;
            case 1:
                drawHorizontal(18,45,45+12,img);
                drawHorizontal(46,45,45+12,img);
                drawVertical(45,18,46,img);
                drawVertical(45+12,18,46,img);
                break;
            case 2:
                drawHorizontal(21,89,111,img);
                drawHorizontal(43,89,111,img);
                drawVertical(89,21,43,img);
                drawVertical(111,21,43,img);
                break;
        }
    }
    else // edit actual setting (mutes, volume)
    {
        switch (paramSelected)
        {
            case 0:
                regbfr = cs4270GetInputState();
                if ((regbfr & 0x1) != 0 && encoderDelta < 0) // switch off, was on
                {
                    cs4270SetInputState(CS4270_CHANNEL_A,0);
                    drawImage(16,20,&toggleswitch_off_streamimg,img);
                }
                else if ((regbfr & 0x1) == 0 && encoderDelta > 0) // switch on, was off
                {
                    cs4270SetInputState(CS4270_CHANNEL_A,1);
                    drawImage(16,20,&toggleswitch_on_streamimg,img);
                }
                break;
            case 1:
                regbfr = cs4270GetInputState();
                if ((regbfr & 0x2) != 0 && encoderDelta < 0) // switch off, was on
                {
                    cs4270SetInputState(CS4270_CHANNEL_B,0);
                    drawImage(47,20,&toggleswitch_off_streamimg,img);
                }
                else if ((regbfr & 0x2) == 0 && encoderDelta > 0) // switch on, was off
                {
                    cs4270SetInputState(CS4270_CHANNEL_B,1);
                    drawImage(47,20,&toggleswitch_on_streamimg,img);
                }
                break;
            case 2:
                currentVolume = cs4270GetOutputVolume();
                currentVolume &= 0xFF;
                currentVolume += encoderDelta;
                if (encoderDelta > 0)
                {
                    if (currentVolume > 0xFF)
                    {
                        currentVolume = 0xFF;
                    }  
                }
                else
                {
                    if (currentVolume > 0xFF)
                    {
                        currentVolume = 0x0;
                    } 
                }
                drawOval(10.f,10.f,100.f,32.f,img);
                clearOval(8.f,8.f,100.f,32.f,img);

                fValue = int2float((int32_t)currentVolume);
                fMaxValue = int2float((int32_t)(1 << 8));
                fMinValue = int2float((int32_t)0);
                fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
                px = 100.0f - fsin(fValue)*9.0f;
                py = 32.0f + fcos(fValue)*9.0f;
                cx = 100.0f;
                cy = 32.0f;
                drawLine(cx,cy,px,py,img);
                cs4270SetOutputVolume(CS4270_CHANNEL_BOTH,(uint8_t)currentVolume);
                break;
        }
    }
}


void enterLevel5(PiPicoFxUiType*data)
{
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

