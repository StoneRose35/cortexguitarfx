#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "picofxCore.hpp"

}

namespace PiPicoFX {
    namespace MultimodeFilter {
        class MultimodeFilter : public FxProgram
        {
            public:
                MultimodeFilter(): FxProgram(4,"MMFilter",0)
                {
                    this->setup();
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
                uint16_t newCutoff;
                uint16_t newResonance;
                uint16_t interpCnt=0;
            private:
                void setup();

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