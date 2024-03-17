#include "stdint.h"
#include "audio/audiotools.h"

static volatile uint32_t audioState=0;

#ifndef STM32
float clip(float sample,volatile uint32_t*audioStatePtr)
{
    if (sample > 1.0f)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return 1.0f;
    }
    else if (sample < -1.0f)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return -1.0f;
    }
    else
    {
        return sample;
    }
}

#endif

volatile uint32_t * getAudioStatePtr()
{
    return &audioState;
}