#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_
#include <stdint.h>

#define ROTARY_ENCODER_DEBOUNCE 1


void initRotaryEncoder(const uint8_t* pins,const uint8_t nswitches);

uint32_t getEncoderValue();

uint8_t getSwitchValue(uint8_t);

#endif

