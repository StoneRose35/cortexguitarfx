#pragma once
#include "AudioProcessor.hpp"

using namespace  std;

namespace PiPicoFX {
    enum MultiAudioProcessorMode
    {
        MODE_SERIAL,MODE_PARALLEL
    };

    class MultiAudioProcessor: public AudioProcessor
    {
        public:
            MultiAudioProcessor();
            ~MultiAudioProcessor();
            int16_t processSample(int16_t);
            AudioProcessor * getChild(uint8_t pos);
            uint8_t getLength();
            void appendChild(AudioProcessor*);
            void replaceChild(AudioProcessor*,uint8_t pos);
            void removeChild(uint8_t pos);
            MultiAudioProcessorMode mode=MODE_SERIAL;

        private:
            AudioProcessor ** children;
            uint8_t bufferLength;
            uint8_t length;
    };
}
