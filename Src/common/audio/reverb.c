#include "audio/reverbUtils.h"
#include "audio/reverb.h"
#include "audio/delay.h"
#include "audio/audiotools.h"

/**
 * @brief feedback interpolation values for
 * delay lines of 2011, 2551, 3163, 4093 samples
 * and tau values (time when signal is at 0.001) of 0.1, 0.73333333,1.36666667,2.        
 * 
 */
typedef struct 
{
    const char name[8];
    const uint16_t delayInSamples[4];
    const int16_t feedback[4][4];
    const int16_t taus[4];
    const uint16_t allpassDelays[4];
} reverbParameterType;


static const reverbParameterType reverbParameterSet[4]= {
    {
        .name="solid",
        .delayInSamples = {487, 683, 881, 1087, },
        .feedback = {
            {
            0x3f81, 0x7454, 0x7999, 0x7b96, 
            },{
            0x2fe6, 0x6ff0, 0x771d, 0x79db, 
            },{
            0x2405, 0x6bac, 0x74a7, 0x7822, 
            },{
            0x1ac7, 0x6768, 0x7227, 0x765d, 
            },
        },
        .taus = {100, 733,1366,2000},
        .allpassDelays = {241, 331, 401, 487,}
    },
    {
        .name="saturn",
        .delayInSamples = {809, 1201, 1601, 2003,},
        .feedback = {
            {
            0x27f4, 0x6d34, 0x758b, 0x78c2, 
            },{
            0x16ba, 0x651e, 0x70ca, 0x7566, 
            },{
            0xcc7, 0x5d7c, 0x6c23, 0x7211, 
            },{
            0x72a, 0x5665, 0x67a8, 0x6ed0, 
            },},
        .taus = {100, 733,1366,2000},
        .allpassDelays = {113, 227, 337, 449,}
    },
    {
        .name="uranus",
        .delayInSamples = {1601, 2203, 2801, 3407,},
        .feedback = {
            {
            0xcc7, 0x5d7c, 0x6c23, 0x7211, 
            },{
            0x55f, 0x5311, 0x657f, 0x6d3b, 
            },{
            0x245, 0x49df, 0x5f4d, 0x68a1, 
            },{
            0xf3, 0x4196, 0x5969, 0x642a, 
            },},
        .taus = {100, 733,1366,2000},
        .allpassDelays = {113, 173, 233, 293,}
    },
    {
        .name="neptune",
        .delayInSamples = {241, 503, 761, 1021,},
        .feedback = {
            {
            0x5a7b, 0x7a15, 0x7cc9, 0x7dcb, 
            },{
            0x3e0f, 0x73f7, 0x7964, 0x7b72, 
            },{
            0x2ad0, 0x6e3d, 0x7623, 0x792c, 
            },{
            0x1d72, 0x68c1, 0x72f2, 0x76ee, 
            },},
        .taus = {100, 733,1366,2000},
        .allpassDelays = {293, 353, 419, 487,}
    }
};

const int16_t phaseshifts[4]= {22937,22937,22937,22937};

const char * getReverbParameterSetName(ReverbType*reverbData)
{
    return reverbParameterSet[reverbData->paramNr].name;
}

int16_t getFeedback(uint8_t delayLineIndex,int16_t tau,uint8_t paramNr)
{
    int16_t feedbackVal=0;
    for(uint8_t c=0;c<4;c++)
    {
        if (reverbParameterSet[paramNr].taus[c] < tau && reverbParameterSet[paramNr].taus[c+1] > tau)
        {
            feedbackVal = reverbParameterSet[paramNr].feedback[delayLineIndex][c] + (reverbParameterSet[paramNr].feedback[delayLineIndex][c+1]-reverbParameterSet[paramNr].feedback[delayLineIndex][c])*(tau - reverbParameterSet[paramNr].taus[c])/(reverbParameterSet[paramNr].taus[c+1]-reverbParameterSet[paramNr].taus[c]);
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
        reverbData->feedbackValues[c]=getFeedback(c,reverbTime,reverbData->paramNr);
    }
}


void initReverb(ReverbType*reverbData,int16_t reverbTime)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->delayPointers[c]=delayMemoryPointer + c*4096;
    }
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->allpasses[c].delayLineIn = (int16_t*)(delayMemoryPointer+4*4096+c*1024); 
        reverbData->allpasses[c].delayLineOut = (int16_t*)(delayMemoryPointer+4*4096+c*1024+512); 
        reverbData->allpasses[c].oldValues=0;
        reverbData->allpasses[c].coefficient=phaseshifts[c];
        reverbData->allpasses[c].delayPtr=0;
        reverbData->allpasses[c].bufferSize=0x1FF;
        reverbData->allpasses[c].delayInSamples=reverbParameterSet[reverbData->paramNr].allpassDelays[c];
    }
    setReverbTime(reverbTime,reverbData);
    reverbData->delayPointer=0;
}

int16_t reverbProcessSample(int16_t sampleIn,ReverbType*reverbData)
{
    int16_t sampleOut;
    int16_t reverbSignal;
    int32_t sampleInterm;
    volatile uint32_t *  audioStatePtr = getAudioStatePtr();

    reverbSignal = 0;

    for (uint8_t rc=0;rc < 4;rc++)
    {
        sampleInterm = sampleIn + 
        ((((reverbData->delayPointers[rc][(reverbData->delayPointer-reverbParameterSet[reverbData->paramNr].delayInSamples[rc]) & 0xFFF] >> 1) + 
           (reverbData->delayPointers[rc][(reverbData->delayPointer-reverbParameterSet[reverbData->paramNr].delayInSamples[rc]-1) & 0xFFF] >> 1))
        *(reverbData->feedbackValues[0])) >> 15);
        reverbData->delayPointers[rc][reverbData->delayPointer & 0xFFF] = (int16_t)clip(sampleInterm,audioStatePtr);
    }
    reverbData->delayPointer++;
    
    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal += reverbData->delayPointers[c][(reverbData->delayPointer-reverbParameterSet[reverbData->paramNr].delayInSamples[c]) & 0xFFF] >> 2;
    }

    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal = allpassProcessSample(reverbSignal,reverbData->allpasses+c,audioStatePtr);
    }

    sampleOut = ((((1 << 15) - reverbData->mix)*sampleIn) >> 15) + ((reverbData->mix*reverbSignal) >> 15);
    return sampleOut;
}
