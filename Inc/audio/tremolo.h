#ifndef _TREMOLO_H_
#define _TREMOLO_H_
#include "stdint.h"
#include "audio/phaseDistortedSineSquare.h"


#define TREMOLO_LFO_UPDATE_DIV_POW2 5
#define TREMOLO_LFO_UPDATE_DIV (1<<TREMOLO_LFO_UPDATE_DIV_POW2)

 typedef struct {
    int16_t depth;
    uint16_t lfoUpdateCnt;
    int16_t currentLfoVal;
    int16_t nextLfoVal;
    PhaseDistortedSineSquareType modulator;

 } TremoloType;

int16_t tremoloProcessSample(int16_t sample,TremoloType*data);
void initTremolo(TremoloType*data);
#endif