#ifndef _MULTIMODEFILTER_H_
#define _MULTIMODEFILTER_H_
#include "stdint.h"

#define MM_FILTER_LOWPASS 0
#define MM_FILTER_BANDPASS 1
#define MM_FILTER_HIGHPASS 2

typedef struct 
{
    float interm0;
    float interm1;
    float cutoff; // from 0 to 1 
    float resonance; // from 0 to 1
    float feedback;
    uint8_t type;
} MultimodeFilterType;


float MMFilterProcessSample(float sampleIn,MultimodeFilterType*data);
inline void MMFilterCalculateFeedbackAmount(MultimodeFilterType* data) { data->feedback = data->resonance + data->resonance/(1.0f - data->cutoff); }
inline void MMFilterSetCutoff(float cutoff,MultimodeFilterType* data) { data->cutoff = cutoff; MMFilterCalculateFeedbackAmount(data);}
inline void MMFilterSetResonance(float reso,MultimodeFilterType* data) { data->resonance = reso; MMFilterCalculateFeedbackAmount(data);}
inline void MMFilterSetType(uint8_t type,MultimodeFilterType*data) { if (type < 3) {data->type = type;}}


#endif