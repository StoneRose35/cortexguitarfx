#include "audio/reverb.h"
#include "audio/delay.h"
#include "audio/audiotools.h"

/**
 * @brief feedback interpolation values for
 * delay lines of 2011, 2551, 3163, 4093 samples
 * and tau values (time when signal is at 0.001) of 0.1, 0.73333333,1.36666667,2.        
 * 
 */

const int16_t feedback[4][4] = {
{
0xdc6, 0x5e71, 0x6cbb, 0x727e, 
},{
0xdb1, 0x5e5e, 0x6caf, 0x7276, 
},{
0xd93, 0x5e42, 0x6c9e, 0x7269, 
},{
0xd6c, 0x5e1c, 0x6c86, 0x7259, 
},};

const int16_t taus[4] = {100, 733,1366,2000}; 
const uint16_t delayInSamples[4] = {1549, 1553, 1559, 1567};
const uint16_t allpassDelays[3] = {307,311,313};
const int16_t phaseshifts[4]= {22937,22937,22937,22937};

int16_t getFeedback(uint8_t delayLineIndex,int16_t tau)
{
    int16_t feedbackVal=0;
    for(uint8_t c=0;c<3;c++)
    {
        if (taus[c] < tau && taus[c+1] > tau)
        {
            feedbackVal = feedback[delayLineIndex][c] + (feedback[delayLineIndex][c+1]-feedback[delayLineIndex][c])*(tau - taus[c])/(taus[c+1]-taus[c]);
        } 
    }
    return feedbackVal;
}

/**
 * @brief Set the Reverb Time in milliseconds, valid values are from 100 to 2000
 * 
 * @param reverbTime 
 * @param reverbData 
 */
void setReverbTime(int16_t reverbTime,ReverbType*reverbData)
{
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->feedbackValues[c]=getFeedback(c,reverbTime);
    }
}

int16_t  allpassProcessSample(int16_t sampleIn,AllpassType*allpass)
{
    int16_t sampleOut;
    int32_t sampleInterm;
    sampleInterm = sampleIn - (((*(allpass->delayLine + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize)))*allpass->coefficient) >> 15);
    *(allpass->delayLine + allpass->delayPtr) = (int16_t)clip(sampleInterm);
    // allpass->oldValues
    sampleInterm = ((allpass->coefficient*sampleInterm) >> 15) + *(allpass->delayLine + ((allpass->delayPtr - allpass->delayInSamples) & allpass->bufferSize));
    sampleInterm=clip(sampleInterm);
    sampleOut = (int16_t)sampleInterm;
    allpass->delayPtr++;
    allpass->delayPtr &= allpass->bufferSize;
    return sampleOut;
}

void initReverb(ReverbType*reverbData,int16_t reverbTime)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->delayPointers[c]=delayMemoryPointer + c*4096;
    }
    for(uint8_t c=0;c<3;c++)
    {
        reverbData->allpasses[c].delayLine = (int16_t*)(delayMemoryPointer+4*4096+c*1024); 
        reverbData->allpasses[c].oldValues=0;
        reverbData->allpasses[c].coefficient=phaseshifts[c];
        reverbData->allpasses[c].delayPtr=0;
        reverbData->allpasses[c].bufferSize=0x3FF;
        reverbData->allpasses[c].delayInSamples=allpassDelays[c];
    }
    setReverbTime(reverbTime,reverbData);
    reverbData->delayPointer=0;
}

int16_t reverbProcessSample(int16_t sampleIn,ReverbType*reverbData)
{
    int16_t sampleOut;
    int16_t reverbSignal;
    int32_t sampleInterm;

    reverbSignal = 0;

    
    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal += reverbData->delayPointers[c][(reverbData->delayPointer-delayInSamples[c]) & 0xFFF] >> 2;
    }

    for (uint8_t rc=0;rc < 4;rc++)
    {
        sampleInterm = sampleIn + 
        ((reverbData->delayPointers[rc][(reverbData->delayPointer-delayInSamples[rc]) & 0xFFF]*(reverbData->feedbackValues[rc])) >> 15);
        reverbData->delayPointers[rc][reverbData->delayPointer & 0xFFF] = (int16_t)clip(sampleInterm);
    }
    reverbData->delayPointer++;
    

    for (uint8_t c=0;c<3;c++)
    {
        reverbSignal = allpassProcessSample(reverbSignal,reverbData->allpasses+c);
    }

    sampleOut = ((((1 << 15) - reverbData->mix)*sampleIn) >> 15) + ((reverbData->mix*reverbSignal) >> 15);
    return sampleOut;
}
