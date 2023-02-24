#include "stdint.h"
#include "audio/reverbUtils.h"
#include "audio/audiotools.h"

int16_t  allpassProcessSample(int16_t sampleIn,AllpassType*allpass,volatile uint32_t * audioStatePtr)
{
    int16_t sampleOut;
    int32_t sampleInterm;
    //sampleInterm = sampleIn - (((*(allpass->delayLine + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize)))*allpass->coefficient) >> 15);
    sampleInterm =  ((allpass->coefficient*sampleIn) >> 15) + *(allpass->delayLineIn + 
                    ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize)) -
                    ((*(allpass->delayLineOut + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize))*allpass->coefficient) >> 15);  
    //*(allpass->delayLine + allpass->delayPtr) = (int16_t)clip(sampleInterm);
    // allpass->oldValues
    //sampleInterm = ((allpass->coefficient*sampleInterm) >> 15) + *(allpass->delayLine + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize));
    sampleInterm=clip(sampleInterm,audioStatePtr);
    sampleOut = (int16_t)sampleInterm;
    *(allpass->delayLineIn + allpass->delayPtr) = sampleIn;
    *(allpass->delayLineOut + allpass->delayPtr) = sampleOut;
    allpass->delayPtr++;
    allpass->delayPtr &= allpass->bufferSize;
    return sampleOut;
}


void hadamardDiffuserProcessArray(int16_t * channels,HadamardDiffuserType*data)
{
    int32_t sum_first, sum_second;
    int32_t diff_first, diff_second;
    for(uint8_t c=0;c<4;c++)
    {
        data->delayPointers[c][data->delayPointer] = channels[c];
    }

    sum_first = (data->delayPointers[0][(data->delayPointer - data->delayTimes[0]) & 0x1FF] ) +
                (data->delayPointers[1][(data->delayPointer - data->delayTimes[1]) & 0x1FF] ); 
    sum_second = (data->delayPointers[2][(data->delayPointer - data->delayTimes[2]) & 0x1FF] ) +
                (data->delayPointers[3][(data->delayPointer - data->delayTimes[3]) & 0x1FF] );   
    diff_first = (data->delayPointers[0][(data->delayPointer - data->delayTimes[0]) & 0x1FF] ) -
                (data->delayPointers[1][(data->delayPointer - data->delayTimes[1]) & 0x1FF] );  
    diff_second = (data->delayPointers[2][(data->delayPointer - data->delayTimes[2]) & 0x1FF] ) -
                (data->delayPointers[3][(data->delayPointer - data->delayTimes[3]) & 0x1FF] );                       
    channels[0] = (sum_first + sum_second) >> 2;
    channels[1] = (diff_first + diff_second) >> 2;
    channels[2] = (sum_first - sum_second) >> 2;
    channels[3] = (diff_first + diff_second) >> 2;

    data->delayPointer++;
    data->delayPointer &= 0x1FF;
}
