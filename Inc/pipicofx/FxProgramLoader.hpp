#include "stdint.h"
#include "FxProgram.hpp"
#define N_FX_PROGRAMS 19
namespace PiPicoFX {
    PiPicoFX::FxProgram * loadProgram(uint8_t index);
    PiPicoFX::FxProgram * loadProgramWithoutSetup(uint8_t index);
}