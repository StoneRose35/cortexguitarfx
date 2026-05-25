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
                VibChorus() : FxProgram(4,"Vibrato/Chorus",2048,1){
                    this->setup(1);
                };

                VibChorus(uint8_t discarded) : FxProgram(4,"Vibrato/Chorus",2048,1){
                    (void)discarded;
                    this->setup(0);
                };
                float processSample(float);
                SimpleChorusType chorusData={   
                    .delayBuffer=0,     
                    .frequency = 500,
                    .depth = 10,
                    .mix = 0.5f,
                    .lfoVal=0,
                    .lfoValOld=0,
                    .lfoQuadrant=0,
                    .lfoPhaseinc=0,
                    .delayInputPtr=0,
                    .lfoUpdateCnt=0
                };
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
                ~VibChorus();
            private:
                void setup(uint8_t);
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