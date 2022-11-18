#ifndef _WAVESHAPER_H_
#define _WAVESHAPER_H_
#include <stdint.h>

#define WAVESHAPER_CURVE_LENGTH 256
#ifndef FLOAT_AUDIO
typedef struct 
{
    int16_t * transferFunctionPoints;

} WaveShaperDataType;


typedef struct 
{
    int16_t transferFunctionPoints[128];

} WaveShaperDataTypeRO;

typedef struct 
{
    int16_t * transferFunctions[64];
    uint8_t functionIndex;
} MultiWaveShaperDataType;

typedef struct 
{
    int16_t transferFunctions[64][WAVESHAPER_CURVE_LENGTH];
} MultiWaveShaperDataTypeRO;

extern const WaveShaperDataTypeRO waveShaperUnity;
extern const WaveShaperDataTypeRO waveShaperDefaultOverdrive;
extern const WaveShaperDataTypeRO waveShaperSoftOverdrive;
extern const WaveShaperDataTypeRO waveShaperDistortion;
extern const WaveShaperDataTypeRO waveShaperCurvedOverdrive;
extern const WaveShaperDataTypeRO waveShaperAsymm; 
extern const MultiWaveShaperDataTypeRO multiWaveshaper1;


int16_t multiWaveShaperProcessSample(int16_t sampleIn,MultiWaveShaperDataType*data);
int16_t waveShaperProcessSample(int16_t sampleIn,WaveShaperDataType*data);
#else

typedef struct 
{
    float * transferFunctionPoints;

} WaveShaperDataType;

typedef struct 
{
    float transferFunctionPoints[128];

} WaveShaperDataTypeRO;

typedef struct 
{
    float * transferFunctions[64];
    uint8_t functionIndex;
} MultiWaveShaperDataType;

typedef struct 
{
    float transferFunctions[64][WAVESHAPER_CURVE_LENGTH];
} MultiWaveShaperDataTypeRO;


extern const WaveShaperDataTypeRO waveShaperUnity;
extern const WaveShaperDataTypeRO waveShaperDefaultOverdrive;
extern const WaveShaperDataTypeRO waveShaperSoftOverdrive;
extern const WaveShaperDataTypeRO waveShaperDistortion;
extern const WaveShaperDataTypeRO waveShaperCurvedOverdrive;
extern const WaveShaperDataTypeRO waveShaperAsymm; 
extern const MultiWaveShaperDataTypeRO multiWaveshaper1;



float multiWaveShaperProcessSample(float sampleIn,MultiWaveShaperDataType*data);
float waveShaperProcessSample(float sampleIn,WaveShaperDataType*data);


#endif

void initMultiWaveShaper(MultiWaveShaperDataType * data,const MultiWaveShaperDataTypeRO *copyFrom);
void initWaveShaper(WaveShaperDataType * data,const WaveShaperDataTypeRO* copyFrom);

#endif