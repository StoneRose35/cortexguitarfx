
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace Compressor {
        class Compressor : public FxProgram
        {
            public:
                Compressor() : FxProgram(6,"Compressor"){
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
                Param1(Compressor* p) :FxProgramParameter(0,"Treshhold")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(Compressor* p) :FxProgramParameter(1,"Ratio")
                {
                    rawValue = 0;
                    increment = 512;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
        class Param3:  public FxProgramParameter
        {
            public:
                Param3(Compressor* p) :FxProgramParameter(2,"Makeup Gain")
                {
                    rawValue = 0;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
        class Param4:  public FxProgramParameter
        {
            public:
                Param4(Compressor* p) :FxProgramParameter(255,"Attack")
                {
                    rawValue = 4095;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
        class Param5:  public FxProgramParameter
        {
            public:
                Param5(Compressor* p) :FxProgramParameter(255,"Release")
                {
                    rawValue = 4095;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
        class Param6:  public FxProgramParameter
        {
            public:
                Param6(Compressor* p) :FxProgramParameter(255,"Flavor")
                {
                    rawValue = 0;
                    increment = 1024;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                Compressor * pData;
        };
    }
}
