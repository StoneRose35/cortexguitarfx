#ifndef _PI_PICO_FX_UI_H_
#define _PI_PICO_FX_UI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "pipicofx/fxPrograms.h"
#include "graphics/bwgraphics.h"
#ifdef __cplusplus
}
#endif
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/fxProgramParameter.hpp"

#define PIPICOFX_UI_STACK_SIZE 8

typedef struct 
{
    PiPicoFX::FxProgram* currentProgram;
    PiPicoFX::FxProgramParameter * currentParameter;
    uint8_t uiLevelStack[8];
    void * data;
    uint8_t uiLevelStackPtr;
    uint8_t currentProgramIdx;
    uint8_t currentParameterIdx;
    uint8_t locked : 1;
    uint8_t editViaRotary : 1;
    uint8_t defaultOn : 1; // switches programs on when changing programs, off otherwise 
} PiPicoFXUiType;

#ifdef __cplusplus 
extern "C" {
#endif
typedef struct 
{
    uint32_t pressedTimestamp;
    
} ButtonStateType;


typedef void(*uiEnterFct)(void);


const uiEnterFct*  getEnterFunctions();
void registerEnterButtonPressedCallback(void(*cb)(void));
void registerEnterButtonReleasedCallback(void(*cb)(void));
void registerExitButtonPressedCallback(void(*cb)(void));
void registerExitButtonReleasedCallback(void(*cb)(void));
void registerStompswitch1PressedCallback(void(*cb)(void));
void registerStompswitch1ReleasedCallback(void(*cb)(void));
void registerStompswitch2PressedCallback(void(*cb)(void));
void registerStompswitch2ReleasedCallback(void(*cb)(void));
void registerStompswitch3PressedCallback(void(*cb)(void));
void registerStompswitch3ReleasedCallback(void(*cb)(void));
void registerRotaryCallback(void(*cb)(int16_t));
void registerKnob0Callback(void(*cb)(uint16_t));
void registerKnob1Callback(void(*cb)(uint16_t));
void registerKnob2Callback(void(*cb)(uint16_t));
void registerOnUpdateCallback(void(*cb)(int16_t,int16_t,uint8_t));
void registerOnCreateCallback(void(*cb)(void));
void clearCallbackAssignments();

void onEnterPressed(void);
void onEnterReleased(void);
void onExitPressed(void);
void onExitReleased(void);
void onRotaryChange(int16_t delta);
void onKnob0(uint16_t val);
void onKnob1(uint16_t val);
void onKnob2(uint16_t val);
void onStompSwitch1Pressed(void);
void onStompSwitch1Released(void);
void onStompSwitch2Pressed(void);
void onStompSwitch2Released(void);
void onStompSwitch3Pressed(void);
void onStompSwitch3Released(void);
void onUpdate(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad);
void onCreate(void);
BwImageType * getImageBuffer();


uint8_t uiStackPush(uint8_t val);
uint8_t uiStackPop();
uint8_t uiStackCurrent();
#ifdef __cplusplus
}
#endif

void piPicoFxUiSetup(void);
void enterLevel0(void);
void enterLevel1(void);
void enterLevel2(void);
void enterLevel3(void);
void enterLevel4(void);
void enterLevel5(void);
void enterLevel6(void);
void enterLevel7(void);
void enterLevel8(void);
void enterLevel9(void);
#endif