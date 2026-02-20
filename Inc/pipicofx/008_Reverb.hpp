
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/reverb.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Reverb {
        class Reverb : public FxProgram
        {
            public:
                Reverb() : FxProgram(4,"Reverb",20480<<2){
                    this->setup();
                };
                ~Reverb();
                float processSample(float);
                ReverbType reverb={
                    .allpasses={},
                    .delayPointer=0,
                    .feedbackValues={},
                    .delayPointers={},
                    .mix=0.0f,
                    .paramNr = 0,
                    .frozen=0
                };
                float reverbTime=300;
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void freeze() override;
                void unfreeze() override;
                void setup();
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Reverb* p) :FxProgramParameter(0,"Time")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Reverb* p) :FxProgramParameter(1,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Reverb* p) :FxProgramParameter(2,"Planet")
                {
                    rawValue = 0;
                    increment = 1024;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Reverb* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb * pData;
        };
    }
}
