
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/sineChorus.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace SineModulation {
        class SineModulation : public FxProgram
        {
            public:
                SineModulation() : FxProgram(6,"Sine Modulation",SINE_CHORUS_DELAY_SIZE<<2,8){
                    this->setup();
                };
                ~SineModulation();
                float processSample(float);
                SineChorusType sineChorus= {
                    .delayBuffer=0,
                    .frequency = 500,
                    .depth = 10,
                    .mix = 0.5f,
                    .offset = 49,
                    .feedback = 0.0f,
                    .lfoVal=0,
                    .lfoValOld=0,
                    .lfoPhaseinc=0,
                    .lfoPhase=0,
                    .delayInputPtr=0,
                    .lfoUpdateCnt=0
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
