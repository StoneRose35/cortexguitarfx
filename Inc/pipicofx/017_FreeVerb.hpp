
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
float freeVerbLowpass(float sampleIn,void * filterData);
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
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1695,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1760,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1623,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1548,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1390,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1476,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1293,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0},
                    {.delayLine=0,.delayLinePtr=0,.delayInSamples = 1215,.feedback=0.0f,.delayBufferLength=2048,.mix=0.0f,.gainIn=1.0f,.feedbackFunction=freeVerbLowpass,.feebackData=0,.frozen=0}
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
                    {.coefficient=0.5f,.delayPtr=0,.delayInSamples = 245,.oldValues=0.0f,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=0.5f,.delayPtr=0,.delayInSamples = 605,.oldValues=0.0f,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=0.5f,.delayPtr=0,.delayInSamples = 480,.oldValues=0.0f,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0},
                    {.coefficient=0.5f,.delayPtr=0,.delayInSamples = 371,.oldValues=0.0f,.bufferSize=1023,.delayLineIn=0,.delayLineOut=0}
                };
                float mix=0.0f;
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void freeze() override;
                void unfreeze() override;
                void onFreeze() override;
                void onMelt() override;
                void setup();
                float meltedFeedbackValue;
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
