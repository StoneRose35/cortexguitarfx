#ifndef _PICOFX_CORE_H_
#define _PICOFX_CORE_H_
#include "stdint.h"
#define PARAMETER_NAME_MAXLEN 16
#define FXPROGRAM_NAME_MAXLEN 24
#define FXPROGRAM_MAX_PARAMETERS 8

#ifndef FLOAT_AUDIO
typedef int16_t(*processSampleCallback)(int16_t,void*);
#else
typedef float(*processSampleCallback)(float,void*);
#endif
typedef void(*paramChangeCallback)(uint16_t,void*);
typedef void(*setupCallback)(void*);
typedef void(*resetCallback)(void*);
typedef void*(*getParameterValueFct)(void*);
typedef void(*getParameterDisplayFct)(void*,char*);

typedef struct {
    const char name[PARAMETER_NAME_MAXLEN];
    const uint8_t control; // 0-2: Potentiometers, 255: no control binding
    int16_t rawValue;
    int16_t increment;
    const getParameterValueFct getParameterValue; // returns the converted parameter value, data type depends on the implementation
    const getParameterDisplayFct getParameterDisplay; // returns the display value as a string of a Parameter
    paramChangeCallback setParameter; // sets the parameter in a meaningful way in the individual program
} FxProgramParameterType;

typedef struct {
    const char name[FXPROGRAM_NAME_MAXLEN];
    FxProgramParameterType parameters[FXPROGRAM_MAX_PARAMETERS];
    const processSampleCallback processSample;
    const setupCallback setup;
    const resetCallback reset;
    const uint8_t nParameters;
    void * data;
} FxProgramType;


typedef struct __attribute__((__packed__)) {
    uint8_t bankPos : 3; // position within the bank
    uint8_t bankNr : 5; // the bank number
    uint8_t programNr; // the fx Program used in the preset
    char name[24]; // the name of the preset
    uint16_t parameters[8];
    uint16_t magicNr; // a magic number/checksum which identifies the memory loaded as a preset
} FxPresetType;

void savePreset(FxPresetType* preset,uint16_t presetPos);
uint8_t loadPreset(FxPresetType* preset,uint16_t presetPos);

#endif