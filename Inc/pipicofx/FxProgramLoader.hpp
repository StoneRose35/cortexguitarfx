#include "stdint.h"
#include "FxProgram.hpp"
#include "001_AmpModel.hpp"
#include "002_VibChorus.hpp"
#include "003_Off.hpp"
#include "004_MonsterCrusher.hpp"
#include "005_Delay.hpp"
#include "006_Compressor.hpp"
#include "007_AmpModelHighGain.hpp"
#include "008_Reverb.hpp"
#include "009_SineModulation.hpp"
#include "010_Reverb2.hpp"
#include "011_Reverb3.hpp"
#include "012_Eq.hpp"
#include "013_AcousticProc.hpp"
#include "014_PitchShifter.hpp"
#include "015_ShimmerVerb.hpp"
#include "016_Tremolo.hpp"
#include "017_FreeVerb.hpp"
#include <new>
#define N_FX_PROGRAMS 17
namespace PiPicoFX {
    PiPicoFX::FxProgram * loadProgram(uint8_t index);
    
}