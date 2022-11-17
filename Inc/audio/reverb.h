#ifndef _REVERB_H_
#define _REVERB_H_
#include "stdint.h"

typedef struct 
{
    float coefficient;
    float oldValues;
} AllpassType;

typedef struct 
{
    AllpassType allpasses[4];
    float delayLines[4][128]; 
    uint8_t delayPointers[4];
    float feedbackValues[4];
    float delayTime;
    float mix;

} ReverbType;




#endif