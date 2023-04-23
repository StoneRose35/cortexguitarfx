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

const uint8_t locksymbol[5]={0b01111000,0b01111110,0b01111001,0b01111110,0b01111000 };

/*
Callback function pointers
*/
static void (*enterButtonPressedCallback)(PiPicoFxUiType *ui)=0; 
static void (*enterButtonReleasedCallback)(PiPicoFxUiType* ui)=0; 
static void (*exitButtonPressedCallback)(PiPicoFxUiType* ui)=0;
static void (*exitButtonReleasedCallback)(PiPicoFxUiType* ui)=0;
static void (*rotaryCallback)(uint16_t val,PiPicoFxUiType* ui)=0;
static void (*knob0Callback)(uint16_t val,PiPicoFxUiType* ui)=0;
static void (*knob1Callback)(uint16_t val,PiPicoFxUiType* ui)=0;
static void (*knob2Callback)(uint16_t val,PiPicoFxUiType* ui)=0;
static void (*stompSwitch1PressedCallback)(PiPicoFxUiType* ui)=0;
static void (*stompSwitch1ReleasedCallback)(PiPicoFxUiType* ui)=0;
static void (*stompSwitch2PressedCallback)(PiPicoFxUiType* ui)=0;
static void (*stompSwitch2ReleasedCallback)(PiPicoFxUiType* ui)=0;
static void (*stompSwitch3PressedCallback)(PiPicoFxUiType* ui)=0;
static void (*stompSwitch3ReleasedCallback)(PiPicoFxUiType* ui)=0;
static void (*onUpdateCallback)(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)=0;
static void (*onCreateCallback)(PiPicoFxUiType* ui)=0;

/*
 registration functions, used to attach a certain functionality to a
 ui element callback
 */
void registerEnterButtonPressedCallback(void(*cb)(PiPicoFxUiType*))
{
    enterButtonPressedCallback=cb;
}
void registerEnterButtonReleasedCallback(void(*cb)(PiPicoFxUiType*))
{
    enterButtonReleasedCallback=cb;
}
void registerExitButtonPressedCallback(void(*cb)(PiPicoFxUiType*))
{
    exitButtonPressedCallback=cb;
}
void registerExitButtonReleasedCallback(void(*cb)(PiPicoFxUiType*))
{
    exitButtonReleasedCallback=cb;
}
void registerStompswitch1PressedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch1PressedCallback=cb;
}
void registerStompswitch1ReleasedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch1ReleasedCallback=cb;
}
void registerStompswitch2PressedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch2PressedCallback=cb;
}
void registerStompswitch2ReleasedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch2ReleasedCallback=cb;
}
void registerStompswitch3PressedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch3PressedCallback=cb;
}
void registerStompswitch3ReleasedCallback(void(*cb)(PiPicoFxUiType*))
{
    stompSwitch3ReleasedCallback=cb;
}
void registerRotaryCallback(void(*cb)(uint16_t,PiPicoFxUiType*))
{
    rotaryCallback=cb;
}
void registerKnob0Callback(void(*cb)(uint16_t,PiPicoFxUiType*))
{
    knob0Callback=cb;
}
void registerKnob1Callback(void(*cb)(uint16_t,PiPicoFxUiType*))
{
    knob1Callback=cb;
}
void registerKnob2Callback(void(*cb)(uint16_t,PiPicoFxUiType*))
{
    knob2Callback=cb;
}
void registerOnUpdateCallback(void(*cb)(int16_t,int16_t,uint8_t,PiPicoFxUiType*))
{
    onUpdateCallback=cb;
}
void registerOnCreateCallback(void(*cb)(PiPicoFxUiType*))
{
    onCreateCallback=cb;
}


/*
Callees used by the OS to dispatch UI event, should not be used by "user" code
*/

void onEnterPressed(PiPicoFxUiType*data)
{
    if (enterButtonPressedCallback!=0)
    {
        enterButtonPressedCallback(data);
    }
}

void onEnterReleased(PiPicoFxUiType*data)
{
    if (enterButtonReleasedCallback!=0)
    {
        enterButtonReleasedCallback(data);
    }
}

void onExitPressed(PiPicoFxUiType*data)
{
    if (exitButtonPressedCallback!=0)
    {
        exitButtonPressedCallback(data);
    }
}

void onExitReleased(PiPicoFxUiType*data)
{
    if (exitButtonReleasedCallback!=0)
    {
        exitButtonReleasedCallback(data);
    }
}

void onRotaryChange(uint16_t delta,PiPicoFxUiType*data)
{
    if(rotaryCallback!=0)
    {
        rotaryCallback(delta,data);
    }
}

void onKnob0(uint16_t val,PiPicoFxUiType*data)
{
    if(knob0Callback!=0)
    {
        knob0Callback(val,data);
    }
}

void onKnob1(uint16_t val,PiPicoFxUiType*data)
{
    if(knob1Callback!=0)
    {
        knob1Callback(val,data);
    }
}

void onKnob2(uint16_t val,PiPicoFxUiType*data)
{
    if(knob2Callback!=0)
    {
        knob2Callback(val,data);
    }
}

void onStompSwitch1Pressed(PiPicoFxUiType*data)
{
    if (stompSwitch1PressedCallback!=0)
    {
        stompSwitch1PressedCallback(data);
    }
}

void onStompSwitch1Released(PiPicoFxUiType*data)
{
    if (stompSwitch1ReleasedCallback!=0)
    {
        stompSwitch1ReleasedCallback(data);
    }
}

void onStompSwitch2Pressed(PiPicoFxUiType*data)
{
    if (stompSwitch2PressedCallback!=0)
    {
        stompSwitch2PressedCallback(data);
    }
}

void onStompSwitch2Released(PiPicoFxUiType*data)
{
    if (stompSwitch2ReleasedCallback!=0)
    {
        stompSwitch2ReleasedCallback(data);
    }
}

void onStompSwitch3Pressed(PiPicoFxUiType*data)
{
    if (stompSwitch3PressedCallback!=0)
    {
        stompSwitch3PressedCallback(data);
    }
}

void onStompSwitch3Released(PiPicoFxUiType*data)
{
    if (stompSwitch3ReleasedCallback!=0)
    {
        stompSwitch3ReleasedCallback(data);
    }
}

void onUpdate(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    if (onUpdateCallback != 0)
    {
        onUpdateCallback(avgInput, avgOutput, cpuLoad, data);
    }
}

void onCreate(PiPicoFxUiType*data)
{
    if (onCreateCallback != 0)
    {
        onCreate(data);
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

