#include "stdint.h"
#include "audio/audiotools.h"

static volatile uint32_t audioState=0;


volatile uint32_t * getAudioStatePtr()
{
    return &audioState;
}