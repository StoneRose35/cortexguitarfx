
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/delay.h"
#include "picofxCore.hpp"
}
#define FXPROGRAM_DELAY_DELAY_TIME_LOWPASS_T 2

namespace PiPicoFX {
    namespace Delay {
        class Delay : public FxProgram
        {
            public:
                Delay() : FxProgram(4,"Delay",MAX_DELAY_SINGLEBUFFER<<2,4){
                    this->setup(1);
                };
                Delay(uint8_t discarded) : FxProgram(4,"Delay",MAX_DELAY_SINGLEBUFFER<<2,4){
                    (void)discarded;
                    this->setup(0);
                };
                float processSample(float);
                DelayDataType delay={
                    .delayLine=0,
                    .delayLinePtr=0,
                    .delayInSamples=10,
                    .feedback=0.0f,
                    .delayBufferLength=0xff,
                    .mix=0.0f,
                    .gainIn=1.0f,
                    .feedbackFunction=0,
                    .feebackData=0,
                    .frozen = 0
                };
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
                ~Delay();
                void freeze() override;
                void unfreeze() override;
                void onFreeze() override;
                void onMelt() override;
                int32_t delayInSamplesTargetValue;
                uint16_t interpCnt;
            private:
                void setup(uint8_t);
                float meltedFeedbackValue;
                
               
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Delay* p) :FxProgramParameter(0,"Time")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Delay* p) :FxProgramParameter(1,"Feedback")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Delay* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Delay* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Delay * pData;
        };
    }
}
