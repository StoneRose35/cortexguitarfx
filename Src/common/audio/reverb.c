#include "audio/reverbUtils.h"
#include "audio/reverb.h"
#include "audio/delay.h"
#include "audio/audiotools.h"

/**
 * @brief feedback interpolation values for
 * delay lines of 31, 67, 101, 113 samples
 * and tau values (time when signal is at 0.001) of 0.1, 0.73333333,1.36666667,2.        
 * 
 */
typedef struct 
{
    const char name[8];
    const uint16_t delayInSamples[4];
    const float feedback[4][4];
    const float taus[4];
    const uint16_t allpassDelays[4];
} reverbParameterType;

__attribute__((section (".qspi_data")))
static const reverbParameterType reverbParameterSet[4]= {
    {
        .name="solid",
        .delayInSamples = {487, 683, 881, 1087, },
        .feedback = {
            {
            0x3f81/32768.0f, 0x7454/32768.0f, 0x7999/32768.0f, 0x7b96/32768.0f, 
            },{
            0x2fe6/32768.0f, 0x6ff0/32768.0f, 0x771d/32768.0f, 0x79db/32768.0f, 
            },{
            0x2405/32768.0f, 0x6bac/32768.0f, 0x74a7/32768.0f, 0x7822/32768.0f, 
            },{
            0x1ac7/32768.0f, 0x6768/32768.0f, 0x7227/32768.0f, 0x765d/32768.0f, 
            },
        },
        .taus = {0.1f, 0.733333333f,1.366666666f,2.0f},
        .allpassDelays = {241, 331, 401, 487,}
    },
    {
        .name="saturn",
        .delayInSamples = {809, 1201, 1601, 2003,},
        .feedback = {
            {
            0x27f4/32768.0f, 0x6d34/32768.0f, 0x758b/32768.0f, 0x78c2/32768.0f, 
            },{
            0x16ba/32768.0f, 0x651e/32768.0f, 0x70ca/32768.0f, 0x7566/32768.0f, 
            },{
            0xcc7/32768.0f, 0x5d7c/32768.0f, 0x6c23/32768.0f, 0x7211/32768.0f, 
            },{
            0x72a/32768.0f, 0x5665/32768.0f, 0x67a8/32768.0f, 0x6ed0/32768.0f, 
            },},
        .taus = {0.1f, 0.733333333f,1.366666666f,2.0f},
        .allpassDelays = {113, 227, 337, 449,}
    },
    {
        .name="uranus",
        .delayInSamples = {1601, 2203, 2801, 3407,},
        .feedback = {
            {
            0xcc7/32768.0f, 0x5d7c/32768.0f, 0x6c23/32768.0f, 0x7211/32768.0f, 
            },{
            0x55f/32768.0f, 0x5311/32768.0f, 0x657f/32768.0f, 0x6d3b/32768.0f, 
            },{
            0x245/32768.0f, 0x49df/32768.0f, 0x5f4d/32768.0f, 0x68a1/32768.0f, 
            },{
            0xf3/32768.0f, 0x4196/32768.0f, 0x5969/32768.0f, 0x642a/32768.0f, 
            },},
        .taus = {0.1f, 0.733333333f,1.366666666f,2.0f},
        .allpassDelays = {113, 173, 233, 293,}
    },
    {
        .name="neptune",
        .delayInSamples = {241, 503, 761, 1021,},
        .feedback = {
            {
            0x5a7b/32768.0f, 0x7a15/32768.0f, 0x7cc9/32768.0f, 0x7dcb/32768.0f, 
            },{
            0x3e0f/32768.0f, 0x73f7/32768.0f, 0x7964/32768.0f, 0x7b72/32768.0f, 
            },{
            0x2ad0/32768.0f, 0x6e3d/32768.0f, 0x7623/32768.0f, 0x792c/32768.0f, 
            },{
            0x1d72/32768.0f, 0x68c1/32768.0f, 0x72f2/32768.0f, 0x76ee/32768.0f, 
            },},
        .taus = {0.1f, 0.733333333f,1.366666666f,2.0f},
        .allpassDelays = {293, 353, 419, 487,}
    }
};

