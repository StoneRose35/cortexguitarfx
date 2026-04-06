#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "audio/delay.h"
#include "audio/genericDistortionSimple.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace XAmp {
        class XAmp : public FxProgram
        {
            public:
                XAmp(): FxProgram(4,"XAmp",MAX_DELAY_SINGLEBUFFER<<2,18)
                {
                    this->setup();
                };
                ~XAmp();
                float processSample(float);

                GainStageDataType gain={
                    .gain=0.5f,
                    .offset=0.0f
                };

                GenericDistortionSimpleType distortion;

                MultimodeFilterType lowpass = {
                    .interm0 = 0.0f,
                    .interm1 = 0.0f,
                    .cutoff = 0.77f,
                    .resonance = 0.0f,
                    .feedback = 0.0f,
                    .type = MM_FILTER_LOWPASS
                };
                DelayDataType delay={.delayLine=0,
                    .delayLinePtr=0,
                    .delayInSamples=0,
                    .feedback=0.0f,
                    .delayBufferLength=0,
                    .mix=0.0f,
                    .gainIn=1.0f,
                    .feedbackFunction=0,
                    .feebackData=0,
                    .frozen=0};
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
                Param1(XAmp* p) :FxProgramParameter(0,"Gain")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                XAmp * pData;
        };

        class Param2:  public FxProgramParameter
        {
            public:
                Param2(XAmp* p) :FxProgramParameter(1,"Lowpass")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                XAmp * pData;
        };

        class Param3:  public FxProgramParameter
        {
            public:
                Param3(XAmp* p) :FxProgramParameter(2,"Delay Int")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                XAmp * pData;
        };

        class Param4:  public FxProgramParameter
        {
            public:
                Param4(XAmp* p) :FxProgramParameter(3,"Volume")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                XAmp * pData;
        };
    }
}