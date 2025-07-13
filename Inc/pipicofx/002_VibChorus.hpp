#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace VibChorus {
        class VibChorus : public FxProgram
        {
            public:
                VibChorus() : FxProgram(4,"Vibrato/Chorus"){
                    this->setup();
                };
                int16_t processSample(int16_t);
                SimpleChorusType chorusData={        
                    .frequency = 500,
                    .depth = 10,
                    .mix = 128};
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
                Param1(VibChorus* p) :FxProgramParameter(0,"Frequency")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                VibChorus * pData;
        };

        class Param2:  public FxProgramParameter
        {
            public:
                Param2(VibChorus* p) :FxProgramParameter(1,"Depth")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                VibChorus * pData;
        };

        class Param3:  public FxProgramParameter
        {
            public:
                Param3(VibChorus* p) :FxProgramParameter(2,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                VibChorus * pData;
        };

        class Param4:  public FxProgramParameter
        {
            public:
                Param4(VibChorus* p) :FxProgramParameter(0xff,"Volume")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                VibChorus * pData;
        };

    };
};