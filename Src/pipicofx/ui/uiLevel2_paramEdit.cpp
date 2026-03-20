extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
}
#include "pipicofx/MultiAudioProcessor.hpp"

/**
 * Submode "Parameter Edit": Allows to change the values of the parameters in focus of the program
 * currently in focus
 * Rotary: Change parameter value
 * Exit: return to previous mode 
 * Exit: return to previous mode
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

    appendToString(lineBfr,ui.currentParameter->getParameterName());
    appendToStringUntil(lineBfr,"               ",20);
    UInt8ToChar(ui.currentParameterIdx+1,lineBfr+17);
    appendToString(lineBfr+17,"/");
    UInt8ToChar(((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount(),lineBfr+19);

    drawText(0,16,lineBfr,imgBuffer,0);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    BwImageType * img = getImageBuffer();
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
    // center is at (51+13)/(24+16)
    px = 64.0f - fsin(fValue)*14.0f;
    py = 40.0f + fcos(fValue)*14.0f;
    cx = 64.0f;
    cy = 40.0f;
    drawImage(13,16,&pipicofx_param_2_scaled_streamimg,img);
    drawLine(cx,cy,px,py,img);   
    ui.currentParameter->parameterDisplay(paramValueBfr);
    drawText(0,64,paramValueBfr,img,0);
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
    if (ui.currentParameter->increment > 1) // handle parameter whose increments are
    {                                          // larger than one as discrete param, thus increment only by one
        if (encoderDelta > 1)
        {
            encoderDelta = 1;
        }
        else if (encoderDelta < -1)
        {
            encoderDelta = -1;
        }
    }
    ui.currentParameter->rawValue += encoderDelta*ui.currentParameter->increment;
    if (ui.currentParameter->rawValue < 0)
    {
        ui.currentParameter->rawValue = 0;
    }
    else if  (ui.currentParameter->rawValue > ((1 << 12)-1))
    {
        ui.currentParameter->rawValue = ((1 << 12)-1);
    }
    ui.currentParameter->parameterCallback(ui.currentParameter->rawValue);
}

void enterLevel2()
{
    clearCallbackAssignments();
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    create();
}