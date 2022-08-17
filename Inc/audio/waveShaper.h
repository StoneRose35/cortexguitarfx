#ifndef _WAVESHAPER_H_
#define _WAVESHAPER_H_
#include <stdint.h>

typedef struct 
{
    int16_t transferFunctionPoints[128];

} WaveShaperDataType;

const WaveShaperDataType waveShaperUnity;
const WaveShaperDataType waveShaperDefaultOverdrive;
const WaveShaperDataType waveShaperSoftOverdrive;
const WaveShaperDataType waveShaperDistortion;
const WaveShaperDataType waveShaperCurvedOverdrive;
const WaveShaperDataType waveShaperAsymm; 


void initWaveShaper(WaveShaperDataType * data,const WaveShaperDataType* copyFrom);

int16_t waveShaperProcessSample(int16_t sampleIn,WaveShaperDataType*data);
#endif