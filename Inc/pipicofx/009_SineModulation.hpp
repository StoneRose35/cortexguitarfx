
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace SineModulation {
        class SineModulation : public FxProgram
        {
            public:
                SineModulation() : FxProgram(6,"Sine Modulation"){
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
                Param1(SineModulation* p) :FxProgramParameter(0,"Frequency")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(SineModulation* p) :FxProgramParameter(1,"Depth")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(SineModulation* p) :FxProgramParameter(2,"Blend")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(SineModulation* p) :FxProgramParameter(255,"Offset")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(SineModulation* p) :FxProgramParameter(255,"Feedback")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
        class Param6:  public FxProgramParameter
        {
            public:
                Param6(SineModulation* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                SineModulation * pData;
        };
    }
}
