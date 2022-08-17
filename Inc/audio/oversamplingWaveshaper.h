#ifndef _OVERSAMPLING_WAVESHAPER_H_
#define _OVERSAMPLING_WAVESHAPER_H_
#include <stdint.h>
#include "audio/waveShaper.h"
#include "audio/secondOrderIirFilter.h"


typedef struct {
    int16_t oldValue;
    WaveShaperDataType waveshaper;
    SecondOrderIirFilterType oversamplingFilter;

} OversamplingWaveshaperDataType;

void initOversamplingWaveshaper(OversamplingWaveshaperDataType*data);
void  applyOversamplingDistortion(uint16_t*data,OversamplingWaveshaperDataType*waveshaper);
int16_t  OversamplingDistortionProcessSample(int16_t sample,OversamplingWaveshaperDataType* waveshaper);
#endif