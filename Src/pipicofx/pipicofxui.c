#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "ssd1306_display.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "romfunc.h"
#include "audio/fxprogram/fxProgram.h"

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
            ssd1306WriteText(paramValueBfr,0,7);

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
            ssd1306WriteText(paramValueBfr,0,7);

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
    uint8_t paramIdxesDrawn[3]={0,0,0};
    switch(data->displayLevel)
    {
        case 0:
            ssd1306WriteText(data->currentProgram->name,0,0);
            if (data->locked != 0)
            {
                ssd1306DisplayByteArray(0,122,locksymbol,5);
            }
            ssd1306WriteText("                   ",0,1);
            ssd1306WriteText("                   ",0,2);
            ssd1306WriteText("                   ",0,3);
            for (uint8_t c=0;c<data->currentProgram->nParameters;c++)
            {
                if (data->currentProgram->parameters[c].control == 0)
                {
                    ssd1306WriteText("P1:",0,4);
                    ssd1306WriteText(data->currentProgram->parameters[c].name,3,4);
                    paramIdxesDrawn[0]=1;
                }
                if (data->currentProgram->parameters[c].control == 1)
                {
                    ssd1306WriteText("P2:",0,5);
                    ssd1306WriteText(data->currentProgram->parameters[c].name,3,5);
                    paramIdxesDrawn[1]=1;
                }
                if (data->currentProgram->parameters[c].control == 2)
                {
                    ssd1306WriteText("P3:",0,6);
                    ssd1306WriteText(data->currentProgram->parameters[c].name,3,6);
                    paramIdxesDrawn[2]=1;
                }                
            }
            for (uint8_t c=0;c<3;c++)
            {
                if (paramIdxesDrawn[c]==0)
                {
                    ssd1306WriteText("                   ",0,c+4);
                }
            }
            ssd1306WriteText("                   ",0,7);
            break;
        case 1:
            ssd1306WriteText(data->currentProgram->name,0,0);
            ssd1306WriteText(data->currentParameter->name,0,1);
            ssd1306WriteText("                   ",0,2);
            ssd1306WriteText("                   ",0,3);
            ssd1306WriteText("                   ",0,4);
            ssd1306WriteText("                   ",0,5);
            ssd1306WriteText("                   ",0,6);
            ssd1306WriteText("                   ",0,7);
            break;
        case 2:
            ssd1306WriteText(data->currentProgram->name,0,0);
            ssd1306WriteText(data->currentParameter->name,0,1);
            ssd1306WriteText("                   ",0,2);    
            ssd1306WriteText("                   ",0,3);
            ssd1306WriteText("                   ",0,4);
            ssd1306WriteText("                   ",0,5);
            ssd1306WriteText("                   ",0,6);
            ssd1306WriteText("                   ",0,7);
            break;
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

void button1Callback(PiPicoFxUiType*data)
{
    switch (data->displayLevel)
    {
        case 0: // UI Level 0: to level 1
            data->displayLevel = 1;
            drawUi(data);
            break;
        case 1: // UI Level 1: go to  level 2 if the parameter is not controlled by knob
            if (data->currentParameter->control == 0xFF)
            {
                data->oldParamValue = data->currentParameter->rawValue;
                data->displayLevel = 2;
                drawUi(data);
            }
            break;
        case 2: // UI Level 2: apply/do not revert and go back to level 1
            data->displayLevel = 1;
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
        case 2: // UI Level 2, revert and go back to level 1
            data->currentParameter->rawValue = data->oldParamValue;
            data->displayLevel = 1;
            drawUi(data);
            break;
        default:
            break;
    }
}

void rotaryCallback(uint32_t encoderValue,PiPicoFxUiType*data)
{
    int32_t diff = (encoderValue - data->oldEncoderValue);
    if (diff > 0)
    {
        diff = 1;
    }
    else
    {
        diff = -1;
    }
    if (data->locked == 0 && diff != 0)
    {
        switch(data->displayLevel)
        {
            case 0: // UI Level 0 
                //switch Programs
                data->currentProgramIdx += diff;
                if (data->currentProgramIdx >= N_FX_PROGRAMS && diff > 0)
                {
                    data->currentProgramIdx = N_FX_PROGRAMS-1;
                } 
                else if (data->currentProgramIdx >= N_FX_PROGRAMS && diff < 0)
                {
                    data->currentProgramIdx = 0;
                }
                data->currentProgram = fxPrograms[data->currentProgramIdx];
                data->currentParameterIdx=0;
                data->currentParameter = data->currentProgram->parameters;
                drawUi(data);
                break;
            case 1: // UI Level 1, change parameter
                data->currentParameterIdx += diff;
                if (data->currentParameterIdx >= data->currentProgram->nParameters && diff > 0)
                {
                    data->currentParameterIdx=data->currentProgram->nParameters-1;
                }
                else if (data->currentParameterIdx >= data->currentProgram->nParameters && diff < 0)
                {
                    data->currentParameterIdx = 0;
                }
                data->currentParameter = data->currentProgram->parameters + data->currentParameterIdx;
                drawUi(data);
                break;
            case 2: // UI Level 2, change Parameter Value
                // TODO increase by defined increment
                data->currentParameter->rawValue += diff;
                if (data->currentParameter->rawValue < 0)
                {
                    data->currentParameter->rawValue = 0;
                }
                else if  (data->currentParameter->rawValue > ((1 << 12)-1))
                {
                    data->currentParameter->rawValue = ((1 << 12)-1);
                }
                data->currentParameter->setParameter(data->currentParameter->rawValue,data->currentProgram->data);
                drawUi(data);
                break;
        }
        data->oldEncoderValue=encoderValue;
    }
}


PiPicoFxUiType piPicoUiController;

void piPicoFxUiSetup()
{
    piPicoUiController.currentProgram=fxPrograms[0];
    piPicoUiController.currentProgramIdx=0;
    piPicoUiController.currentParameter=fxPrograms[piPicoUiController.currentProgramIdx]->parameters;
    piPicoUiController.currentParameterIdx=0;
    piPicoUiController.displayLevel=0;
    piPicoUiController.locked=0;
    piPicoUiController.oldEncoderValue= 0x7FFFFFFF;
    piPicoUiController.oldParamValue=0;

}