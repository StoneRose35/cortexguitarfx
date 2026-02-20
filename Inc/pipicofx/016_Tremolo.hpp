
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/tremolo.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Tremolo {
        class Tremolo : public FxProgram
        {
            public:
                Tremolo() : FxProgram(5,"Tremolo",0){
                    this->setup();
                };
                float processSample(float);
                TremoloType tremolo={
                    .depth=0,
                    .lfoUpdateCnt=0,
                    .currentLfoVal=0,
                    .nextLfoVal=0,
                    .modulator = {
                        .squareRatio=0,
                        .phaseIncrement=131,
                        .phaseIncrementCorrection1=0,
                        .phaseIncrementCorrection2=0,
                        .currentPhase=0,
                        .pulseWidth=0
                    }
                };
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
                Param1(Tremolo* p) :FxProgramParameter(0,"Rate")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Tremolo * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Tremolo* p) :FxProgramParameter(1,"Depth")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Tremolo * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Tremolo* p) :FxProgramParameter(2,"Shape")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Tremolo * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Tremolo* p) :FxProgramParameter(255,"Pulse Width")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Tremolo * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(Tremolo* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Tremolo * pData;
        };
    }
}
