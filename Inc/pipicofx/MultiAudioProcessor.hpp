#pragma once
#include <stdint.h>
#include "AudioProcessor.hpp"

#define MULTI_AUDIO_PROCESSOR_DEFAULT_LENGTH 3

#define MULTI_AUDIO_PROCESSOR_ROUTING_3S 0 // all three in serie
#define MULTI_AUDIO_PROCESSOR_ROUTING_3P 1 // all 3 in parallel

/*             ---1---  
              |       |
  In --- 0 ---         ---- Out
              |       |
               ---2---
*/
#define MULTI_AUDIO_PROCESSOR_ROUTING_S_2P 2

/*       ---0---  
        |       |
  In ---         ---2--- Out
        |       |
         ---1---
*/
#define MULTI_AUDIO_PROCESSOR_ROUTING_2P_S 3


/*       ---0---1---  
        |           |
  In ---             --- Out
        |           |
         -----2-----
*/
#define MULTI_AUDIO_PROCESSOR_ROUTING_2S_P 4

namespace PiPicoFX
{
    class MultiAudioProcessor : public AudioProcessor
    {
        public:
            MultiAudioProcessor();
            uint8_t addFxProgram(AudioProcessor*prg,uint8_t pos);
            AudioProcessor * removeFxProgram(uint8_t pos);
            AudioProcessor * getFxProgram(uint8_t pos);
            void swapPrograms(uint8_t posA,uint8_t posB);
            uint8_t getProgramListLength();
            virtual ~MultiAudioProcessor();
            float processSample(float s);
            uint8_t getRouting();
            uint8_t setRouting(uint8_t r);
        private:
            AudioProcessor ** processors;
            uint8_t processorArrayLength;
            uint8_t routing;
    };
}
