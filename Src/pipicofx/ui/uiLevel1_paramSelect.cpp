extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
}
#include "pipicofx/MultiAudioProcessor.hpp"

/**
 * Submode "Select Parameter", allow viewing the parameters of the program/effect
 * currently in focus
 * Rotary: changes between parameters
 * Enter: switch to Submode "Edit Parameter" if parameter is not controlled via knob
 * Exit: return to previous mode, stores the values in the preset structure in memory at the current position
 * Knobs: edit parameters assigned to knobs
 */

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 

static void create()
{
    char lineBfr[24];
    for (uint8_t c=0;c<24;c++)
    {
        lineBfr[c]=0;
    }
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    drawText(0,8,((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getName(),imgBuffer,0);
    *lineBfr=0;
    appendToString(lineBfr,ui.currentParameter->getParameterName());
    appendToStringUntil(lineBfr,"               ",20);
    UInt8ToChar(ui.currentParameterIdx+1,lineBfr+17);
    appendToString(lineBfr+17,"/");
    UInt8ToChar(((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount(),lineBfr+19);

    drawText(0,16,lineBfr,imgBuffer,0);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    BwImageType * img=getImageBuffer();
    clearSquareInt(0,16,128,64,img);
    float fValue,fMaxValue,fMinValue;
    char paramValueBfr[16];
    float cx,cy,px,py;
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;


    fValue = int2float((int32_t)ui.currentParameter->rawValue);
    fMaxValue = int2float((int32_t)(1 << 12));
    fMinValue = int2float((int32_t)0);
    fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
    // center is at (51+13(/(24+16)
    px = 64.0f - fsin(fValue)*14.0f;
    py = 40.0f + fcos(fValue)*14.0f;
    cx = 64.0f;
    cy = 40.0f;
    drawImage(13,16,&pipicofx_param_1_scaled_streamimg,img);
    drawLine(cx,cy,px,py,img);
    ui.currentParameter->parameterDisplay(paramValueBfr);
    drawText(0,64,paramValueBfr,img,0);
}

static void enterCallback() 
{
    if (ui.currentParameter->getControl() == 0xFF || ui.editViaRotary != 0)
    {
        uiStackPush(1);
        enterLevel2();
    }
}

static void exitCallback()
{
    parametersToPreset(presets+currentPreset,((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition)),ui.currentProgramPosition);
}

static inline void knobCallback(uint16_t val,uint8_t control)
{
    if (ui.locked == 0)
    {
        for (uint8_t c=0;c<((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount();c++)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter(c)->getControl()==control)
            {
                ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter(c)->parameterCallback(val);
            }
        }  
    } 
}


static void knob0Callback(uint16_t val)
{
    knobCallback(val,0);
}

static void knob1Callback(uint16_t val)
{
    knobCallback(val,1);
}

static void knob2Callback(uint16_t val)
{
    knobCallback(val,2);
}


static void rotaryCallback(int16_t encoderDelta)
{
    if (encoderDelta > 1)
    {
        encoderDelta = 1;
    }
    else if (encoderDelta < -1)
    {
        encoderDelta = -1;
    }
    ui.currentParameterIdx += encoderDelta;
    if (ui.currentParameterIdx >= ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount() && encoderDelta > 0)
    {
        ui.currentParameterIdx=((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount()-1;
    }
    else if (ui.currentParameterIdx >= ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount() && encoderDelta < 0)
    {
        ui.currentParameterIdx = 0;
    }
    ui.currentParameter = ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter(ui.currentParameterIdx);
    create();
}

void enterLevel1()
{
    //clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    create();
}