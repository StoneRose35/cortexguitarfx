#ifndef _PHASE_DISTORTED_SINE_SQUARE_H_
#define _PHASE_DISTORTED_SINE_SQUARE_H_
#include "stdint.h"
#include "globalConfig.h"

#define PHASE_DISTORTED_SINE_SQUARE_SR_DIV_POW2 5
#define PHASE_DISTORTED_SINE_SQUARE_SR (AUDIO_SAMPLING_RATE >> PHASE_DISTORTED_SINE_SQUARE_SR_DIV_POW2)
#define PHASE_DISTORTED_SINE_SQUARE_SR_DIV (1 << PHASE_DISTORTED_SINE_SQUARE_SR_DIV_POW2)
typedef struct 
{
    uint8_t squareRatio;
    int32_t phaseIncrement;
    int32_t phaseIncrementCorrection1,phaseIncrementCorrection2;
    uint32_t currentPhase;
    int16_t pulseWidth;
} PhaseDistortedSineSquareType;



__attribute__ ((section (".ramfunc"))) int16_t phaseDistortedSineSquareNextSample(PhaseDistortedSineSquareType*data);
void phaseDistortedSineSquarePulseWidth(int16_t value,PhaseDistortedSineSquareType*data);
void phaseDistortedSineSquareSetFrequency(float f,PhaseDistortedSineSquareType*data);
float phaseDistortedSineSquareGetFrequency(PhaseDistortedSineSquareType*data);
#endif