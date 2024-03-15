#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "pipicofx/picofxCore.h"
#include "drivers/24lc128.h"
#include "stringFunctions.h"

FxProgramType* fxPrograms[N_FX_PROGRAMS]={
    
    &fxProgram1, // amp model
    &fxProgram9, // amp model high gain
    &fxProgram14, // eq
    &fxProgram15, // vocal processor
    &fxProgram8, // compressor
    &fxProgram2, // vibchorus
    &fxProgram11, // sine chorus
    &fxProgram6, // delay
    &fxProgram10, // reverb
    &fxProgram12, // allpass reverb
    &fxProgram13, // hadamard diffusor reverb
    &fxProgram16, // hadamard diffusor reverb
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
        if (preset->programNr >= N_FX_PROGRAMS) // program number is larger than the maximum number of program
        {
            return 1;
        }
        return 0;
    }
    return 1;
}

void applyPreset(FxPresetType* preset,FxProgramType ** programs)
{
    uint8_t nParams;
    nParams = (*(programs + preset->programNr))->nParameters;
    for (uint8_t c=0;c<nParams;c++)
    {
        (*(programs + preset->programNr))->parameters[c].setParameter(preset->parameters[c],(*(programs + preset->programNr))->data);
    }
}

void parametersToPreset(FxPresetType* preset,FxProgramType ** programs)
{
    uint8_t nParams;
    nParams = (*(programs+preset->programNr))->nParameters;
    for (uint8_t c=0;c<nParams;c++)
    {
        preset->parameters[c] = (*(programs + preset->programNr))->parameters[c].rawValue;
    }    
}

void generateEmptyPreset(FxPresetType* preset,uint8_t bank,uint8_t pos)
{
    char nrbfr[8];
    preset->bankNr = bank;
    preset->bankPos = pos;
    preset->name[0] = 0;
    appendToString(preset->name,"B");
    UInt8ToChar(bank,nrbfr);
    appendToString(preset->name,nrbfr);
    appendToString(preset->name," P");
    UInt8ToChar(pos,nrbfr);
    appendToString(preset->name,nrbfr);
    appendToStringUntil(preset->name,"        ",8);
    preset->programNr = N_FX_PROGRAMS -1; // off should always be last
    for (uint8_t c=0;c< 8; c++)
    {
        preset->parameters[c] = 0;
    } 
    preset->ledColor = 1;

}
 