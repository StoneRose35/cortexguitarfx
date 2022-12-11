#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "romfunc.h"
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

uint8_t locksymbol[5]={0b01111000,0b01111110,0b01111001,0b01111110,0b01111000 };

/**
 * @brief called at regular intervals to update the diplay
 * 
 * @param avgInput 
 * @param avgOutput 
 * @param cpuLoad 
 * @param data 
 */
void updateAudioUi(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    uint8_t bargraphBuffer[128];
    char paramValueBfr[16];
    BwImageBufferType imgBfr;
    float fValue,fMaxValue,fMinValue;
    float cx,cy,px,py;
    switch(data->displayLevel)
    {
        case 0:
            // show basic display
            for (uint8_t c=0;c<128;c++)
            {
                if (c<=avgInput)
                {
                    bargraphBuffer[c] = 126;
                }
                else
                {
                    bargraphBuffer[c] = 0;
                }
            }
            ssd1306DisplayByteArray(1,0,bargraphBuffer,128);

            for (uint8_t c=0;c<128;c++)
            {
                if (c<=avgOutput)
                {
                    bargraphBuffer[c] = 126;
                }
                else
                {
                    bargraphBuffer[c] = 0;
                }
            }
            ssd1306DisplayByteArray(2,0,bargraphBuffer,128);

            for (uint8_t c=0;c<128;c++)
            {
                if (c<=cpuLoad)
                {
                    bargraphBuffer[c] = 126;
                }
                else
                {
                    bargraphBuffer[c] = 0;
                }
            }
            ssd1306DisplayByteArray(3,0,bargraphBuffer,128);
            break;
        case 1:
            imgBfr.sx=pipicofx_param_1_scaled_streamimg.sx;
            imgBfr.sy=pipicofx_param_1_scaled_streamimg.sy;
            for (uint16_t c=0;c<510;c++)
            {
                imgBfr.data[c]=pipicofx_param_1_scaled_streamimg.data[c];
            }
            fValue = int2float((int32_t)data->currentParameter->rawValue);
            fMaxValue = int2float((int32_t)(1 << 12));
            fMinValue = int2float((int32_t)0);
            fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
            // center is at 51/24
            px = 51.0f - fsin(fValue)*14.0f;
            py = 24.0f + fcos(fValue)*14.0f;
            cx = 51.0f;
            cy = 24.0f;
            drawLine(cx,cy,px,py,&imgBfr);
            ssd1306DisplayImageStandardAdressing(13,2,imgBfr.sx,imgBfr.sy>>3,imgBfr.data); 
            data->currentParameter->getParameterDisplay(data->currentProgram->data,paramValueBfr);
            ssd1306WriteTextLine(paramValueBfr,7);

            break;
        case 2:
            imgBfr.sx=pipicofx_param_2_scaled_streamimg.sx;
            imgBfr.sy=pipicofx_param_2_scaled_streamimg.sy;
            for (uint16_t c=0;c<510;c++)
            {
                imgBfr.data[c]=pipicofx_param_2_scaled_streamimg.data[c];
            }
            fValue = int2float((int32_t)data->currentParameter->rawValue);
            fMaxValue = int2float((int32_t)(1 << 12));
            fMinValue = int2float((int32_t)0);
            fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
            // center is at 51/24
            px = 51.0f - fsin(fValue)*14.0f;
            py = 24.0f + fcos(fValue)*14.0f;
            cx = 51.0f;
            cy = 24.0f;
            drawLine(cx,cy,px,py,&imgBfr);   
            ssd1306DisplayImageStandardAdressing(13,2,imgBfr.sx,imgBfr.sy>>3,imgBfr.data); 
            data->currentParameter->getParameterDisplay(data->currentProgram->data,paramValueBfr);
            ssd1306WriteTextLine(paramValueBfr,7);

            break;
    }
}

/**
 * @brief draws the entire screen except the dynamic contents (input/output levels, cpu load)
 * 
 * @param data 
 */
void drawUi(PiPicoFxUiType*data)
{                  
    char lineBuffer[24];
    lineBuffer[0]=0;
    uint8_t paramIdxesDrawn[3]={0,0,0};
    switch(data->displayLevel)
    {
        case 0:
            ssd1306WriteTextLine(data->currentProgram->name,0);
            if (data->locked != 0)
            {
                ssd1306DisplayByteArray(0,122,locksymbol,5);
            }
            ssd1306WriteTextLine(" ",1);
            ssd1306WriteTextLine(" ",2);
            ssd1306WriteTextLine(" ",3);
            for (uint8_t c=0;c<data->currentProgram->nParameters;c++)
            {
                if (data->currentProgram->parameters[c].control == 0)
                {
                    lineBuffer[0]=0;
                    appendToString(lineBuffer,"P1:");
                    appendToString(lineBuffer,data->currentProgram->parameters[c].name);
                    ssd1306WriteTextLine(lineBuffer,4);
                    paramIdxesDrawn[0]=1;
                }
                if (data->currentProgram->parameters[c].control == 1)
                {
                    lineBuffer[0]=0;
                    appendToString(lineBuffer,"P2:");
                    appendToString(lineBuffer,data->currentProgram->parameters[c].name);
                    ssd1306WriteTextLine(lineBuffer,5);
                    paramIdxesDrawn[1]=1;
                }
                if (data->currentProgram->parameters[c].control == 2)
                {
                    lineBuffer[0]=0;
                    appendToString(lineBuffer,"P3:");
                    appendToString(lineBuffer,data->currentProgram->parameters[c].name);
                    ssd1306WriteTextLine(lineBuffer,6);
                    paramIdxesDrawn[2]=1;
                }                
            }
            for (uint8_t c=0;c<3;c++)
            {
                if (paramIdxesDrawn[c]==0)
                {
                    ssd1306WriteTextLine(" ",c+4);
                }
            }
            ssd1306WriteTextLine(" ",7);
            break;
        case 1:
            ssd1306WriteTextLine(data->currentProgram->name,0);
            ssd1306WriteTextLine(data->currentParameter->name,1);
            ssd1306WriteTextLine(" ",2);
            ssd1306WriteTextLine(" ",3);
            ssd1306WriteTextLine(" ",4);
            ssd1306WriteTextLine(" ",5);
            ssd1306WriteTextLine(" ",6);
            ssd1306WriteTextLine(" ",7);
            break;
        case 2:
            ssd1306WriteTextLine(data->currentProgram->name,0);
            ssd1306WriteTextLine(data->currentParameter->name,1);
            ssd1306WriteTextLine(" ",2);    
            ssd1306WriteTextLine(" ",3);
            ssd1306WriteTextLine(" ",4);
            ssd1306WriteTextLine(" ",5);
            ssd1306WriteTextLine(" ",6);
            ssd1306WriteTextLine(" ",7);
            break;
    }
}

