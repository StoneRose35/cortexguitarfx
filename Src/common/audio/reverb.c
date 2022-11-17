#include "audio/reverb.h"

/**
 * @brief feedback interpolation values for
 * delay lines of 31, 67, 101, 113 samples
 * and tau values (time when signal is at 0.001) of 0.1, 0.73333333,1.36666667,2.        
 * 
 */
const float feedback[4][4] = {
{
0.9563679f, 0.9939349f, 0.9967410f, 0.9977719f, 
},{
0.9080819f, 0.9869378f, 0.9929697f, 0.9951906f, 
},{
0.8647190f, 0.9803746f, 0.9894209f, 0.9927588f, 
},{
0.8499140f, 0.9780686f, 0.9881715f, 0.9919020f, 
}};

const float taus[4] = {0.1, 0.733333333f,1.366666666f,2.0f}; 
const uint8_t delayInSamples[4] = {31,67,101,113};
const float phaseshifts[4]= {0.01f,0.1f,0.47f,0.98f};

float getFeedback(uint8_t delayLineIndex,float tau)
{
    float feedbackVal=0.0f;
    for(uint8_t c=0;c<3;c++)
    {
        if (taus[c] < tau && taus[c+1] > tau)
        {
            feedbackVal = feedback[delayLineIndex][c] + (feedback[delayLineIndex][c+1]-feedback[delayLineIndex][c])*(tau - taus[c])/(taus[c+1]-taus[c]);
        } 
    }
    return feedbackVal;
}

void setReverbTime(float reverbTime,ReverbType*reverbData)
{
    for(uint8_t c=0;c<4;c++)
    {
        reverbData->feedbackValues[c]=getFeedback(c,reverbTime);
    }
}

float allpassProcessSample(float sampleIn,AllpassType*allpass)
{
    float sampleOut;
    sampleOut = allpass->coefficient*sampleIn + allpass->oldValues;
    allpass->oldValues = sampleIn - allpass->coefficient*sampleOut;
    return sampleOut;
}

void initReverb(ReverbType*reverbData)
{
    for(uint8_t c=0;c<4;c++)
    {
        for(uint8_t cc=0;cc<128;cc++)
        {
            reverbData->delayLines[c][cc]=0.0;
        }
        reverbData->delayPointers[c]=0;
        reverbData->delayTime=0.3;
        setReverbTime(reverbData->delayTime,reverbData);
        reverbData->allpasses[c].oldValues=0.0f;
        reverbData->allpasses[c].coefficient=phaseshifts[c];
    }
}

float reverbProcessSample(float sampleIn,ReverbType*reverbData)
{
    float sampleOut;
    float reverbSignal;
    for (uint8_t rc=0;rc < 4;rc++)
    {
        reverbData->delayLines[rc][reverbData->delayPointers[rc] & 0x7F] = sampleIn + 
            reverbData->delayLines[rc][(reverbData->delayPointers[rc]-delayInSamples[rc]) & 0x7F]*(reverbData->feedbackValues[rc]);
        reverbData->delayPointers[rc]++;
    }

    reverbSignal = 0.0f;

    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal += reverbData->delayLines[c][(reverbData->delayPointers[0]-delayInSamples[0]) & 0x7F];
    }

    for (uint8_t c=0;c<4;c++)
    {
        reverbSignal = allpassProcessSample(reverbSignal,reverbData->allpasses+c);
    }

    sampleOut = (1.0f-reverbData->mix)*sampleIn + reverbData->mix*reverbSignal;
    return sampleOut;
}
