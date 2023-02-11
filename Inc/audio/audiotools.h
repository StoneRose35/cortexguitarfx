#ifndef _AUDIOTOOLS_H_
#define _AUDIOTOOLS_H_

#include "stdint.h"
#ifdef RP2040_FEATHER
inline int32_t clip(int32_t sample)
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
#else
int32_t clip(int32_t sample);
#endif

#endif