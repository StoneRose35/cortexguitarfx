
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/pitchshifter.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace PitchShifter {
        class PitchShifter : public FxProgram
        {
            public:
                PitchShifter() : FxProgram(4,"Pitchshifter",8192<<2){
                    this->setup();
                };
                ~PitchShifter();
                int16_t processSample(int16_t);
                GainStageDataType presetVolume={
                    .gain=0xff,
                    .offset=0
                };
                Pitchshifter2DataType  pitchShifter={
                    .delayMemoryPtr = 0,
                    .delayPointer1 = 0,
                    .delayPointer2 = 0,
                    .currentDelayPosition=0,
                    .delayIncrement=0x4,
                    .buffersizePowerTwo=13,
                    .buffersize = 0,
                    .crossFadeWidth = 0,
                    .crossFadeWidthPwr2=11,
                };
                int16_t mix;

            private:
                void setup();
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(PitchShifter* p) :FxProgramParameter(0,"ShiftAmt")
                {
                    rawValue = 0;
                    increment = 512;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifter * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(PitchShifter* p) :FxProgramParameter(1,"Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifter * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(PitchShifter* p) :FxProgramParameter(2,"AvgDelay")
                {
                    rawValue = 0;
                    increment = 512;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifter * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(PitchShifter* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifter * pData;
        };
    }
}