__attribute__((section (".qspi_data")))
const float phaseshifts[4]= {0.7f,0.7f,0.7f,0.7f};

__attribute__((section (".qspi_code")))
const char * getReverbParameterSetName(ReverbType*reverbData)
{
    return reverbParameterSet[reverbData->paramNr].name;
}

__attribute__((section (".qspi_code")))
float getFeedback(uint8_t delayLineIndex,float tau,uint8_t paramNr)
{
    float feedbackVal=0.0f;
    for(uint8_t c=0;c<4;c++)
    {
        if (reverbParameterSet[paramNr].taus[c] < tau && reverbParameterSet[paramNr].taus[c+1] > tau)
        {
            feedbackVal = reverbParameterSet[paramNr].feedback[delayLineIndex][c] 
            + (reverbParameterSet[paramNr].feedback[delayLineIndex][c+1]-reverbParameterSet[paramNr].feedback[delayLineIndex][c])
            *(tau - reverbParameterSet[paramNr].taus[c])/(reverbParameterSet[paramNr].taus[c+1]-reverbParameterSet[paramNr].taus[c]);
        } 
    }
    return feedbackVal;
}

__attribute__((section (".qspi_code")))
void setReverbTime(float reverbTime,ReverbType*reverbData)
{
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->feedbackValues[c]=getFeedback(c,reverbTime,reverbData->paramNr);
    }
}


__attribute__((section (".qspi_code")))
void initReverb(ReverbType*reverbData,float reverbTime)
{
    float * delayMemoryPointer = getDelayMemoryPointer();
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->delayPointers[c]=delayMemoryPointer + c*4096;
    }
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->allpasses[c].delayLineIn = (float*)(delayMemoryPointer+4*4096+c*1024); 
        reverbData->allpasses[c].delayLineOut = (float*)(delayMemoryPointer+4*4096+c*1024+512); 
        reverbData->allpasses[c].oldValues=0.0f;
        reverbData->allpasses[c].coefficient=phaseshifts[c];
        reverbData->allpasses[c].delayPtr=0;
        reverbData->allpasses->bufferSize=0x3FF;
        reverbData->allpasses[c].delayInSamples=reverbParameterSet[reverbData->paramNr].allpassDelays[c];
    }
    setReverbTime(reverbTime,reverbData);
    reverbData->delayPointer=0;
}


__attribute__ ((section (".qspi_code")))
float reverbProcessSample(float sampleIn,ReverbType*reverbData)
{
    float sampleOut;
    float reverbSignal;
    float sampleInterm;
    volatile uint32_t *  audioStatePtr = getAudioStatePtr();


    reverbSignal = 0.0f;

    
    for (uint8_t rc=0;rc<4;rc++)
    {
        sampleInterm = sampleIn + ((reverbData->delayPointers[rc][((reverbData->delayPointer - reverbParameterSet[reverbData->paramNr].delayInSamples[rc]) & 0xFFF)]+
        reverbData->delayPointers[rc][(reverbData->delayPointer - reverbParameterSet[reverbData->paramNr].delayInSamples[rc]-1) & 0xFFF] )*0.5f)*reverbData->feedbackValues[rc];
        reverbData->delayPointers[rc][reverbData->delayPointer & 0xFFF]=sampleInterm;
    }
    reverbData->delayPointer++;

    for (uint8_t c=0;c < 4;c++)
    {
        reverbSignal += reverbData->delayPointers[c][(reverbData->delayPointer -reverbParameterSet[reverbData->paramNr].delayInSamples[c]) & 0xFFF]*0.25f;
    }

    

    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal = allpassProcessSample(reverbSignal,reverbData->allpasses+c,audioStatePtr);
    }

    sampleOut = (1.0f-reverbData->mix)*sampleIn + reverbData->mix*reverbSignal;
    return sampleOut;
}
