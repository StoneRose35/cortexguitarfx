
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/reverb2.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Reverb2 {
        class Reverb2 : public FxProgram
        {
            public:
                Reverb2() : FxProgram(3,"Allpass Reverb",24576<<2,9){
                    this->setup();
                };
                ~Reverb2();
                float processSample(float);
                Reverb2Type reverb;
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void freeze() override;
                void unfreeze() override;
                void onFreeze() override;
                void onMelt() override;
                float meltedDecay;
                void setup();
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Reverb2* p) :FxProgramParameter(0,"Decay")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb2 * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Reverb2* p) :FxProgramParameter(1,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb2 * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Reverb2* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Reverb2 * pData;
        };
    }
}
