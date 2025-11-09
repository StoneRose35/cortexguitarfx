#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_
#include <stdint.h>

#define ROTARY_ENCODER_DEBOUNCE 10

typedef struct {
    int16_t increment; // the actual increment value
    int32_t speed; // the number of increments in the last 10 ms
} RotaryEncoderIncrementType;

void initRotaryEncoder(const uint8_t* pins,const uint8_t nswitches);

uint32_t getEncoderValue();

uint8_t getSwitchValue(uint8_t);

void clearPressedStickyBit(uint8_t nr);

void clearReleasedStickyBit(uint8_t nr);

void clearStickyIncrementDelta();

uint8_t getMomentarySwitchValue(uint8_t sw);

void getStickyIncrementAndSpeed(RotaryEncoderIncrementType * res);

#endif

