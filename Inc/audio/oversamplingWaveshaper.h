#ifndef _OVERSAMPLING_WAVESHAPER_H_
#define _OVERSAMPLING_WAVESHAPER_H_
#include <stdint.h>
#include "audio/waveShaper.h"
#include "audio/secondOrderIirFilter.h"


typedef struct {
    float oldValue;
    WaveShaperDataType waveshaper;
    SecondOrderIirFilterType oversamplingFilter;

} OversamplingWaveshaperDataType;

void initOversamplingWaveshaper(OversamplingWaveshaperDataType*data);
void  applyOversamplingDistortion(float*data,OversamplingWaveshaperDataType*waveshaper);
float  OversamplingDistortionProcessSample(float sample,OversamplingWaveshaperDataType* waveshaper);
void oversamplingWaveshaperReset(OversamplingWaveshaperDataType*data);
#endif