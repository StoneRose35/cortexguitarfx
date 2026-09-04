#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "picofxCore.hpp"
#include "globalConfig.h"

}

namespace PiPicoFX {
    namespace MultimodeFilter {
        class MultimodeFilter : public FxProgram
        {
            public:
                MultimodeFilter(): FxProgram(4,"MMFilter",0,17)
                {
                    this->setup(1);
                };
                MultimodeFilter(uint8_t discarded): FxProgram(4,"MMFilter",0,17)
                {
                    (void)discarded;
                    this->setup(0);
                };
                ~MultimodeFilter();
                float processSample(float);
                MultimodeFilterType mmfilter = {
                    .interm0 = 0.0f,
                    .interm1 = 0.0f,
                    .cutoff = 0.97f,
                    .resonance = 0.0f,
                    .feedback = 0.0f,
                    .type = MM_FILTER_LOWPASS
                };

                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
                uint16_t newCutoff=0.97f;
                uint16_t newResonance=0.0f;
                uint16_t interpCnt=UI_LATENCY_IN_SAMPLES;
            private:
                void setup(uint8_t);

        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(MultimodeFilter* p) :FxProgramParameter(0,"Cutoff")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MultimodeFilter * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(MultimodeFilter* p) :FxProgramParameter(1,"Resonance")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MultimodeFilter * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(MultimodeFilter* p) :FxProgramParameter(2,"Type")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MultimodeFilter * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(MultimodeFilter* p) :FxProgramParameter(0xff,"Volume")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MultimodeFilter * pData;
        };
    }
}