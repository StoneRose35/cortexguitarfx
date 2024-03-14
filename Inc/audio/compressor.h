#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_
#include "firstOrderIirFilter.h"

typedef struct 
{
    float threshhold;
    float gainReduction; // power of two 1 1:2,2 1:4, 3 1:8, 4 1:16 ,5 1: inf
} GainFunctionType;


typedef struct 
{
    GainFunctionType gainFunction;
    FirstOrderIirDualCoeffLPType avgLowpass;
    float currentAvg;
} CompressorDataType;

float getMaxGain(CompressorDataType*comp);
float compressorProcessSample(float sampleIn,CompressorDataType*data);
float compressor2ProcessSample(float sampleIn,CompressorDataType*data);
float compressor3ProcessSample(float sampleIn,CompressorDataType*data);
float applyGain(float sample,float avgVolume,CompressorDataType*gainFunction);

float applyGain2(float sample,float avgVolume,CompressorDataType*gainFunction);
void compressorReset(CompressorDataType*data);
#endif