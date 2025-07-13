
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Reverb3 {
        class Reverb3 : public FxProgram
        {
            public:
                Reverb3() : FxProgram(3,"Hadamard Reverb"){
                    this->setup();
                };
                int16_t processSample(int16_t);
                GainStageDataType presetVolume={
                    .gain=0xff,
                    .offset=0
                };
            private:
                void setup();
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Reverb3* p) :FxProgramParameter(0,"Decay")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb3 * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Reverb3* p) :FxProgramParameter(1,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb3 * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Reverb3* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb3 * pData;
        };
    }
}
