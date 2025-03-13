#ifndef _PHASE_DISTORTED_SINE_SQUARE_H_
#define _PHASE_DISTORTED_SINE_SQUARE_H_
#include "stdint.h"

typedef struct 
{
    uint8_t squareRatio;
    uint32_t phaseIncrement;
    int32_t phaseIncrementCorrection1,phaseIncrementCorrection2;
    uint32_t currentPhase;
    int16_t pulseWidth;
} PhaseDistortedSineSquareType;



int16_t phaseDistortedSineSquareNextSample(PhaseDistortedSineSquareType*data);
void phaseDistortedSineSquarePulseWidth(int16_t value,PhaseDistortedSineSquareType*data);
void phaseDistortedSineSquareSetFrequency(float f,PhaseDistortedSineSquareType*data);
float phaseDistortedSineSquareGetFrequency(PhaseDistortedSineSquareType*data);
#endif