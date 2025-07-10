#ifndef _AUDIO_PROCESSOR_HPP_
#define _AUDIO_PROCESSOR_HPP_
#include <stdint.h>
namespace PiPicoFX {
    class AudioProcessor
    {
        public:
            virtual int16_t processSample(int16_t sampleIn);
    };
};
#endif