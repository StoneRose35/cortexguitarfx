#ifndef _PI_PICO_FX_UI_H_
#define _PI_PICO_FX_UI_H_
#include "stdint.h"
#include "pipicofx/fxPrograms.h"
#include "graphics/bwgraphics.h"

#define PIPICOFX_UI_STACK_SIZE 8

typedef struct 
{
    FxProgramType* currentProgram;
    FxProgramParameterType * currentParameter;
    uint8_t uiLevelStack[8];
    uint8_t uiLevelStackPtr;
    uint8_t currentProgramIdx;
    uint8_t currentParameterIdx;
    uint8_t locked : 1;
    uint8_t editViaRotary : 1;
    uint8_t lastUiLevel; // the ui level that should be entered upon exiting the current one, 0xFF means null
} PiPicoFxUiType;


typedef struct 
{
    uint32_t pressedTimestamp;
    
} ButtonStateType;


typedef void(*uiEnterFct)(PiPicoFxUiType*);

void registerEnterButtonPressedCallback(void(*cb)(PiPicoFxUiType*));
void registerEnterButtonReleasedCallback(void(*cb)(PiPicoFxUiType*));
void registerExitButtonPressedCallback(void(*cb)(PiPicoFxUiType*));
void registerExitButtonReleasedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch1PressedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch1ReleasedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch2PressedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch2ReleasedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch3PressedCallback(void(*cb)(PiPicoFxUiType*));
void registerStompswitch3ReleasedCallback(void(*cb)(PiPicoFxUiType*));
void registerRotaryCallback(void(*cb)(uint16_t,PiPicoFxUiType*));
void registerKnob0Callback(void(*cb)(uint16_t,PiPicoFxUiType*));
void registerKnob1Callback(void(*cb)(uint16_t,PiPicoFxUiType*));
void registerKnob2Callback(void(*cb)(uint16_t,PiPicoFxUiType*));
void registerOnUpdateCallback(void(*cb)(int16_t,int16_t,uint8_t,PiPicoFxUiType*));
void registerOnCreateCallback(void(*cb)(PiPicoFxUiType*));

void onEnterPressed(PiPicoFxUiType*data);
void onEnterReleased(PiPicoFxUiType*data);
void onExitPressed(PiPicoFxUiType*data);
void onExitReleased(PiPicoFxUiType*data);
void onRotaryChange(uint16_t delta,PiPicoFxUiType*data);
void onKnob0(uint16_t val,PiPicoFxUiType*data);
void onKnob1(uint16_t val,PiPicoFxUiType*data);
void onKnob2(uint16_t val,PiPicoFxUiType*data);
void onStompSwitch1Pressed(PiPicoFxUiType*data);
void onStompSwitch1Released(PiPicoFxUiType*data);
void onStompSwitch2Pressed(PiPicoFxUiType*data);
void onStompSwitch2Released(PiPicoFxUiType*data);
void onStompSwitch3Pressed(PiPicoFxUiType*data);
void onStompSwitch3Released(PiPicoFxUiType*data);
void onUpdate(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data);
void onCreate(PiPicoFxUiType*data);
BwImageBufferType * getImageBuffer();


void piPicoFxUiSetup(PiPicoFxUiType*);
void enterLevel0(PiPicoFxUiType*data);
void enterLevel1(PiPicoFxUiType*data);
void enterLevel2(PiPicoFxUiType*data);
void enterLevel3(PiPicoFxUiType*data);
void enterLevel4(PiPicoFxUiType*data);
void enterLevel5(PiPicoFxUiType*data);

uint8_t uiStackPush(PiPicoFxUiType* piPicoUiController,uint8_t val);
uint8_t uiStackPop(PiPicoFxUiType* piPicoUiController);
uint8_t uiStackCurrent(PiPicoFxUiType* piPicoUiController);

#endif