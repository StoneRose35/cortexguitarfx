#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Boost {
        class Boost : public FxProgram
        {
            public:
                Boost() : FxProgram(1,"Boost",0,2){
                    this->setup();
                };
                float processSample(float);
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void setup();
        };


        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Boost* p) :FxProgramParameter(0xff,"Volume")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Boost * pData;
        };
    }
}