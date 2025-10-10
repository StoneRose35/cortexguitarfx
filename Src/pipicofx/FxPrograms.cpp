extern "C" {
#include <stdint.h>

#include "drivers/24lc128.h"
#include "stringFunctions.h"
}
#include "pipicofx/picofxCore.hpp"
//#include "pipicofx/fxPrograms.h"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/fxProgramParameter.hpp"
#include "pipicofx/FxProgramLoader.hpp"

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
    #ifdef STM32
    eeprom24lc128WriteArray(address,sizeof(FxPresetType),presetArrayPtr);
    #endif
}

uint8_t loadPreset(FxPresetType* preset,uint16_t presetPos)
{
    uint16_t cs=0;
    uint32_t address;
    uint8_t * presetArrayPtr;
    presetArrayPtr = (uint8_t*)preset;
    address = presetPos*sizeof(FxPresetType);
    #ifdef STM32
    eeprom24lc128ReadArray(address,sizeof(FxPresetType),presetArrayPtr);
    #endif
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

void applyPreset(FxPresetType* preset,PiPicoFX::FxProgram * program)
{
    uint8_t nParams;
    nParams = program->getParameterCount();
    for (uint8_t c=0;c<nParams;c++)
    {
        program->getParameter(c)->parameterCallback(preset->parameters[c]);
    }
}

void parametersToPreset(FxPresetType* preset,FxProgram * program)
{
    uint8_t nParams;
    nParams = program->getParameterCount();
    for (uint8_t c=0;c<nParams;c++)
    {
        preset->parameters[c] = program->getParameter(c)->rawValue;
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
 