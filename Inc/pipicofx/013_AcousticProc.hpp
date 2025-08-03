
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
                AcousticProc() : FxProgram(7,"Acoustic Proc",20480<<1){
                    this->setup();
                };
                ~AcousticProc();
                int16_t processSample(int16_t);
                int16_t reverbTime;
                ThreeBandEQType eq={
                    .lowFactor = 0,
                    .midFactor = 0,
                    .highFactor = 0,
                };
                CompressorDataType comp={
                    .gainFunction = {
                        .threshhold = 0x7FFF,
                        .gainReduction = 1
                    },
                    .avgLowpass = {
                        .alphaRising = 15,
                        .alphaFalling = 32765,
                    }
                };
                GainStageDataType postGain={
                    .gain = 0x100
                };
                ReverbType reverb={
                    .mix = 0,
                    .paramNr = 1
                };
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
