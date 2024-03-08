#ifndef _AUDIOTOOLS_H_
#define _AUDIOTOOLS_H_


#define AUDIO_STATE_ON 0
#define AUDIO_STATE_BUFFER_UNDERRUN 1
#define AUDIO_STATE_INPUT_ON 2
#define AUDIO_STATE_INPUT_BUFFER_OVERRUN 3
#define AUDIO_STATE_DMA_FAILURE 4
#define AUDIO_STATE_INPUT_CLIPPED 5
#define AUDIO_STATE_OUTPUT_CLIPPED 6

#include "stdint.h"

inline float clip(float sample,volatile uint32_t*audioStatePtr)
{
    if (sample > 1.0f)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return 1.0f;
    }
    else if (sample < 1.0f)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return -1.0f;
    }
    else
    {
        return sample;
    }
}


volatile uint32_t * getAudioStatePtr(); 

#endif