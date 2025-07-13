
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace AmpModelHighGain {
        class AmpModelHighGain : public FxProgram
        {
            public:
                AmpModelHighGain() : FxProgram(6,"Amp High Gain"){
                    this->setup();
                };
                int16_t processSample(int16_t);
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
                Param1(AmpModelHighGain* p) :FxProgramParameter(0,"Hi-Cut")
                {
                    rawValue = 4095;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(AmpModelHighGain* p) :FxProgramParameter(1,"Gain")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(AmpModelHighGain* p) :FxProgramParameter(2,"Mod Intensity")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(AmpModelHighGain* p) :FxProgramParameter(255,"Mod Type")
                {
                    rawValue = 0;
                    increment = 2048;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(AmpModelHighGain* p) :FxProgramParameter(255,"Cab Type")
                {
                    rawValue = 0;
                    increment = 256;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
        class Param6:  public FxProgramParameter
        {
            public:
                Param6(AmpModelHighGain* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                AmpModelHighGain * pData;
        };
    }
}
