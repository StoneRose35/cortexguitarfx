#include "stdint.h"
#include "audio/reverbUtils.h"
#include "audio/audiotools.h"

__attribute__((section (".qspi_code")))
float  allpassProcessSample(float sampleIn,AllpassType*allpass,volatile uint32_t*audioStatePtr)
{
    float sampleInterm;
    sampleInterm= allpass->coefficient*sampleIn + *(allpass->delayLineIn + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize))
    -
    *(allpass->delayLineOut + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize))*allpass->coefficient;
    //sampleInterm = clip(sampleInterm,audioStatePtr);
    *(allpass->delayLineIn + allpass->delayPtr)=sampleIn;
    *(allpass->delayLineOut + allpass->delayPtr)=sampleInterm;
    allpass->delayPtr++;
    allpass->delayPtr &= allpass->bufferSize;
    return sampleInterm; 

}

__attribute__((section (".qspi_code")))
void hadamardDiffuserProcessArray(float * channels,HadamardDiffuserType*data,volatile uint32_t * audioStatePtr)
{
    float sum_first, sum_second;
    float diff_first, diff_second;
        for(uint8_t c=0;c<4;c++)
    {
        data->delayPointers[c][data->delayPointer] = clip(channels[c],audioStatePtr);
    }

    sum_first = (data->delayPointers[0][(data->delayPointer - data->delayTimes[0]) & (data->diffusorSize -1)] ) +
                (data->delayPointers[1][(data->delayPointer - data->delayTimes[1]) & (data->diffusorSize -1)] ); 
    sum_second = (data->delayPointers[2][(data->delayPointer - data->delayTimes[2]) & (data->diffusorSize -1)] ) +
                (data->delayPointers[3][(data->delayPointer - data->delayTimes[3]) & (data->diffusorSize -1)] );   
    diff_first = (data->delayPointers[0][(data->delayPointer - data->delayTimes[0]) & (data->diffusorSize -1)] ) -
                (data->delayPointers[1][(data->delayPointer - data->delayTimes[1]) & (data->diffusorSize -1)] );  
    diff_second = (data->delayPointers[2][(data->delayPointer - data->delayTimes[2]) & (data->diffusorSize -1)] ) -
                (data->delayPointers[3][(data->delayPointer - data->delayTimes[3]) & (data->diffusorSize -1)] );                       
    channels[0] = (sum_first + sum_second)*0.5f;
    channels[1] = (diff_first + diff_second)*0.5f;
    channels[2] = (sum_first - sum_second)*0.5f;
    channels[3] = (diff_first + diff_second)*0.5f;

    data->delayPointer++;
    data->delayPointer &= (data->diffusorSize -1);
}