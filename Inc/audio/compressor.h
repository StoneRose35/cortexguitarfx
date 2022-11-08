#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_

#ifndef FLOAT_AUDIO
typedef struct 
{
    int16_t threshhold;
    uint8_t gainReduction; // power of two 1 1:2,2 1:4, 3 1:8, 4 1:16 ,5 1: inf
} GainFunctionType;


typedef struct 
{
    int16_t attack; // maximum change in 20.83333 us, attack time is 32768/attack*20.83us
    int16_t release;
    int16_t currentAvg;
    GainFunctionType gainFunction;
} CompressorDataType;


void setAttack(int32_t attackInUs,CompressorDataType*data);
void setRelease(int32_t releaseInUs,CompressorDataType*data);
int16_t compressorProcessSample(int16_t sampleIn,CompressorDataType*data);
int16_t applyGain(int16_t sample,int16_t avgVolume,CompressorDataType*gainFunction);
#else
typedef struct 
{
    float threshhold;
    float gainReduction; // division factor, compression ratio is 1:gainReduction, to be compliant with the integer implementation
                           // this should only be 2,4,8,16,Inf 
} GainFunctionType;


typedef struct 
{
    float attack; // maximum change in 20.83333 us, attack time is 32768/attack*20.83us
    float release;
    float currentAvg;
    GainFunctionType gainFunction;
} CompressorDataType;


void setAttack(int32_t attackInUs,CompressorDataType*data);
void setRelease(int32_t releaseInUs,CompressorDataType*data);
float compressorProcessSample(float sampleIn,CompressorDataType*data);
float applyGain(float sample,float avgVolume,CompressorDataType*gainFunction);
#endif

#endif