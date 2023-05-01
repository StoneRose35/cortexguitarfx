#ifndef _PI_PICO_FX_UI_H_
#define _PI_PICO_FX_UI_H_
#include "stdint.h"
#include "pipicofx/fxPrograms.h"

typedef struct 
{
    uint8_t displayLevel; // 0,1 or two
    FxProgramType* currentProgram;
    FxProgramParameterType * currentParameter;
    uint32_t oldEncoderValue;
    uint8_t currentProgramIdx;
    uint8_t currentParameterIdx;
    int16_t oldParamValue;
    uint8_t locked;
} PiPicoFxUiType;


typedef struct 
{
    uint32_t pressedTimestamp;
    
} ButtonStateType;


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




/*
void updateAudioUi(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data);

void drawUi(PiPicoFxUiType*data);

void knob0Callback(uint16_t val,PiPicoFxUiType*data);
void knob1Callback(uint16_t val,PiPicoFxUiType*data);
void knob2Callback(uint16_t val,PiPicoFxUiType*data);

void button1Callback(PiPicoFxUiType*data);
void button2Callback(PiPicoFxUiType*data);
void rotaryCallback(int16_t encoderValue,PiPicoFxUiType*data);
*/
void piPicoFxUiSetup(PiPicoFxUiType*);

void enterLevel0(PiPicoFxUiType*data);
void enterLevel1(PiPicoFxUiType*data);
void enterLevel2(PiPicoFxUiType*data);

#endif