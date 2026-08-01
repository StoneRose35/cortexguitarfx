extern "C" {
#include "memoryRegions.h"
#include "stdlib.h"
#include "globalConfig.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/stompswitches.h"
}

#include "pipicofx/Presets.hpp"

extern uint8_t currentBank;
extern uint8_t currentPreset;
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
extern FxPresetType presets[3];
extern volatile uint8_t programsToInitialize[3]; 
extern volatile uint8_t programChangeState;

uint8_t previewBankNr=0xFF;


__QSPI_CODE
void reloadPresetsFromEeprom(FxPresetType*priis,uint8_t bnk)
{
    if (loadPreset(priis,bnk*3)!=0)
    {
        generateEmptyPreset(priis,bnk,0);
    }
    if (loadPreset(priis+1,bnk*3+1)!=0)
    {
        generateEmptyPreset(priis+1,bnk,1);
    }
    if (loadPreset(priis+2, bnk*3+2)!=0)
    {
        generateEmptyPreset(priis+2,bnk,2);
    }
}


__QSPI_CODE
void setPresetAtBank(uint8_t bankNr,uint8_t presetNr)
{
    previewBankNr = bankNr;
    setPresetNr(presetNr);
}

__QSPI_CODE
void setPresetNr(uint8_t nr)
{
    if (previewBankNr != currentBank && previewBankNr != 0xFF)
    {
        currentBank = previewBankNr;
        previewBankNr = 0xFF;
        reloadPresetsFromEeprom(presets,currentBank);
    }
    currentPreset = nr;
    setPreset();
}

__QSPI_CODE
void setPreset()
{
    if (presets[currentPreset].programNrA != 0x3F && (audioProcessor.getFxProgram(0) == nullptr || 
        (audioProcessor.getFxProgram(0) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(0))->getIndex() != presets[currentPreset].programNrA)))
    {
        programsToInitialize[0] = presets[currentPreset].programNrA | 0x80;
    }
    else if (presets[currentPreset].programNrA == 0x3F) // no effect should be set
    {
        programsToInitialize[0]=0x7e;
    }
    else // effect program remains unchanged, only apply parameters
    {
        programsToInitialize[0]=0x7f | 0x80;
    }

    if (presets[currentPreset].programNrB != 0x3F && (audioProcessor.getFxProgram(1) == nullptr || 
        (audioProcessor.getFxProgram(1) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(1))->getIndex() != presets[currentPreset].programNrB)))
    {
        programsToInitialize[1] = presets[currentPreset].programNrB | 0x80;
    }
    else if (presets[currentPreset].programNrB == 0x3F)
    {
        programsToInitialize[1]=0x7e;
    }
    else
    {
        programsToInitialize[0]=0x7f | 0x80;
    }

    if (presets[currentPreset].programNrC != 0x3F && (audioProcessor.getFxProgram(2) == nullptr || 
        (audioProcessor.getFxProgram(2) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(2))->getIndex() != presets[currentPreset].programNrC)))
    {
        programsToInitialize[2] = presets[currentPreset].programNrC | 0x80;
    }
    else if (presets[currentPreset].programNrC == 0x3F)
    {
        programsToInitialize[2]=0x7e;
    }
    else
    {
        programsToInitialize[2]=0x7f | 0x80;
    }

    programChangeState = 1;
    if (ui.mode == PPFX_MODE_PRESETS)
    {
        setStompswitchColorRaw(presets[currentPreset].ledColorPreset << (currentPreset << 1));
    }
}