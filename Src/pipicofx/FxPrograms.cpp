extern "C" {
#include <stdint.h>

#ifdef HARDWARE
#include "drivers/24lc128.h"
#endif
#include "stringFunctions.h"
}
#include "pipicofx/picofxCore.hpp"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/fxProgramParameter.hpp"
#include "pipicofx/FxProgramLoader.hpp"

__QSPI_CODE
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
    #ifdef HARDWARE
    eeprom24lc128WriteArray(address,sizeof(FxPresetType),presetArrayPtr);
    #endif
}

__QSPI_CODE
uint8_t loadPreset(FxPresetType* preset,uint16_t presetPos)
{
    uint16_t cs=0;
    uint32_t address;
    uint8_t * presetArrayPtr;
    presetArrayPtr = (uint8_t*)preset;
    address = presetPos*sizeof(FxPresetType);
    #ifdef HARDWARE
    eeprom24lc128ReadArray(address,sizeof(FxPresetType),presetArrayPtr);
    #endif
    for (uint8_t c=0;c<sizeof(FxPresetType)-2;c++)
    {
        cs += *(presetArrayPtr + c);
    }
    if (cs==preset->magicNr)
    {
        if (preset->programNrA >= N_FX_PROGRAMS 
            || preset->programNrB >= N_FX_PROGRAMS
            || preset->programNrC >= N_FX_PROGRAMS) // any program nr is larger than the maximum number of program
        {
            return 1;
        }
        return 0;
    }
    return 1;
}

__QSPI_CODE
void clearPreset(uint16_t presetPos)
{
    uint32_t address;

    address = presetPos*sizeof(FxPresetType);
    uint8_t presetArray[sizeof(FxPresetType)];
    for (uint8_t c=0;c<sizeof(FxPresetType);c++)
    {
        presetArray[c]=0xFF;
    }
    eeprom24lc128WriteArray(address,sizeof(FxPresetType),presetArray);
}

__QSPI_CODE
void applyPreset(FxPresetType* preset,PiPicoFX::FxProgram * program,uint8_t programPosition)
{
    uint8_t nParams;
    nParams = program->getParameterCount();
    for (uint8_t c=0;c<nParams;c++)
    {
        program->getParameter(c)->parameterCallback(preset->parametersA[c + (programPosition << 3)]);
    }
}

__QSPI_CODE
void parametersToPreset(FxPresetType* preset,FxProgram * program,uint8_t programPosition)
{
    uint8_t nParams;
    nParams = program->getParameterCount();
    for (uint8_t c=0;c<nParams;c++)
    {
        preset->parametersA[c + (programPosition<<3)] = program->getParameter(c)->rawValue;
    }    
}

__QSPI_CODE
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
    preset->programNrA = 2;
    preset->programNrB = 2;
    preset->programNrC = 2;
    preset->topology = PRESET_TOPOLOGY_SERIAL;

    for (uint8_t c=0;c< 8; c++) 
    {
        preset->parametersA[c] = 0x3FF; //set all values to 1023 to start with Volume 1
    } 

    for (uint8_t c=0;c< 8; c++) 
    {
        preset->parametersB[c] = 0x3FF; //set all values to 1023 to start with Volume 1
    } 

    for (uint8_t c=0;c< 8; c++) 
    {
        preset->parametersC[c] = 0x3FF; //set all values to 1023 to start with Volume 1
    } 

    preset->ledColorA = 1;
    preset->ledColorB = 1;
    preset->ledColorC = 1;
}
 