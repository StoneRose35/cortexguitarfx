#ifdef __cplusplus
extern "C" {
#endif
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#ifdef __cplusplus
}
#endif
#include "pipicofx/FxProgramLoader.hpp"


extern PiPicoFXUiType ui;
static BwImageBufferType imgBuffer;
static BwImageType img;
const uiEnterFct uiEnterFunctions[]={
    &enterLevel0, 
    &enterLevel1, 
    &enterLevel2, 
    &enterLevel3, 
    &enterLevel4, 
    &enterLevel5,
    &enterLevel6,
    &enterLevel7,
    &enterLevel8};
/*
Callback function pointers
*/
static void (*enterButtonPressedCallback)(void)=0; 
static void (*enterButtonReleasedCallback)(void)=0; 
static void (*exitButtonPressedCallback)(void)=0;
static void (*exitButtonReleasedCallback)(void)=0;
static void (*rotaryCallback)(int16_t val)=0;
static void (*knob0Callback)(uint16_t val)=0;
static void (*knob1Callback)(uint16_t val)=0;
static void (*knob2Callback)(uint16_t val)=0;
static void (*stompSwitch1PressedCallback)(void)=0;
static void (*stompSwitch1ReleasedCallback)(void)=0;
static void (*stompSwitch2PressedCallback)(void)=0;
static void (*stompSwitch2ReleasedCallback)(void)=0;
static void (*stompSwitch3PressedCallback)(void)=0;
static void (*stompSwitch3ReleasedCallback)(void)=0;
static void (*onUpdateCallback)(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)=0;
static void (*onCreateCallback)(void)=0;

__QSPI_CODE
BwImageType * getImageBuffer()
{
    /*
    img.data = imgBuffer.data;
    img.sx = imgBuffer.sx;
    img.sy = imgBuffer.sy;
    img.type = imgBuffer.type;
    */
    return &img;
}

const uiEnterFct*  getEnterFunctions()
{
    return uiEnterFunctions;
}


/*
 registration functions, used to attach a certain functionality to a
 ui element callback
 */
