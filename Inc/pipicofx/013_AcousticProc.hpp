
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/threebandeq.h"
#include "audio/compressor.h"
#include "audio/reverb.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace AcousticProc {
        class AcousticProc : public FxProgram
        {
            public:
                AcousticProc() : FxProgram(7,"Acoustic Proc",20480<<2,12){
                    this->setup(1);
                };
                AcousticProc(uint8_t discarded) : FxProgram(7,"Acoustic Proc",20480<<2,12){
                    (void)discarded;
                    this->setup(0);
                };
                ~AcousticProc();
                float processSample(float);
                float reverbTime;
                ThreeBandEQType eq={
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
                CompressorDataType comp={
                    .gainFunction = {
                        .threshhold = 1.0f,
                        .gainReduction = 2.0f
                    },
                    .avgLowpass = {
                        .oldVal=0.0f,
                        .oldXVal=0.0f,
                        .alphaRising = 15.0f/32768.0f,
                        .alphaFalling = 32765.0f/32768.0f,
                    },
                    .currentAvg=0.0f
                };
                GainStageDataType postGain={
                    .gain = 1.0f,
                    .offset=0.0f
                };
                ReverbType reverb={
                    .allpasses={},
                    .delayPointer=0,
                    .feedbackValues={},
                    .delayPointers={},
                    .mix = 0.0f,
                    .gainIn =1.0f,
                    .paramNr = 1,
                    .frozen=0
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
                Param1(AcousticProc* p) :FxProgramParameter(0,"EQ Low")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(AcousticProc* p) :FxProgramParameter(1,"EQ Mid")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(AcousticProc* p) :FxProgramParameter(2,"EQ High")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(AcousticProc* p) :FxProgramParameter(255,"Compressor Int.")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(AcousticProc* p) :FxProgramParameter(255,"Reverb Time")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param6:  public FxProgramParameter
        {
            public:
                Param6(AcousticProc* p) :FxProgramParameter(255,"Reverb Mix")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
        class Param7:  public FxProgramParameter
        {
            public:
                Param7(AcousticProc* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AcousticProc * pData;
        };
    }
}
