#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "stompswitches.h"

const uint8_t locksymbol[5]={0b01111000,0b01111110,0b01111001,0b01111110,0b01111000 };
extern volatile uint8_t programsToInitialize[3];
extern volatile uint8_t programChangeState;
extern const uint8_t stompswitch_progs[];
extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;

static void create(PiPicoFxUiType*data)
{
    char lineBuffer[24];
    lineBuffer[0]=0;
    uint8_t paramIdxesDrawn[3]={0,0,0};
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
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    uint8_t bargraphBuffer[128];
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

static void enterCallback(PiPicoFxUiType*data) 
{
    uiStackPush(data, 0);
    enterLevel1(data);
}

static void exitCallback(PiPicoFxUiType*data)
{

    // apply current program to preset when coming from 4
    if(uiStackCurrent(data)==4)
    {
        presets[currentPreset].programNr = data->currentProgramIdx;
    }
    else
    {
        data->locked ^= 0x1;
        create(data);
    }

}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    uint16_t knobVal;
    if (encoderDelta != 0)
    {
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
    }
    create(data);
}

static void genericStompSwitchCallback(uint8_t switchNr, PiPicoFxUiType* data)
{
    //if (data->currentProgramIdx != stompswitch_progs[switchNr]) // switch pressed an program not activated 
    //{
        /*switch (data->currentProgramIdx)
        {
            case 8:
                setStompswitchColor(0,0);
                break;
            case 7:
                setStompswitchColor(1,0);
                break;
            case 1:
                setStompswitchColor(2,0);
                break;                        

        }*/
        //programsToInitialize[switchNr]=1;
        //programChangeState=1;
        /*data->currentProgramIdx = stompswitch_progs[switchNr];
        data->currentProgram = fxPrograms[data->currentProgramIdx];
        data->currentParameterIdx=0;
        data->currentParameter = data->currentProgram->parameters;
        */
        //setStompswitchColorRaw(3 << (switchNr << 1));
    //}
    uiStackPush(data, 0);
    enterLevel3(data);
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
    genericStompSwitchCallback(0,data);
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
    genericStompSwitchCallback(1,data);
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
    genericStompSwitchCallback(2,data);
}


/*
register exit, rotary, knobs and stompswitch callbacks
remove enter callback
register onUpdate, on Create
*/
void enterLevel0(PiPicoFxUiType*data)
{
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}
