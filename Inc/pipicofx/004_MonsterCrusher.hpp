
#pragma once
#include "FxProgram.hpp"
extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/bitcrusher.h"
#include "picofxCore.hpp"
}

namespace PiPicoFX {
    namespace MonsterCrusher {
        class MonsterCrusher : public FxProgram
        {
            public:
                MonsterCrusher() : FxProgram(2,"Monstercrusher",0){
                    this->setup();
                };
                int16_t processSample(int16_t);

                BitCrusherDataType bitcrusher={
                    .bitmask = 0x8000,
                    .halfvalue = 0x4000
                };
                uint8_t resolution;
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
                Param1(MonsterCrusher* p) :FxProgramParameter(0,"Resolution")
                {
                    rawValue = 0;
                    increment = 256;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MonsterCrusher * pData;
        };
        class Param2:  public FxProgramParameter
        {
            public:
                Param2(MonsterCrusher* p) :FxProgramParameter(255,"Volume")
                {
                    rawValue = 1023;
                    increment = 1;
                    pData=p;
                };
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                MonsterCrusher * pData;
        };
    }
}
