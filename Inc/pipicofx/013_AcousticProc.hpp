
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
                AcousticProc() : FxProgram(7,"Acoustic Proc",20480<<2){
                    this->setup();
                };
                ~AcousticProc();
                float processSample(float);
                float reverbTime;
                ThreeBandEQType eq={
                    .lowFactor = 0.0f,
                    .midFactor = 0.0f,
                    .highFactor = 0.0f,
                };
                CompressorDataType comp={
                    .gainFunction = {
                        .threshhold = 1.0f,
                        .gainReduction = 2.0f
                    },
                    .avgLowpass = {
                        .alphaRising = 15.0f/32768.0f,
                        .alphaFalling = 32765.0f/32768.0f,
                    }
                };
                GainStageDataType postGain={
                    .gain = 1.0f
                };
                ReverbType reverb={
                    .mix = 0.0f,
                    .paramNr = 1
                };
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
