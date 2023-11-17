#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "oled_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"


extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;

static void create(PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    drawText(0,8,data->currentProgram->name,imgBuffer,0);
    drawText(0,16,data->currentParameter->name,imgBuffer,0);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType * img = getImageBuffer();
    clearSquareInt(0,16,128,64,img);
    float fValue,fMaxValue,fMinValue;
    char paramValueBfr[16];
    float cx,cy,px,py;
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;
    fValue = int2float((int32_t)data->currentParameter->rawValue);
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
    data->currentParameter->getParameterDisplay(data->currentProgram->data,paramValueBfr);
    drawText(0,64,paramValueBfr,img,0);
    OledwriteFramebufferAsync(img->data);
}


static void exitCallback(PiPicoFxUiType*data)
{
    // none
}

static inline void knobCallback(uint16_t val,PiPicoFxUiType*data,uint8_t control)
{
    if (data->locked == 0)
    {
        for (uint8_t c=0;c<data->currentProgram->nParameters;c++)
        {
            if (data->currentProgram->parameters[c].control==control)
            {
                data->currentProgram->parameters[c].setParameter(val,data->currentProgram->data);
                data->currentProgram->parameters[c].rawValue = (int16_t)val;

            }
        }  
    } 
}


static void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,0);
}

static void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,1);
}

static void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,2);
}


static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    data->currentParameter->rawValue += encoderDelta*data->currentParameter->increment;
    if (data->currentParameter->rawValue < 0)
    {
        data->currentParameter->rawValue = 0;
    }
    else if  (data->currentParameter->rawValue > ((1 << 12)-1))
    {
        data->currentParameter->rawValue = ((1 << 12)-1);
    }
    data->currentParameter->setParameter(data->currentParameter->rawValue,data->currentProgram->data);
}

void enterLevel2(PiPicoFxUiType*data)
{
    clearCallbackAssignments();
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    create(data);
}