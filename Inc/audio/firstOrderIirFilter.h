#ifndef _FIRST_ORDER_IIR_FILTER_H_
#define _FIRST_ORDER_IIR_FILTER_H_
#include <stdint.h>

typedef struct 
{
    float oldVal;
    float oldXVal;
    float alpha;
} FirstOrderIirType;


typedef struct 
{
    float oldVal;
    float oldXVal;
    float alphaRising;
    float alphaFalling;
} FirstOrderIirDualCoeffLPType;

typedef struct 
{
    float oldVal;
    float oldYVal;
    float alpha;
} FirstOrderIirAllpassType;


float firstOrderIirLowpassProcessSample(float sampleIn,FirstOrderIirType*data);
float firstOrderIirHighpassProcessSample(float sampleIn,FirstOrderIirType*data);
float firstOrderIirDualCoeffLPProcessSample(float sampleIn,FirstOrderIirDualCoeffLPType*data);
float firstOrderIirAllpassProcessSample(float sampleIn,FirstOrderIirAllpassType*data);
void firstOrderIirReset(FirstOrderIirType*data);
void firstOrderIirDualCoeffLPReset(FirstOrderIirDualCoeffLPType*data);

#endif