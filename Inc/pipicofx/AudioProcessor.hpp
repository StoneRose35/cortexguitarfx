#pragma once
#include <stdint.h>
namespace PiPicoFX {
    class AudioProcessor
    {
        public:
            virtual int16_t processSample(int16_t sampleIn);
    };
};
