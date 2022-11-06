#ifndef _WAVESHAPER_H_
#define _WAVESHAPER_H_
#include <stdint.h>

#ifndef FLOAT_AUDIO
typedef struct 
{
    int16_t transferFunctionPoints[128];

} WaveShaperDataType;

extern const WaveShaperDataType waveShaperUnity;
extern const WaveShaperDataType waveShaperDefaultOverdrive;
extern const WaveShaperDataType waveShaperSoftOverdrive;
extern const WaveShaperDataType waveShaperDistortion;
extern const WaveShaperDataType waveShaperCurvedOverdrive;
extern const WaveShaperDataType waveShaperAsymm; 




int16_t waveShaperProcessSample(int16_t sampleIn,WaveShaperDataType*data);
#else

typedef struct 
{
    float transferFunctionPoints[128];

} WaveShaperDataType;

extern const WaveShaperDataType waveShaperUnity;
extern const WaveShaperDataType waveShaperDefaultOverdrive;
extern const WaveShaperDataType waveShaperSoftOverdrive;
extern const WaveShaperDataType waveShaperDistortion;
extern const WaveShaperDataType waveShaperCurvedOverdrive;
extern const WaveShaperDataType waveShaperAsymm; 




float waveShaperProcessSample(float sampleIn,WaveShaperDataType*data);

#endif

void initWaveShaper(WaveShaperDataType * data,const WaveShaperDataType* copyFrom);
#endif