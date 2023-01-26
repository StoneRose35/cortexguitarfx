#ifndef _REVERB_2_H_
#define _REVERB_2_H_
#include "stdint.h"
#include "reverb.h"

typedef struct 
{
    AllpassType aps[8];
    int16_t mix;
    int16_t decay;
    int16_t outs[4];
} Reverb2Type;

#endif