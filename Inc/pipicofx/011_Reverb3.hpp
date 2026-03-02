
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/reverb3.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Reverb3 {
        class Reverb3 : public FxProgram
        {
            public:
                Reverb3() : FxProgram(3,"MatrixReverb",(16*DIFFUSOR_SIZE + 4096)<<2){
                    this->setup();
                };
                ~Reverb3();
                float processSample(float);
                float mix;
                Reverb3Type reverb;
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void setup();
                void freeze() override;
                void unfreeze() override;
                void onFreeze() override;
                void onMelt() override;
                float meltedFeedbackValue;
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
