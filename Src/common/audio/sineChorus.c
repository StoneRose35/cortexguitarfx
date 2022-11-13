
#include "audio/sineChorus.h"

static const int16_t firstSineQuadrant[33] = {0,12,25,38,51,63,76,88,100,112,123,134,145,156,166,175,184,193,201,209,216,222,228,234,239,243,246,249,252,253,254,255,255};

/** set frequency in Hz/100 */
void sineChorusSetFrequency(uint16_t freq,SineChorusType*data)
{
    data->lfoPhaseinc=freq*65536*SINE_CHORUS_LFO_DIVIDER/4800000;
    data->frequency = freq;
}

int16_t getSineValue(uint16_t phase)
{
    uint8_t index;
    uint8_t quadrant;
    int16_t val;
    index=(phase >> 9) & 0x1f;
    quadrant = phase >>14;
    switch (quadrant)
    {
        case 0:
            val = firstSineQuadrant[index] + (((firstSineQuadrant[index+1]-firstSineQuadrant[index])*(phase & 0x1FF)) >> 9);
            break;
        case 1:
            index = 32 - index;
            val = firstSineQuadrant[index] + (((firstSineQuadrant[index-1]-firstSineQuadrant[index])*(phase & 0x1FF)) >> 9);
            break;
        case 2:
            val = -firstSineQuadrant[index] + (((-firstSineQuadrant[index+1]+firstSineQuadrant[index])*(phase & 0x1FF)) >> 9);
            break;
        default:
            index = 32 - index;
            val = -firstSineQuadrant[index] + (((-firstSineQuadrant[index-1]+firstSineQuadrant[index])*(phase & 0x1FF)) >> 9);
            break;
    }
    return val;
}

#ifndef FLOAT_AUDIO
#else

void initSineChorus(SineChorusType*data)
{
    for(uint16_t c=0;c<SINE_CHORUS_DELAY_SIZE;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
    data->frequency=100;
    data->lfoVal=0;
    data->lfoValOld=0;
    data->lfoPhaseinc=data->frequency*4*256*SINE_CHORUS_LFO_DIVIDER/4800000;
}

float sineChorusProcessSample(float sampleIn,SineChorusType*data)
{
    uint16_t delayPtr;
    float sampleOut, mixf;
    int16_t lfoValInterp;
    data->lfoUpdateCnt++;

    if (data->lfoUpdateCnt == SINE_CHORUS_LFO_DIVIDER)
    {
        data->lfoValOld = data->lfoVal;
        data->lfoVal = getSineValue(data->lfoPhase+=data->lfoPhaseinc);
        data->lfoUpdateCnt=0;
    }

        data->delayInputPtr &= (SINE_CHORUS_DELAY_SIZE-1);
        lfoValInterp = data->lfoValOld + ((data->lfoUpdateCnt*(data->lfoVal - data->lfoValOld)) >> 8);
        // compute current index of the delay pointer
        delayPtr = (data->delayInputPtr - 5 - (((lfoValInterp+0xFF)*data->depth) >> 8)) & (SINE_CHORUS_DELAY_SIZE-1); 
        mixf=((float)data->mix)/255.0f;
        sampleOut=sampleIn*(1.0f-mixf) + mixf*data->delayBuffer[delayPtr];
        *(data->delayBuffer + data->delayInputPtr++)=sampleIn;
        return sampleOut;
}



#endif