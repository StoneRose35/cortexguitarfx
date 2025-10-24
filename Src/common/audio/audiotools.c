#include "stdint.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"

static volatile uint32_t audioState=0;


volatile uint32_t * getAudioStatePtr()
{
    return &audioState;
}