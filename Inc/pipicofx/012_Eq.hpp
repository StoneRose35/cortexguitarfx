
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Eq {
        class Eq : public FxProgram
        {
            public:
                Eq() : FxProgram(4,"3-Band Equalizer"){
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
                Param1(Eq* p) :FxProgramParameter(0,"Low")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Eq* p) :FxProgramParameter(1,"Mid")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Eq* p) :FxProgramParameter(2,"High")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Eq* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
    }
}
