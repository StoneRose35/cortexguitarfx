#include "stdint.h"
#include "memoryRegions.h"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/001_AmpModel.hpp"
#include "pipicofx/002_VibChorus.hpp"
#include "pipicofx/003_Boost.hpp"
#include "pipicofx/004_MonsterCrusher.hpp"
#include "pipicofx/005_Delay.hpp"
#include "pipicofx/006_Compressor.hpp"
#include "pipicofx/007_AmpModelHighGain.hpp"
#include "pipicofx/008_Reverb.hpp"
#include "pipicofx/009_SineModulation.hpp"
#include "pipicofx/010_Reverb2.hpp"
#include "pipicofx/011_Reverb3.hpp"
#include "pipicofx/012_Eq.hpp"
#include "pipicofx/013_AcousticProc.hpp"
#include "pipicofx/014_PitchShifter.hpp"
#include "pipicofx/015_ShimmerVerb.hpp"
#include "pipicofx/016_Tremolo.hpp"
#include "pipicofx/017_FreeVerb.hpp"
#include "pipicofx/018_MultimodeFilter.hpp"
#include "pipicofx/019_XAmp.hpp"
#include <new>
namespace PiPicoFX {
    __QSPI_CODE
    PiPicoFX::FxProgram * loadProgram(uint8_t index)
    {
        PiPicoFX::FxProgram * res=nullptr;
        switch(index)
        {
            case 0:
                res = new(std::nothrow)  PiPicoFX::AmpModel::AmpModel();
                break;
            case 1:
                res = new(std::nothrow) PiPicoFX::VibChorus::VibChorus();
                break;
            case 2:
                res = new(std::nothrow) PiPicoFX::Boost::Boost();
                break;
            case 3:
                res = new(std::nothrow) PiPicoFX::MonsterCrusher::MonsterCrusher();
                break;
            case 4:
                res = new(std::nothrow) PiPicoFX::Delay::Delay();
                break;
            case 5:
                res = new(std::nothrow) PiPicoFX::Compressor::Compressor();
                break;
            case 6:
                res = new(std::nothrow) PiPicoFX::AmpModelHighGain::AmpModelHighGain();
                break;
            case 7:
                res = new(std::nothrow) PiPicoFX::Reverb::Reverb();
                break;
            case 8:
                res = new(std::nothrow) PiPicoFX::SineModulation::SineModulation();
                break;
            case 9:
                res = new(std::nothrow) PiPicoFX::Reverb2::Reverb2();
                break;
            case 10:
                res = new(std::nothrow) PiPicoFX::Reverb3::Reverb3();
                break;
            case 11:
                res = new(std::nothrow) PiPicoFX::Eq::Eq();
                break;
            case 12:
                res = new(std::nothrow) PiPicoFX::AcousticProc::AcousticProc();
                break;
            case 13:
                res = new(std::nothrow) PiPicoFX::PitchShifter::PitchShifter();
                break;
            case 14:
                res = new(std::nothrow) PiPicoFX::ShimmerVerb::ShimmerVerb();
                break;
            case 15:
                res = new(std::nothrow) PiPicoFX::Tremolo::Tremolo();
                break;
            case 16:
                res = new(std::nothrow) PiPicoFX::FreeVerb::FreeVerb();
                break;
            case 17:
                res = new(std::nothrow) PiPicoFX::MultimodeFilter::MultimodeFilter();
                break;
            case 18:
                res = new(std::nothrow) PiPicoFX::XAmp::XAmp();
            default:
                break;

        }
        return res;
    }
    
}