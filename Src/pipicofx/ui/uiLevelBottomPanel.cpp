extern "C" {
#include "stdlib.h"
#include "pipicofx/pipicofxui.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/display128x64.h"
#include "audio/looper.h"

#include "images/rt_2s_p_16x16.h"
#include "images/rt_2p_sl_16x16.h"
#include "images/rt_s_2p_16x16.h"
#include "images/rt_parallel_16x16.h"
#include "images/rt_serial_16x16.h"
#include "images/mode_preset_16x16.h"
#include "images/mode_pedalboard_16x16.h"
#include "images/mode_stompbox_16x16.h"
#include "images/mode_parameter_16x16.h"
#include "images/mode_looper_16x16.h"
#include "fonts/TomThumb.h"
#include "ln.h"
#include "stringFunctions.h"
}

#include "pipicofx/MultiAudioProcessor.hpp"


extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
extern LooperDataType looper;
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern FxPresetType presets[3];
extern volatile float avgOutOld,avgInOld;
extern volatile uint32_t cpuLoad;

inline void drawThreePxLine(uint8_t px,uint8_t py,BwImageType*img)
{
    setPixel(px,py,img);
    setPixel(px+1,py,img);
    setPixel(px+2,py,img);
}

inline void removeEdgePixels(uint8_t px, uint8_t py,BwImageType*img)
{
    clearPixel(px+1,py,img);
    clearPixel(px,py+1,img);
    clearPixel(px+2,py+1,img);
    clearPixel(px+1,py+2,img);
}

inline void drawPlay(uint8_t px,uint8_t py,BwImageType*img)
{
    setPixel(px,py,img);
    setPixel(px,py+1,img);
    setPixel(px,py+2,img);
    setPixel(px,py+3,img);
    setPixel(px,py+4,img);
    setPixel(px+1,py+1,img);
    setPixel(px+1,py+2,img);
    setPixel(px+1,py+3,img);
    setPixel(px+2,py+2,img);
}

inline void drawStop(uint8_t px,uint8_t py,uint8_t hasContent,BwImageType*img)
{
    drawSquareInt(px,py,px+4,py+4,img);
    if (!hasContent)
    {
        clearSquareInt(px+1,py+1,px+3,py+3,img);
    }
}

inline void drawRecord(uint8_t px, uint8_t py,uint8_t hasContent,BwImageType*img)
{
    setPixel(px+1,py,img);
    setPixel(px+2,py,img);
    setPixel(px,py+1,img);
    setPixel(px,py+2,img);
    setPixel(px+1,py+3,img);
    setPixel(px+2,py+3,img);
    setPixel(px+3,py+1,img);
    setPixel(px+3,py+2,img);
    if (hasContent)
    {
        setPixel(px+1,py+1,img);
        setPixel(px+1,py+2,img);
        setPixel(px+2,py+1,img);
        setPixel(px+2,py+2,img);
    }
}

