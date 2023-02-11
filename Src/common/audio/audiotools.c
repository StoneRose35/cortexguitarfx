#include "stdint.h"
#include "audio/audiotools.h"
#ifndef RP2040_FEATHER

int32_t clip(int32_t sample)
{
    if (sample > 32767)
    {
        return 32767;
    }
    else if (sample < -32768)
    {
        return -32768;
    }
    else
    {
        return sample;
    }
}
#endif