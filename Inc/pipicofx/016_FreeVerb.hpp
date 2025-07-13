
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace FreeVerb {
        class FreeVerb : public FxProgram
        {
            public:
                FreeVerb() : FxProgram(4,"FreeVerb"){
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
                Param1(FreeVerb* p) :FxProgramParameter(0,"Decay")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(FreeVerb* p) :FxProgramParameter(1,"Damping")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(FreeVerb* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(FreeVerb* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
    }
}
