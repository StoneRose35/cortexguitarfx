#pragma once
#include "stdint.h"
#include "picofxCore.hpp"

void setPresetNr(uint8_t nr);
void setPresetAtBank(uint8_t bankNr,uint8_t presetNr);
void setPreset();
void reloadPresetsFromEeprom(FxPresetType*priis,uint8_t bnk);
