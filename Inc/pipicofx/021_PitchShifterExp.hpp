#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/pitchshifter.h"
#include "audio/firstOrderIirFilter.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace PitchShifterExp {
        class PitchShifterExp : public FxProgram
        {
            public:
                PitchShifterExp() : FxProgram(3,"PS Exp",PS3_MAX_BUFFER_SIZE << 2,20){
                    this->setup(1);
                };
                PitchShifterExp(uint8_t discarded) : FxProgram(3,"PS Exp",PS3_MAX_BUFFER_SIZE << 2,20){
                    (void)discarded;
                    this->setup(0);
                };
                ~PitchShifterExp();
                FirstOrderIirType dcRemoval = {
                    .oldVal =0.0f,
                    .oldXVal = 0.0f,
                    .alpha = 0.98f
                };
                float mix;
                Pitchshifter3DataType pitchShifter3;
                float processSample(float);
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void setup(uint8_t);
        };


        class Param1: public FxProgramParameter
        {
            public:
                Param1(PitchShifterExp* p) :FxProgramParameter(0xff,"Tune")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifterExp * pData;

        };

        class Param2:  public FxProgramParameter
        {
            public:
                Param2(PitchShifterExp* p) :FxProgramParameter(0xff,"Mix")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifterExp * pData;
        };

        class Param3:  public FxProgramParameter
        {
            public:
                Param3(PitchShifterExp* p) :FxProgramParameter(0xff,"Volume")
                {
                    rawValue = 0x3FF;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                PitchShifterExp * pData;
        };
    }
}