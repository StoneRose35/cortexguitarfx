
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.h"
}

namespace PiPicoFX {
    namespace Delay {
        class Delay : public FxProgram
        {
            public:
                Delay() : FxProgram(4,"Delay"){
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
                Param1(Delay* p) :FxProgramParameter(0,"Time")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Delay* p) :FxProgramParameter(1,"Feedback")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Delay* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Delay* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
    }
}
