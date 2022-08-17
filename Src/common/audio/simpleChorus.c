
#include "audio/simpleChorus.h"

//static int16_t delayBuffer[2048];

//static volatile SimpleChorusType simpleChorusData;
//static volatile uint16_t lfoUpdateCnt=0;

void initSimpleChorus(SimpleChorusType*data)
{
    for(uint16_t c=0;c<2048;c++)
    {
        data->delayBuffer[c]=0;
    }
    data->lfoQuadrant=0;
    data->lfoVal=0;
    data->lfoValOld=0;
    data->lfoPhaseinc=data->frequency*4*256*SIMPLE_CHORUS_LFO_DIVIDER/4800000;
}

/** set frequency in Hz/100 */
void simpleChorusSetFrequency(uint16_t freq,SimpleChorusType*data)
{
    data->lfoPhaseinc=freq*4*256*SIMPLE_CHORUS_LFO_DIVIDER/4800000;
}


int16_t simpleChorusProcessSample(int16_t sampleIn,SimpleChorusType*data)
{
    int16_t delayPtr;
    int16_t sampleOut;
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
        delayPtr = (data->delayInputPtr-1) - 4 - (((lfoValInterp+0xFF)*data->depth) >> 8);
        if (delayPtr < 0)
        {
            delayPtr = SIMPLE_CHORUS_DELAY_SIZE + delayPtr;
        }  
        sampleOut = ((sampleIn*(0xFF - data->mix)) >> 8) + (((data->mix*data->delayBuffer[delayPtr])>>8));
        *(data->delayBuffer + data->delayInputPtr++)=sampleIn;
        return sampleOut;
}
