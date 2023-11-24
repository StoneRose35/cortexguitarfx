#include "audio/gainstage.h"
#include "audio/audiotools.h"

void initGainstage(GainStageDataType*data)
{
    data->gain=256;
    data->offset=0;
}

int16_t gainStageProcessSample(int16_t sampleIn,GainStageDataType*data)
{
    int16_t sampleOut;
    int32_t sampleWord = (int32_t)sampleIn;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    sampleWord = sampleWord* data->gain;
    sampleWord >>= 8;
    sampleWord = sampleWord + data->offset;

    sampleOut = (int16_t)clip(sampleWord,audioStatePtr);
    return sampleOut;
}