
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/threebandeq.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Eq {
        class Eq : public FxProgram
        {
            public:
                Eq() : FxProgram(4,"3-Band EQ",0,11){
                    this->setup(1);
                };
                Eq(uint8_t discarded) : FxProgram(4,"3-Band EQ",0,11){
                    (void)discarded;
                    this->setup(0);
                };
                float processSample(float);
                ThreeBandEQType eq=
                {
                    .lowShelf = {
                        .x1=0.0f,
                        .x2=0.0f,
                        .y1=0.0f,
                        .y2=0.0f,
                        .acc=0.0f,
                        .coeffA = {-30948.0f/16384.0f, 14660.0f/16384.0f},
                        .coeffB = {23.0f/16384.0f, 47.0f/16384.0f, 23.0f/16384.0f}
                    },
                    .midBand = {
                        .x1=0.0f,
                        .x2=0.0f,
                        .y1=0.0f,
                        .y2=0.0f,
                        .acc=0.0f,
                        .coeffA = {-29699.0f/16384.0f, 13625.0f/16384.0f},
                        .coeffB = {1378.0f/16384.0f, 0.0f, -1378.0f/16384.0f}
                    },
                    .highShelf = {
                        .x1=0.0f,
                        .x2=0.0f,
                        .y1=0.0f,
                        .y2=0.0f,
                        .acc=0.0f,
                        .coeffA = {-26753.0f/16384.0f, 11314.0f/16384.0f},
                        .coeffB = {13612.0f/16384.0f, -27225.0f/16384.0f, 13612.0f/16384.0f}
                    },
                    .lowFactor=0.0f,
                    .midFactor=0.0f,
                    .highFactor=0.0f
                };
                GainStageDataType presetVolume={
                    .gain=1.0f,
                    .offset=0.0f
                };
            private:
                void setup(uint8_t);
        };

        class Param1:  public FxProgramParameter
        {
            public:
                Param1(Eq* p) :FxProgramParameter(0,"Low")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Eq* p) :FxProgramParameter(1,"Mid")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Eq* p) :FxProgramParameter(2,"High")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Eq* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Eq * pData;
        };
    }
}
