#ifndef _DOGM128_DISPLAY_H_
#define _DOGM128_DISPLAY_H_
#include "stdint.h"
#define DOGM128_N_PAGES 8
#define DOGM128_DISPLAY_N_COLUMNS 128
#define HORIZONTAL_OFFSET 0 // set to 4 if display is flipped

#define DOGM128_CS_DELAY 0x7F
void dogm128SendCommand(uint8_t cmd);
void dogm128SendData(const uint8_t*data,uint8_t l);

void dogm128Disable();
void dogm128Enable();
#endif