__QSPI_CODE
void registerEnterButtonPressedCallback(void(*cb)(void))
{
    enterButtonPressedCallback=cb;
}
__QSPI_CODE
void registerEnterButtonReleasedCallback(void(*cb)(void))
{
    enterButtonReleasedCallback=cb;
}
__QSPI_CODE
void registerExitButtonPressedCallback(void(*cb)(void))
{
    exitButtonPressedCallback=cb;
}
__QSPI_CODE
void registerExitButtonReleasedCallback(void(*cb)(void))
{
    exitButtonReleasedCallback=cb;
}
__QSPI_CODE
void registerStompswitch1PressedCallback(void(*cb)(void))
{
    stompSwitch1PressedCallback=cb;
}
__QSPI_CODE
void registerStompswitch1ReleasedCallback(void(*cb)(void))
{
    stompSwitch1ReleasedCallback=cb;
}
__QSPI_CODE
void registerStompswitch2PressedCallback(void(*cb)(void))
{
    stompSwitch2PressedCallback=cb;
}
__QSPI_CODE
void registerStompswitch2ReleasedCallback(void(*cb)(void))
{
    stompSwitch2ReleasedCallback=cb;
}
__QSPI_CODE
void registerStompswitch3PressedCallback(void(*cb)(void))
{
    stompSwitch3PressedCallback=cb;
}
__QSPI_CODE
void registerStompswitch3ReleasedCallback(void(*cb)(void))
{
    stompSwitch3ReleasedCallback=cb;
}
__QSPI_CODE
void registerRotaryCallback(void(*cb)(int16_t))
{
    rotaryCallback=cb;
}
__QSPI_CODE
void registerKnob0Callback(void(*cb)(uint16_t))
{
    knob0Callback=cb;
}
__QSPI_CODE
void registerKnob1Callback(void(*cb)(uint16_t))
{
    knob1Callback=cb;
}
__QSPI_CODE
void registerKnob2Callback(void(*cb)(uint16_t))
{
    knob2Callback=cb;
}
__QSPI_CODE
void registerOnUpdateCallback(void(*cb)(int16_t,int16_t,uint8_t))
{
    onUpdateCallback=cb;
}
__QSPI_CODE
void registerOnCreateCallback(void(*cb)(void))
{
    onCreateCallback=cb;
}
__QSPI_CODE
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
__QSPI_CODE
void onEnterPressed()
{
    if (enterButtonPressedCallback!=0)
    {
        enterButtonPressedCallback();
    }
}
__QSPI_CODE
void onEnterReleased()
{
    if (enterButtonReleasedCallback!=0)
    {
        enterButtonReleasedCallback();
    }
}
__QSPI_CODE
void onExitPressed()
{
    if (exitButtonPressedCallback!=0)
    {
        exitButtonPressedCallback();
    }
    if(uiStackCurrent() != 0xFF)
    {
        uiEnterFunctions[uiStackPop()]();
    }
}
__QSPI_CODE
void onExitReleased()
{
    if (exitButtonReleasedCallback!=0)
    {
        exitButtonReleasedCallback();
    }
}
__QSPI_CODE
void onRotaryChange(int16_t delta)
{
    if(rotaryCallback!=0)
    {
        rotaryCallback(delta);
    }
}
__QSPI_CODE
void onKnob0(uint16_t val)
{
    if(knob0Callback!=0)
    {
        knob0Callback(val);
    }
}
__QSPI_CODE
void onKnob1(uint16_t val)
{
    if(knob1Callback!=0)
    {
        knob1Callback(val);
    }
}
__QSPI_CODE
void onKnob2(uint16_t val)
{
    if(knob2Callback!=0)
    {
        knob2Callback(val);
    }
}
__QSPI_CODE
void onStompSwitch1Pressed()
{
    if (stompSwitch1PressedCallback!=0)
    {
        stompSwitch1PressedCallback();
    }
}
__QSPI_CODE
void onStompSwitch1Released()
{
    if (stompSwitch1ReleasedCallback!=0)
    {
        stompSwitch1ReleasedCallback();
    }
}
__QSPI_CODE
void onStompSwitch2Pressed()
{
    if (stompSwitch2PressedCallback!=0)
    {
        stompSwitch2PressedCallback();
    }
}
__QSPI_CODE
void onStompSwitch2Released()
{
    if (stompSwitch2ReleasedCallback!=0)
    {
        stompSwitch2ReleasedCallback();
    }
}
__QSPI_CODE
void onStompSwitch3Pressed(void)
{
    if (stompSwitch3PressedCallback!=0)
    {
        stompSwitch3PressedCallback();
    }
}
__QSPI_CODE
void onStompSwitch3Released(void)
{
    if (stompSwitch3ReleasedCallback!=0)
    {
        stompSwitch3ReleasedCallback();
    }
}
__QSPI_CODE
void onUpdate(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    if (onUpdateCallback != 0)
    {
        onUpdateCallback(avgInput, avgOutput, cpuLoad);
    }
}
__QSPI_CODE
void onCreate(void)
{
    if (onCreateCallback != 0)
    {
        onCreateCallback();
    }
}
__QSPI_CODE
uint8_t uiStackPush(uint8_t val)
{
    if (ui.uiLevelStackPtr < PIPICOFX_UI_STACK_SIZE)
    {
        *(ui.uiLevelStack + ui.uiLevelStackPtr++) = val;
        return 0;
    }
    return 1;
}
__QSPI_CODE
uint8_t uiStackPop()
{
    if (ui.uiLevelStackPtr != 0)
    {
        return *(ui.uiLevelStack + --ui.uiLevelStackPtr);
    }
    return  *(ui.uiLevelStack + ui.uiLevelStackPtr);
    //return 0xFF;
}
__QSPI_CODE
uint8_t uiStackCurrent()
{
    if (ui.uiLevelStackPtr != 0)
    {
        return *(ui.uiLevelStack + ui.uiLevelStackPtr-1);
    }
    return *(ui.uiLevelStack);
}
__QSPI_CODE
void piPicoFxUiSetup(void)
{
    ui.currentProgram=PiPicoFX::loadProgram(2);
    ui.currentProgramIdx=2;
    ui.currentParameter=ui.currentProgram->getParameter(0);
    ui.currentParameterIdx=0;
    ui.locked=0;
    ui.editViaRotary =0;
    ui.uiLevelStackPtr = 0;
    *(ui.uiLevelStack) = 0;
    for (uint8_t c=1;c<8;c++)
    {
        *(ui.uiLevelStack + c) = 0xFF;
    }
    imgBuffer.sx=128;
    imgBuffer.sy=64;
    img.byteSize = (imgBuffer.sx*imgBuffer.sy)>>3;
    img.data = imgBuffer.data;
    img.sx = imgBuffer.sx;
    img.sy = imgBuffer.sy;
    img.type = imgBuffer.type;

    #ifdef HORIZONTAL_DISPLAY
    imgBuffer.type = BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES;
    #endif
    #ifdef VERTICAL_DISPLAY
    imgBuffer.type = BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES;
    #endif
}
