#ifndef _DOGM128_DISPLAY_H_
#define _DOGM128_DISPLAY_H_
#include "stdint.h"
#include "globalConfig.h"
#define DOGM128_N_PAGES 8
#define DOGM128_DISPLAY_N_COLUMNS 128

#define DOGM128_CS_DELAY 0x10


#ifdef HORIZONTAL_FLIP
#define DOGM128_COM_DIRECTION 0xC8
#else
#define DOGM128_COM_DIRECTION 0xC0
#endif

#ifdef HORIZONTAL_FLIP
#define HORIZONTAL_OFFSET 4 // set to 4 if display is flipped
#else
#define HORIZONTAL_OFFSET 0
#endif

#ifdef VERTICAL_FLIP
#define DOGM128_ADC_REVERSE 0xA0
#else
#define DOGM128_ADC_REVERSE 0xA1
#endif

#endif