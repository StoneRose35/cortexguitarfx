
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/delay.h"
#include "audio/reverbUtils.h"
#include "picofxCore.hpp"
int16_t freeVerbLowpass(int16_t sampleIn,void * filterData,volatile uint32_t * audioStatePtr);
}

namespace PiPicoFX {
    namespace FreeVerb {
        class FreeVerb : public FxProgram
        {
            public:
                FreeVerb() : FxProgram(4,"FreeVerb",24576<<1){
                    this->setup();
                };
                ~FreeVerb();
                int16_t processSample(int16_t);
                DelayDataType delays[8]= {
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1695,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1760,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1623,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1548,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1390,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1476,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1293,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0},
                    {.delayLine = 0,.delayLinePtr = 0, .delayInSamples = 1215,.feedback = 0, .delayBufferLength=2048,.mix=0, .feedbackFunction=freeVerbLowpass, .feebackData = 0}
                };
                FirstOrderIirType feedbackFilters[8]={
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554,
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    },
                    {
                        .oldVal=0,.oldXVal=0,.alpha = 6554
                    }
                };
                AllpassType allpasses[4]={
                    {.coefficient=16384,.delayPtr=0,.delayInSamples=245,.oldValues=0,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=16384,.delayPtr=0,.delayInSamples=605,.oldValues=0,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=16384,.delayPtr=0,.delayInSamples=480,.oldValues=0,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=16384,.delayPtr=0,.delayInSamples=371,.oldValues=0,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0}
                };
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
                Param1(FreeVerb* p) :FxProgramParameter(0,"Decay")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(FreeVerb* p) :FxProgramParameter(1,"Damping")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(FreeVerb* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(FreeVerb* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                FreeVerb * pData;
        };
    }
}
