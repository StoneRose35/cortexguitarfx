
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

float unicornGlitter(float sampleIn,void*data);
}

namespace PiPicoFX {
    namespace ShimmerVerb {
        class ShimmerVerb : public FxProgram
        {
            public:
                ShimmerVerb() : FxProgram(4,"ShimmerVerb",15104<<2,14){
                    this->setup(1);
                };
                ShimmerVerb(uint8_t discarded) : FxProgram(4,"ShimmerVerb",15104<<2,14){
                    (void)discarded;
                    this->setup(0);
                };
                ~ShimmerVerb();
                float processSample(float);
                UnicornGlitterDataType unicornGlitterData = {
                    .pitchShifter={
                        .delayMemoryPtr=0,
                        .delayPointer1=0,
                        .delayPointer2=0,
                        .currentDelayPosition = 0,
                        .delayIncrement = 8,
                        .buffersizePowerTwo = 12,
                        .buffersize=0,
                        .crossFadeWidth=0,
                        .crossFadeWidthPwr2 = 10
                    },
                    .glitterTamer={
                        .oldVal = 0,
                        .oldXVal = 0,
                        .alpha = 20000.0/32768.0f
                    }
                };
                DelayDataType delays[4];
                AllpassType allpasses[2];
                float oldVal=0.0f;
                float feedback=0.0f;
                float mix=0.0f;
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
