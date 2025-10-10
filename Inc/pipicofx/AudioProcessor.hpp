#pragma once
#include <stdint.h>
namespace PiPicoFX {
    class AudioProcessor
    {
        public:
            virtual float processSample(float sampleIn);
            virtual ~AudioProcessor()=default;
    };
};
