#ifndef _TREMOLO_H_
#define _TREMOLO_H_
#include "stdint.h"
#include "audio/phaseDistortedSineSquare.h"


 typedef struct {
    int16_t depth;
    uint16_t lfoUpdateCnt;
    int16_t currentLfoVal;
    int16_t nextLfoVal;
    PhaseDistortedSineSquareType modulator;

 } TremoloType;

__attribute__ ((section (".ramfunc"))) int16_t tremoloProcessSample(int16_t sample,TremoloType*data);
void initTremolo(TremoloType*data);
#endif