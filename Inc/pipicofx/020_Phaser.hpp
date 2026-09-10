#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/delay.h"
#include "audio/genericDistortionSimple.h"
#include "picofxCore.hpp"
}

//#define M_PI 3.14159265358979f
//#define M_2_PI 6.28318530717959f

#define M_PI (3.14159f)
#define M_2_PI (6.28318f)

namespace PiPicoFX {
    namespace Phaser {
        class Phaser : public FxProgram
        {
            public:
                Phaser(): FxProgram(5,"Phaser",0,19)
                {
                    this->setup(1);
                };
                Phaser(uint8_t discarded): FxProgram(5,"Phaser",0,19)
                {
                    (void)discarded;
                    this->setup(0);
                };
                //~Phaser();
                float processSample(float);

                float lfoPhase=0.0f;
                float lfoPhaseInc=0.001f;
                float mix=0.5f;
                float oldVal = 0.0f;
                float feedback =0.0f;
                float depth=1.0f;
                int16_t frequency=1;
                FirstOrderIirAllpassType aps[8]={
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                                        {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                    {
                    .oldVal=0.0f,
                    .oldYVal=0.0f,
                    .alpha=0.7f
                    },
                };

                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void setup(uint8_t);
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Phaser* p) :FxProgramParameter(0,"Speed")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Phaser * pData;
        };

        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Phaser* p) :FxProgramParameter(1,"Depth")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Phaser * pData;
        };

        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Phaser* p) :FxProgramParameter(3,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Phaser * pData;
        };

        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Phaser* p) :FxProgramParameter(4,"Feedback")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Phaser * pData;
        };

        class Param5:  public FxProgramParameter
        {
            public:
                Param5(Phaser* p) :FxProgramParameter(5,"Volume")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Phaser * pData;
        };
    }
}