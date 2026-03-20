#ifndef _PICOFX_CORE_H_
#define _PICOFX_CORE_H_
#include "stdint.h"
#include "FxProgram.hpp"
#define PARAMETER_NAME_MAXLEN 16
#define FXPROGRAM_NAME_MAXLEN 16
#define FXPROGRAM_MAX_PARAMETERS 8

#define PRESET_TOPOLOGY_SERIAL 0
#define PRESET_TOPOLOGY_PARALLEL 1
#define PRESET_TOPOLOGY_S_TWOP 2
#define PRESET_TOPOLOGY_TWOP_S 3
#define PRESET_TOPOLOGY_TWOS_P 4
using namespace PiPicoFX;
typedef float(*processSampleCallback)(float,void*);

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
    const paramChangeCallback setParameter; // sets the parameter in a meaningful way in the individual program
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
    uint8_t bankPos : 2; // position within the bank
    uint8_t bankNr : 6; // the bank number
    uint8_t programNrA : 6; // the fx Program used in position A of the preset
    uint8_t ledColorA : 2; // led color of effect at position A 0: off, 1: red, 2: green, 3: red+green 
    uint8_t programNrB : 6; // the fx Program used in position B of the preset
    uint8_t ledColorB : 2; // led color of effect at position B 
    uint8_t programNrC : 6; // the fx Program used in position C of the preset
    uint8_t ledColorC : 2; // led color of effect at position C 
    uint8_t topology; // the topology, see enumeration on top
    uint8_t ledColorPreset; // the led color which should be lit when switch presets
    char name[16]; // the name of the preset
    uint16_t parametersA[8];
    uint16_t parametersB[8];
    uint16_t parametersC[8];    
    uint16_t magicNr; // a magic number/checksum which identifies the memory loaded as a preset
} FxPresetType;
extern "C" {
    void savePreset(FxPresetType* preset,uint16_t presetPos);
    uint8_t loadPreset(FxPresetType* preset,uint16_t presetPos);
    void clearPreset(uint16_t presetPos);
    void applyPreset(FxPresetType* preset,FxProgram * program,uint8_t programPosition);
    void parametersToPreset(FxPresetType* preset,FxProgram * programs,uint8_t programPosition);
    void generateEmptyPreset(FxPresetType* preset,uint8_t bank,uint8_t pos);
}
#endif