#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_
#include <stdint.h>

#define ROTARY_ENCODER_DEBOUNCE 10

typedef struct {
    int16_t increment; // the actual increment value
    uint32_t deltaTime; // the time measured between this and the previous increment in microsecond ticks
} RotaryEncoderIncrementType;

void initRotaryEncoder(const uint8_t* pins,const uint8_t nswitches);

uint32_t getEncoderValue();

uint8_t getSwitchValue(uint8_t);

void clearPressedStickyBit(uint8_t nr);

void clearReleasedStickyBit(uint8_t nr);

int16_t getStickyIncrementDelta();

void clearStickyIncrementDelta();

uint8_t getMomentarySwitchValue(uint8_t sw);

void getStickyIncrementAndTime(RotaryEncoderIncrementType * res);

#endif

