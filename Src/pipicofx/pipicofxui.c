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



static BwImageBufferType imgBuffer;
static BwImageType img;
const uiEnterFct uiEnterFunctions[]={&enterLevel0, &enterLevel1, &enterLevel2, &enterLevel3, &enterLevel4, &enterLevel5};


/*
Callback function pointers
*/
static void (*enterButtonPressedCallback)(PiPicoFxUiType *ui)=0; 
static void (*enterButtonReleasedCallback)(PiPicoFxUiType* ui)=0; 
static void (*exitButtonPressedCallback)(PiPicoFxUiType* ui)=0;
static void (*exitButtonReleasedCallback)(PiPicoFxUiType* ui)=0;
static void (*rotaryCallback)(int16_t val,PiPicoFxUiType* ui)=0;
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

BwImageType * getImageBuffer()
{
    img.data = imgBuffer.data;
    img.sx = imgBuffer.sx;
    img.sy = imgBuffer.sy;
    img.type = imgBuffer.type;
    return &img;
}


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
void registerRotaryCallback(void(*cb)(int16_t,PiPicoFxUiType*))
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

void clearCallbackAssignments()
{
    enterButtonPressedCallback = 0;
    enterButtonReleasedCallback = 0;
    exitButtonPressedCallback = 0;
    exitButtonReleasedCallback = 0;
    knob0Callback = 0;
    knob1Callback = 0;
    knob2Callback = 0;
    rotaryCallback = 0;
    stompSwitch1PressedCallback = 0;
    stompSwitch1ReleasedCallback = 0;
    stompSwitch2PressedCallback = 0;
    stompSwitch2ReleasedCallback = 0;
    stompSwitch3PressedCallback = 0;
    stompSwitch3ReleasedCallback = 0;
    onUpdateCallback = 0;
    onCreateCallback = 0;
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
    if(uiStackCurrent(data) != 0xFF)
    {
        uiEnterFunctions[uiStackPop(data)](data);
    }
}

void onExitReleased(PiPicoFxUiType*data)
{
    if (exitButtonReleasedCallback!=0)
    {
        exitButtonReleasedCallback(data);
    }
}

void onRotaryChange(int16_t delta,PiPicoFxUiType*data)
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
        onCreateCallback(data);
    }
}

uint8_t uiStackPush(PiPicoFxUiType* piPicoUiController,uint8_t val)
{
    if (piPicoUiController->uiLevelStackPtr < PIPICOFX_UI_STACK_SIZE)
    {
        *(piPicoUiController->uiLevelStack + piPicoUiController->uiLevelStackPtr++) = val;
        return 0;
    }
    return 1;
}

uint8_t uiStackPop(PiPicoFxUiType* piPicoUiController)
{
    if (piPicoUiController->uiLevelStackPtr != 0)
    {
        return *(piPicoUiController->uiLevelStack + --piPicoUiController->uiLevelStackPtr);
    }
    return 0xFF;
}

uint8_t uiStackCurrent(PiPicoFxUiType* piPicoUiController)
{
    if (piPicoUiController->uiLevelStackPtr != 0)
    {
        return *(piPicoUiController->uiLevelStack + piPicoUiController->uiLevelStackPtr-1);
    }
    return 0xFF;
}

void piPicoFxUiSetup(PiPicoFxUiType* piPicoUiController)
{
    piPicoUiController->currentProgram=fxPrograms[0];
    piPicoUiController->currentProgramIdx=0;
    piPicoUiController->currentParameter=fxPrograms[piPicoUiController->currentProgramIdx]->parameters;
    piPicoUiController->currentParameterIdx=0;
    piPicoUiController->locked=0;
    piPicoUiController->editViaRotary =0;
    piPicoUiController->uiLevelStackPtr = 0;
    for (uint8_t c=0;c<8;c++)
    {
        *(piPicoUiController->uiLevelStack + c) = 0xFF;
    }
    imgBuffer.sx=128;
    imgBuffer.sy=64;
    #ifdef HORIZONTAL_DISPLAY
    imgBuffer.type = BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES;
    #endif
    #ifdef VERTICAL_DISPLAY
    imgBuffer.type = BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES;
    #endif
}

