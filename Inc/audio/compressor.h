#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_
#include "firstOrderIirFilter.h"

typedef struct 
{
    int16_t threshhold;
    uint8_t gainReduction; // power of two 1 1:2,2 1:4, 3 1:8, 4 1:16 ,5 1: inf
} GainFunctionType;


typedef struct 
{
    GainFunctionType gainFunction;
    FirstOrderIirDualCoeffLPType avgLowpass;
    int16_t currentAvg;
} CompressorDataType;


void setAttack(int32_t attackInUs,CompressorDataType*data);
void setRelease(int32_t releaseInUs,CompressorDataType*data);
int16_t compressorProcessSample(int16_t sampleIn,CompressorDataType*data);
int16_t applyGain(int16_t sample,int16_t avgVolume,CompressorDataType*gainFunction);
#endif