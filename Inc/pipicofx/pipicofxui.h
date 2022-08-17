#ifndef _PI_PICO_FX_UI_H_
#define _PI_PICO_FX_UI_H_
#include "stdint.h"
#include "audio/fxprogram/fxProgram.h"

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

void updateAudioUi(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data);

void drawUi(PiPicoFxUiType*data);

void knob0Callback(uint16_t val,PiPicoFxUiType*data);
void knob1Callback(uint16_t val,PiPicoFxUiType*data);
void knob2Callback(uint16_t val,PiPicoFxUiType*data);

void button1Callback(PiPicoFxUiType*data);
void button2Callback(PiPicoFxUiType*data);
void rotaryCallback(uint32_t encoderValue,PiPicoFxUiType*data);

void piPicoFxUiSetup();

PiPicoFxUiType piPicoUiController;
#endif