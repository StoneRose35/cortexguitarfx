
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
float freeVerbLowpass(float sampleIn,void * filterData,volatile uint32_t * audioStatePtr);
}

namespace PiPicoFX {
    namespace FreeVerb {
        class FreeVerb : public FxProgram
        {
            public:
                FreeVerb() : FxProgram(4,"FreeVerb",24576<<2){
                    this->setup();
                };
                ~FreeVerb();
                float processSample(float);
                DelayDataType delays[8]= {
                    {.delayInSamples = 1695,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1760,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1623,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1548,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1390,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1476,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1293,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass},
                    {.delayInSamples = 1215,.delayBufferLength=2048,.feedbackFunction=freeVerbLowpass}
                };
                FirstOrderIirType feedbackFilters[8]={
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=0.0f,.alpha = 0.2f
                    },
                    {
                        .oldVal=0.0f,.oldXVal=00.f,.alpha = 0.2f
                    }
                };
                AllpassType allpasses[4]={
                    {.coefficient=0.5f,.delayInSamples=245,
                        .bufferSize=1023},
                    {.coefficient=0.5f,.delayInSamples = 605,
                        .bufferSize=1023},
                    {.coefficient=0.5f,.delayInSamples = 480,
                        .bufferSize=1023},
                    {.coefficient=0.5f,.delayInSamples = 371,
                        .bufferSize=1023}
                };
                float mix=0.0f;
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
