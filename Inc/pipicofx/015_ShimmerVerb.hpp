
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/pitchshifter.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/delay.h"
#include "audio/reverbUtils.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"

typedef struct 
{
    Pitchshifter2DataType pitchShifter;
    FirstOrderIirType glitterTamer;
} UnicornGlitterDataType;

int16_t unicornGlitter(int16_t sampleIn,void*data,volatile uint32_t * audioState);
}

namespace PiPicoFX {
    namespace ShimmerVerb {
        class ShimmerVerb : public FxProgram
        {
            public:
                ShimmerVerb() : FxProgram(4,"ShimmerVerb",15104<<1){
                    this->setup();
                };
                ~ShimmerVerb();
                int16_t processSample(int16_t);
                UnicornGlitterDataType unicornGlitterData = {
                    .pitchShifter={
                        .currentDelayPosition = 0,
                        .delayIncrement = 8,
                        .buffersizePowerTwo = 12,
                        .crossFadeWidthPwr2 = 10
                    },
                    .glitterTamer={
                        .oldVal = 0,
                        .oldXVal = 0,
                        .alpha = 20000
                    }
                };
                DelayDataType delays[4];
                AllpassType allpasses[2];
                int16_t oldVal=0;
                int16_t feedback=0;
                int16_t mix=0;
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
                Param1(ShimmerVerb* p) :FxProgramParameter(0,"Shimmer")
                {
                    rawValue = 0;
                    increment = 512;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                ShimmerVerb * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(ShimmerVerb* p) :FxProgramParameter(1,"Decay")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                ShimmerVerb * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(ShimmerVerb* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                ShimmerVerb * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(ShimmerVerb* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                ShimmerVerb * pData;
        };
    }
}