/* draws the bottom panel */
void drawBottomPanel(BwImageType* imgBuffer)
{
    uint8_t offsetX=128-16;
    char lineBfr[24];
    for(uint8_t c=0;c<24;c++)
    {
        c[lineBfr]=0;
    }
    clearImage(imgBuffer);

    // rightmost part, shows the routing, and each effect as 
    // as a 3*3 pixel bpos
    //  ***           * *      ***       ***
    //  * *                    ***       * * 
    //  ***           * *      ***       ***
    //                                   ***
    // off,present   none       on    off, selected     
    //
    switch(audioProcessor.getRouting())
    {
        case MULTI_AUDIO_PROCESSOR_ROUTING_3S:
            drawImage(offsetX,0,&rt_serial_16x16_streamimg,imgBuffer);
            if (ui.currentProgramPosition == 0)
            {
                drawThreePxLine(offsetX + 2,11,imgBuffer);
            }
            else if (ui.currentProgramPosition == 1)
            {
                drawThreePxLine(offsetX + 6,11,imgBuffer);
            }
            else if (ui.currentProgramPosition == 2)
            {
                drawThreePxLine(offsetX + 10,11,imgBuffer);
            }
            // fill square if effect is on
            if (audioProcessor.getFxProgram(0) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                setPixel(offsetX+3,8,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                setPixel(offsetX+7,8,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                setPixel(offsetX+11,8,imgBuffer);
            }
            // remove edge pixels of effect isn't present 
            if (audioProcessor.getFxProgram(0) == nullptr )
            {
                removeEdgePixels(offsetX+2,7,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) == nullptr )
            {
                removeEdgePixels(offsetX+6,7,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) == nullptr )
            {
                removeEdgePixels(offsetX+10,7,imgBuffer);
            }
            break; 
        case MULTI_AUDIO_PROCESSOR_ROUTING_3P:
            drawImage(offsetX,0,&rt_parallel_16x16_streamimg,imgBuffer);
            if (ui.currentProgramPosition == 0)
            {
                drawThreePxLine(offsetX + 6,4,imgBuffer);
            }
            else if (ui.currentProgramPosition == 1)
            {
                drawThreePxLine(offsetX + 6,10,imgBuffer);
            }
            else if (ui.currentProgramPosition == 2)
            {
                drawThreePxLine(offsetX + 6,15,imgBuffer);
            }
            // fill square if effect is on
            if (audioProcessor.getFxProgram(0) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                setPixel(offsetX+7,1,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                setPixel(offsetX+7,7,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                setPixel(offsetX+7,13,imgBuffer);
            }
            // remove edge pixels of effect isn't present 
            if (audioProcessor.getFxProgram(0) == nullptr )
            {
                removeEdgePixels(offsetX+6,0,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) == nullptr )
            {
                removeEdgePixels(offsetX+6,6,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) == nullptr )
            {
                removeEdgePixels(offsetX+6,12,imgBuffer);
            }
            break; 
        case MULTI_AUDIO_PROCESSOR_ROUTING_2P_S:
            drawImage(offsetX,0,&rt_2p_sl_16x16_streamimg,imgBuffer);
            if (ui.currentProgramPosition == 0)
            {
                drawThreePxLine(offsetX + 3,6,imgBuffer);
            }
            else if (ui.currentProgramPosition == 1)
            {
                drawThreePxLine(offsetX + 3,14,imgBuffer);
            }
            else if (ui.currentProgramPosition == 2)
            {
                drawThreePxLine(offsetX + 9,10,imgBuffer);
            }
            // fill square if effect is on
            if (audioProcessor.getFxProgram(0) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                setPixel(offsetX+4,3,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                setPixel(offsetX+4,11,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                setPixel(offsetX+10,7,imgBuffer);
            }
            // remove edge pixels of effect isn't present 
            if (audioProcessor.getFxProgram(0) == nullptr )
            {
                removeEdgePixels(offsetX+3,2,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) == nullptr )
            {
                removeEdgePixels(offsetX+3,10,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) == nullptr )
            {
                removeEdgePixels(offsetX+9,6,imgBuffer);
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_S_2P:
            drawImage(offsetX,0,&rt_s_2p_16x16_streamimg,imgBuffer);
            if (ui.currentProgramPosition == 0)
            {
                drawThreePxLine(offsetX + 1,10,imgBuffer);
            }
            else if (ui.currentProgramPosition == 1)
            {
                drawThreePxLine(offsetX + 7,6,imgBuffer);
            }
            else if (ui.currentProgramPosition == 2)
            {
                drawThreePxLine(offsetX + 7,14,imgBuffer);
            }
            // fill square if effect is on
            if (audioProcessor.getFxProgram(0) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                setPixel(offsetX+2,7,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                setPixel(offsetX+8,3,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                setPixel(offsetX+8,11,imgBuffer);
            }
            // remove edge pixels of effect isn't present 
            if (audioProcessor.getFxProgram(0) == nullptr )
            {
                removeEdgePixels(offsetX+1,6,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) == nullptr )
            {
                removeEdgePixels(offsetX+7,2,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) == nullptr )
            {
                removeEdgePixels(offsetX+7,10,imgBuffer);
            }
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2S_P:
            drawImage(offsetX,0,&rt_2s_p_16x16_streamimg,imgBuffer);
            if (ui.currentProgramPosition == 0)
            {
                drawThreePxLine(offsetX + 3,5,imgBuffer);
            }
            else if (ui.currentProgramPosition == 1)
            {
                drawThreePxLine(offsetX + 8,5,imgBuffer);
            }
            else if (ui.currentProgramPosition == 2)
            {
                drawThreePxLine(offsetX + 5,14,imgBuffer);
            }
            // fill square if effect is on
            if (audioProcessor.getFxProgram(0) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                setPixel(offsetX+4,2,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                setPixel(offsetX+9,2,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) != nullptr && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                setPixel(offsetX+6,11,imgBuffer);
            }
            // remove edge pixels of effect isn't present 
            if (audioProcessor.getFxProgram(0) == nullptr )
            {
                removeEdgePixels(offsetX+3,1,imgBuffer);
            }
            if (audioProcessor.getFxProgram(1) == nullptr )
            {
                removeEdgePixels(offsetX+8,1,imgBuffer);
            }
            if (audioProcessor.getFxProgram(2) == nullptr )
            {
                removeEdgePixels(offsetX+5,10,imgBuffer);
            }            
            break;
    }
    offsetX -= 16;
    // draw current preset and bank
    if (currentBank != 0xFF)
    {
        appendToString(lineBfr,"B");
        UInt8ToChar(currentBank, lineBfr+1);
        drawText(offsetX+1,8,lineBfr,imgBuffer,0);
    }
    if (currentPreset != 0xFF)
    {
        lineBfr[0]=0;
        lineBfr[2]=0;
        appendToString(lineBfr,"P");
        UInt8ToChar(currentPreset, lineBfr+1);
        drawText(offsetX+1,16,lineBfr,imgBuffer,0);
    }
    offsetX -= 16;
    // draw mode symbols
    switch (ui.mode)
    {
        case PPFX_MODE_PRESETS:
            drawImage(offsetX,0,&mode_preset_16x16_streamimg,imgBuffer);
            break;
        case PPFX_MODE_PEDALBOARD:
            drawImage(offsetX,0,&mode_pedalboard_16x16_streamimg,imgBuffer);
            break;
        case PPFX_MODE_STOMPBOX:
            drawImage(offsetX,0,&mode_stompbox_16x16_streamimg,imgBuffer);
            break;
        case PPFX_MODE_EDITPARAM:
            drawImage(offsetX,0,&mode_parameter_16x16_streamimg,imgBuffer);
            break;
        case PPFX_MODE_LOOPER:
            drawImage(offsetX,0,&mode_looper_16x16_streamimg,imgBuffer);
            break;
    }
    offsetX -= 16;
    // draw state of the looper(s)
    if (looper.looperState == LOOPER_STATE_STOPPED && looper.indexEnd != LOOPER_INDEX_NULL)
    {
        drawStop(offsetX+1,1,true,imgBuffer);
    }
    else if (looper.looperState == LOOPER_STATE_STOPPED && looper.indexEnd == LOOPER_INDEX_NULL)
    {
        drawStop(offsetX+1,1,false,imgBuffer);
    }
    else if (looper.looperState == LOOPER_STATE_RECORDING)
    {
        drawRecord(offsetX+1,1,false,imgBuffer);
    }
    else if (looper.looperState == LOOPER_STATE_OVERDUBBING)
    {
        drawRecord(offsetX+1,1,true,imgBuffer);
    }
    else
    {
        drawPlay(offsetX+1,0,imgBuffer);
    }
    // draw preset name and levels
    offsetX = 0;
    *lineBfr=0;
    const GFXfont * font =  getGFXFont(TOMTHUMB);
    if (currentPreset != 0xFF)
    {
        appendToString(lineBfr,presets[currentPreset].name);
        drawText(offsetX,1,lineBfr,imgBuffer,font);
    }
    float dbVal;
    if (avgInOld > 0.0f)
    {
        dbVal = (toDb(avgInOld)+120.0f)/120.f*64.0f;
        if (dbVal >= 0.0f)
        {
            drawSquareInt(0,8,(uint8_t)dbVal,10,imgBuffer);
        }
    }
    if (avgOutOld > 0.0f)
    {
        dbVal = (toDb(avgOutOld)+120.0f)/120.f*64.0f;
        if (dbVal >= 0.0f)
        {
            drawSquareInt(0,11,(uint8_t)dbVal,13,imgBuffer);
        }
    }
    drawSquareInt(0,14,cpuLoad>>2,16,imgBuffer);
}