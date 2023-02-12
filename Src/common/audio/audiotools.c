#include "stdint.h"
#include "audio/audiotools.h"

static volatile uint32_t audioState=0;
#ifndef RP2040_FEATHER

int32_t clip(int32_t sample,volatile uint32_t*audioStatePtr)
{
    if (sample > 32767)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return 32767;
    }
    else if (sample < -32768)
    {
        *audioStatePtr |= (1 << AUDIO_STATE_OUTPUT_CLIPPED);
        return -32768;
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