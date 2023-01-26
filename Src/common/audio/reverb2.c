#include "audio/reverb2.h"
#include "audio/delay.h"

const uint16_t ap_delays[]={433,439,443,449,457,461,463,467};

void initReverb2(Reverb2Type*reverbData)
{
    DelayDataType * delaySingleton = getDelayData();
    initDelay(delaySingleton);
    for (uint8_t c=0;c<8;c++)
    {
        reverbData->aps[c].delayLine = delaySingleton->delayLine + c*4096;
        reverbData->aps[c].coefficient = 23116;
        reverbData->aps[c].delayPtr=0;
        reverbData->aps[c].oldValues=0;
        reverbData->aps[c].delayInSamples=ap_delays[c];
        reverbData->outs[c>>1]=0;
    }
    reverbData->mix=0;
}

int16_t reverb2ProcessSample(int16_t sampleIn,Reverb2Type*reverbData)
{
    int16_t sampleOut;
    int16_t reverbSignal=0;
    int16_t processSignal = sampleIn>>1 + ((reverbData->decay*reverbData->outs[3])>> 16);
    processSignal = allpassProcessSample(processSignal,&reverbData->aps[0]);
    reverbData->outs[0] = allpassProcessSample(processSignal,&reverbData->aps[1]);
    reverbSignal += reverbData->outs[0] >> 2;

    processSignal = sampleIn>>1 + ((reverbData->decay*reverbData->outs[0])>> 16);
    processSignal = allpassProcessSample(processSignal,&reverbData->aps[2]);
    reverbData->outs[1] = allpassProcessSample(processSignal,&reverbData->aps[3]);
    reverbSignal += reverbData->outs[1] >> 2;

    processSignal = sampleIn>>1 + ((reverbData->decay*reverbData->outs[1])>> 16);
    processSignal = allpassProcessSample(processSignal,&reverbData->aps[4]);
    reverbData->outs[2] = allpassProcessSample(processSignal,&reverbData->aps[5]);
    reverbSignal += reverbData->outs[2] >> 2;

    processSignal = sampleIn >> 1 + ((reverbData->decay*reverbData->outs[2])>> 16);
    processSignal = allpassProcessSample(processSignal,&reverbData->aps[6]);
    reverbData->outs[3] = allpassProcessSample(processSignal,&reverbData->aps[7]);
    reverbSignal += reverbData->outs[3] >> 2;  

    sampleOut = ((((1 << 15) - reverbData->mix)*sampleIn) >> 15) + ((reverbData->mix*reverbSignal) >> 15);
    return sampleOut;  
}