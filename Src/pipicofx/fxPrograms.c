#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "pipicofx/picofxCore.h"
#include "24lc128.h"

FxProgramType* fxPrograms[N_FX_PROGRAMS]={
    
    &fxProgram1, // amp model
    &fxProgram9, // amp model high gain
    &fxProgram14, // eq
    &fxProgram5, // monstercrusher
    &fxProgram8, // compressor
    &fxProgram2, // vibchorus
    &fxProgram11, // sine chorus
    &fxProgram6, // delay
    &fxProgram10, // reverb
    &fxProgram12, // allpass reverb
    &fxProgram13, // hadamard diffusor reverb
    &fxProgram3 // Off
    };


void savePreset(FxPresetType* preset,uint16_t presetPos)
{
    uint16_t cs=0;
    uint32_t address;
    uint8_t * presetArrayPtr;
    presetArrayPtr = (uint8_t*)preset;
    for (uint8_t c=0;c<sizeof(FxPresetType)-2;c++)
    {
        cs += *(presetArrayPtr + c);
    }
    preset->magicNr = cs;
    address = presetPos*sizeof(FxPresetType);
    eeprom24lc128WriteArray(address,sizeof(FxPresetType),presetArrayPtr);
}

uint8_t loadPreset(FxPresetType* preset,uint16_t presetPos)
{
    uint16_t cs=0;
    uint32_t address;
    uint8_t * presetArrayPtr;
    presetArrayPtr = (uint8_t*)preset;
    address = presetPos*sizeof(FxPresetType);
    eeprom24lc128ReadArray(address,sizeof(FxPresetType),presetArrayPtr);
    for (uint8_t c=0;c<sizeof(FxPresetType)-2;c++)
    {
        cs += *(presetArrayPtr + c);
    }
    if (cs==preset->magicNr)
    {
        return 0;
    }
    return 1;
}

void applyPreset(FxPresetType* preset,FxProgramType ** programs)
{
    uint8_t nParams;
    nParams = (*programs + preset->programNr)->nParameters;
    for (uint8_t c=0;c<nParams;c++)
    {
        (*programs + preset->programNr)->parameters[c].setParameter(preset->parameters[c],(*programs + preset->programNr)->data);
    }
}

void parametersToPreset(FxPresetType* preset,FxProgramType ** programs)
{
    uint8_t nParams;
    nParams = (*programs + preset->programNr)->nParameters;
    for (uint8_t c=0;c<nParams;c++)
    {
        preset->parameters[c] = (*programs + preset->programNr)->parameters[c].rawValue;
    }    
}
