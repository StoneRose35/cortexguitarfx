extern "C" {
#include <stdint.h>

#ifdef HARDWARE
#include "stm32h750/stm32h750xx.h"
#include "drivers/24lc128.h"
#endif
#include "stringFunctions.h"
}
#include "pipicofx/picofxCore.hpp"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/fxProgramParameter.hpp"
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/MultiAudioProcessor.hpp"

#ifndef HARDWARE
uint8_t mockedEeprom[16384];
#endif
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
    #else
    for (uint16_t c=0;c<sizeof(FxPresetType);c++)
    {
        *(mockedEeprom + address + c)=*(presetArrayPtr+c); 
    }
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
    #else
    // simulate an empty eeprom when not compiling against hardware
    for (uint16_t c=0;c<sizeof(FxPresetType);c++)
    {
        *(presetArrayPtr + c)=*(mockedEeprom + presetPos*sizeof(FxPresetType) + c);
    }
    #endif
    for (uint8_t c=0;c<sizeof(FxPresetType)-2;c++)
    {
        cs += *(presetArrayPtr + c);
    }
    if (cs==preset->magicNr)
    {
        /*
        if (preset->programNrA >= N_FX_PROGRAMS 
            || preset->programNrB >= N_FX_PROGRAMS
            || preset->programNrC >= N_FX_PROGRAMS) // any program nr is larger than the maximum number of program
        {
            return 1;
        }
        */
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
    #ifdef HARDWARE
    eeprom24lc128WriteArray(address,sizeof(FxPresetType),presetArray);
    #endif
}

__QSPI_CODE
void applyPreset(FxPresetType* preset,PiPicoFX::MultiAudioProcessor * audioProcessor)
{
    uint8_t nParams;
    
    if (audioProcessor->getFxProgram(0) != nullptr)
    {
        nParams = ((FxProgram*)audioProcessor->getFxProgram(0))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            ((FxProgram*)audioProcessor->getFxProgram(0))->getParameter(c)->parameterCallback(preset->parametersA[c]);
        }
    }
    if (audioProcessor->getFxProgram(1) != nullptr)
    {
        nParams = ((FxProgram*)audioProcessor->getFxProgram(1))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            ((FxProgram*)audioProcessor->getFxProgram(1))->getParameter(c)->parameterCallback(preset->parametersB[c]);
        }
    }   
    if (audioProcessor->getFxProgram(2) != nullptr)
    {
        nParams = ((FxProgram*)audioProcessor->getFxProgram(2))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            ((FxProgram*)audioProcessor->getFxProgram(2))->getParameter(c)->parameterCallback(preset->parametersC[c]);
        }
    }    
}

__QSPI_CODE
void applyPresetToProgram(FxPresetType* preset,PiPicoFX::MultiAudioProcessor * audioProcessor,uint8_t programNr)
{
    uint8_t nParams;
    switch(programNr)
    {
        case 0:
            if (audioProcessor->getFxProgram(0) != nullptr)
            {
                nParams = ((FxProgram*)audioProcessor->getFxProgram(0))->getParameterCount();
                for (uint8_t c=0;c<nParams;c++)
                {
                    ((FxProgram*)audioProcessor->getFxProgram(0))->getParameter(c)->parameterCallback(preset->parametersA[c]);
                }
            }
            break;
        case 1:
            if (audioProcessor->getFxProgram(1) != nullptr)
            {
                nParams = ((FxProgram*)audioProcessor->getFxProgram(1))->getParameterCount();
                for (uint8_t c=0;c<nParams;c++)
                {
                    ((FxProgram*)audioProcessor->getFxProgram(1))->getParameter(c)->parameterCallback(preset->parametersB[c]);
                }
            }   
            break;
        case 2: 
            if (audioProcessor->getFxProgram(2) != nullptr)
            {
                nParams = ((FxProgram*)audioProcessor->getFxProgram(2))->getParameterCount();
                for (uint8_t c=0;c<nParams;c++)
                {
                    ((FxProgram*)audioProcessor->getFxProgram(2))->getParameter(c)->parameterCallback(preset->parametersC[c]);
                }
            }  
            break;
    }
}

__QSPI_CODE
void parametersToPreset(FxPresetType* preset,MultiAudioProcessor * audioProcessor)
{
    uint8_t nParams;
    if (audioProcessor->getFxProgram(0) != nullptr)
    {
        preset->programNrA = ((FxProgram*)audioProcessor->getFxProgram(0))->getIndex();
        nParams = ((FxProgram*)audioProcessor->getFxProgram(0))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            preset->parametersA[c] = ((FxProgram*)audioProcessor->getFxProgram(0))->getParameter(c)->rawValue;
        }   
    }
    if (audioProcessor->getFxProgram(1) != nullptr)
    {
        preset->programNrB = ((FxProgram*)audioProcessor->getFxProgram(1))->getIndex();
        nParams = ((FxProgram*)audioProcessor->getFxProgram(1))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            preset->parametersB[c] = ((FxProgram*)audioProcessor->getFxProgram(1))->getParameter(c)->rawValue;
        } 
    }
    if (audioProcessor->getFxProgram(2) != nullptr)
    {
        preset->programNrC = ((FxProgram*)audioProcessor->getFxProgram(2))->getIndex();
        nParams = ((FxProgram*)audioProcessor->getFxProgram(2))->getParameterCount();
        for (uint8_t c=0;c<nParams;c++)
        {
            preset->parametersC[c] = ((FxProgram*)audioProcessor->getFxProgram(2))->getParameter(c)->rawValue;
        } 
    }
    preset->routing = audioProcessor->getRouting();
 
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
    preset->programNrB = 0x3F;
    preset->programNrC = 0x3F;
    preset->routing = PRESET_ROUTING_SERIAL;

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
    preset->ledColorPreset = 1;
    preset->ledColorA = 1;
    preset->ledColorB = 1;
    preset->ledColorC = 1;
}
 