inline void knobCallback(uint16_t val,PiPicoFxUiType*data,uint8_t control)
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

void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,0);
}

void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,1);
}

void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
    knobCallback(val,data,2);
}

/**
 * @brief Calback for the "Enter"-Button
 * 
 * @param data 
 */
void button1Callback(PiPicoFxUiType*data) 
{
    switch (data->displayLevel)
    {
        case 0: // UI Level 0: to level 1 if the program contains at least one paramenter
            if (data->currentProgram->nParameters > 0)
            {
                data->displayLevel = 1;
                drawUi(data);
            }
            break;
        case 1: // UI Level 1: go to  level 2 if the parameter is not controlled by knob
            if (data->currentParameter->control == 0xFF)
            {
                data->oldParamValue = data->currentParameter->rawValue;
                data->displayLevel = 2;
                drawUi(data);
            }
            break;
        case 2: // UI Level 2: do nothing
            break;
        default:
            break;
    }
}

void button2Callback(PiPicoFxUiType*data)
{
    switch (data->displayLevel)
    {
        case 0: // UI Level 0, lock/unlock
            data->locked ^= 0x1;
            drawUi(data);
            break;
        case 1: // UI Level 1, go back to level 0
            data->displayLevel = 0;
            drawUi(data);
            break;
        case 2: // UI Level 2, apply and go back to level 1
            data->displayLevel = 1;
            drawUi(data);
            break;
        default:
            break;
    }
}

void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    uint16_t knobVal;
    if (data->locked == 0 && encoderDelta != 0)
    {
        switch(data->displayLevel)
        {
            case 0: // UI Level 0 
                //switch Programs
                data->currentProgramIdx += encoderDelta;
                if (data->currentProgramIdx >= N_FX_PROGRAMS && encoderDelta > 0)
                {
                    data->currentProgramIdx = N_FX_PROGRAMS-1;
                } 
                else if (data->currentProgramIdx >= N_FX_PROGRAMS && encoderDelta < 0)
                {
                    data->currentProgramIdx = 0;
                }
                data->currentProgram = fxPrograms[data->currentProgramIdx];
                data->currentParameterIdx=0;
                data->currentParameter = data->currentProgram->parameters;
                // set all parameters controlled by the pots to the current value
                for(uint8_t c=0; c < data->currentProgram->nParameters; c++)
                {
                    switch (data->currentProgram->parameters[c].control)
                    {
                        case 0:
                            knobVal = getChannel0Value();
                            knob0Callback(knobVal,data);
                            break;
                        case 1:
                            knobVal = getChannel1Value();
                            knob1Callback(knobVal,data);
                            break;
                        case 2:
                            knobVal = getChannel2Value();
                            knob2Callback(knobVal,data);
                            break;
                        default:
                            break;
                    }
                }
                drawUi(data);
                break;
            case 1: // UI Level 1, change parameter
                data->currentParameterIdx += encoderDelta;
                if (data->currentParameterIdx >= data->currentProgram->nParameters && encoderDelta > 0)
                {
                    data->currentParameterIdx=data->currentProgram->nParameters-1;
                }
                else if (data->currentParameterIdx >= data->currentProgram->nParameters && encoderDelta < 0)
                {
                    data->currentParameterIdx = 0;
                }
                data->currentParameter = data->currentProgram->parameters + data->currentParameterIdx;
                drawUi(data);
                break;
            case 2: // UI Level 2, change Parameter Value
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
                break;
        }
    }
}


void piPicoFxUiSetup(PiPicoFxUiType* piPicoUiController)
{
    piPicoUiController->currentProgram=fxPrograms[0];
    piPicoUiController->currentProgramIdx=0;
    piPicoUiController->currentParameter=fxPrograms[piPicoUiController->currentProgramIdx]->parameters;
    piPicoUiController->currentParameterIdx=0;
    piPicoUiController->displayLevel=0;
    piPicoUiController->locked=0;
    piPicoUiController->oldEncoderValue= 0x7FFFFFFF;
    piPicoUiController->oldParamValue=0;

}