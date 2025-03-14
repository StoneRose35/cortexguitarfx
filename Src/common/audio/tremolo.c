#include "audio/tremolo.h"
#include "globalConfig.h"
#include "audio/phaseDistortedSineSquare.h"
#include "stdlib.h"
#include "stdio.h"

int16_t tremoloProcessSample(int16_t sample,TremoloType*data)
{
    int16_t currentAmpValue;
    currentAmpValue = ((data->currentLfoVal*(PHASE_DISTORTED_SINE_SQUARE_SR_DIV-data->lfoUpdateCnt)) >> (PHASE_DISTORTED_SINE_SQUARE_SR_DIV_POW2+1))
        + ((data->nextLfoVal*data->lfoUpdateCnt)>> (PHASE_DISTORTED_SINE_SQUARE_SR_DIV_POW2+1)); 

    currentAmpValue = (1 << 15) + (((currentAmpValue*data->depth) >> 15) - (((1 << 14)*data->depth) >> 15));
    if (data->lfoUpdateCnt == (PHASE_DISTORTED_SINE_SQUARE_SR_DIV - 1))
    {
        data->currentLfoVal = data->nextLfoVal;
        data->nextLfoVal = phaseDistortedSineSquareNextSample(&data->modulator);
    }
    data->lfoUpdateCnt++;
    data->lfoUpdateCnt &= PHASE_DISTORTED_SINE_SQUARE_SR_DIV-1;
    return (sample*currentAmpValue) >> 15;
}

void initTremolo(TremoloType*data)
{
    data->depth = 0;
    data->lfoUpdateCnt = 0;
    data->modulator.currentPhase=0;
    data->modulator.pulseWidth=0;
    data->modulator.phaseIncrement = (1 << 16)*3/AUDIO_SAMPLING_RATE*PHASE_DISTORTED_SINE_SQUARE_SR_DIV; // 3 Hz for effective sampling rate
    data->modulator.phaseIncrementCorrection1 = 0;
    data->modulator.squareRatio = 0;
    data->currentLfoVal = 0;
    data->nextLfoVal=phaseDistortedSineSquareNextSample(&data->modulator);
}