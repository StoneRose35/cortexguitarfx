#include "audio/simpleChorus.h"


/** set frequency in Hz/100 */
void simpleChorusSetFrequency(uint16_t freq,SimpleChorusType*data)
{
    data->lfoPhaseinc=freq*4*256*SIMPLE_CHORUS_LFO_DIVIDER/4800000;
    data->frequency = freq;
}


__attribute__((section (".qspi_code")))
void initSimpleChorus(SimpleChorusType*data)
{
    for(uint16_t c=0;c<SIMPLE_CHORUS_DELAY_SIZE;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
    data->lfoQuadrant=0;
    data->lfoVal=0;
    data->lfoValOld=0;
    data->lfoPhaseinc=data->frequency*4*256*SIMPLE_CHORUS_LFO_DIVIDER/4800000;
}

__attribute__ ((section (".qspi_code")))
float simpleChorusProcessSample(float sampleIn,SimpleChorusType*data)
{
    uint16_t delayPtr;
    float sampleOut;
    int16_t lfoValInterp;
    data->lfoUpdateCnt++;

    if (data->lfoUpdateCnt == SIMPLE_CHORUS_LFO_DIVIDER)
    {
        data->lfoValOld = data->lfoVal;
        if (data->lfoQuadrant == 0 )
        {
            data->lfoVal += data->lfoPhaseinc;
            if (data->lfoVal > 255)
            {
                data->lfoVal = 512 - data->lfoVal;
                data->lfoQuadrant += 1;
                data->lfoQuadrant &= 1;
            }
        }
        else
        {
            data->lfoVal -= data->lfoPhaseinc;
            if (data->lfoVal < -255)
            {
                data->lfoVal = -512 - data->lfoVal;
                data->lfoQuadrant += 1;
                data->lfoQuadrant &= 1;
            }
        }
        data->lfoUpdateCnt=0;
    }

        data->delayInputPtr &= (SIMPLE_CHORUS_DELAY_SIZE-1);
        lfoValInterp = data->lfoValOld + ((data->lfoUpdateCnt*(data->lfoVal - data->lfoValOld)) >> 8);
        // compute current index of the delay pointer
        delayPtr = (data->delayInputPtr - SIMPLE_CHORUS_MIN_DELAY - (((lfoValInterp+0xFF)*data->depth) >> 8)) & (SIMPLE_CHORUS_DELAY_SIZE-1); 
        sampleOut=sampleIn*(1.0f-data->mix) + data->mix*data->delayBuffer[delayPtr];
        *(data->delayBuffer + data->delayInputPtr++)=sampleIn;
        return sampleOut;
}
