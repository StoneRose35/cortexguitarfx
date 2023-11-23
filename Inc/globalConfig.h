#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_


// uncomment if compiling for useage with the stomp extension board
#define EXTENSION_BOARD

// #define WAVESHARE_64X128_DISPLAY
#define JOYIT_128X64_DISPLAY


#ifdef WAVESHARE_64X128_DISPLAY
#define VERTICAL_DISPLAY
#define SH1107
#define HORIZONTAL_FLIP
#endif

#ifdef JOYIT_128X64_DISPLAY
#define HORIZONTAL_DISPLAY
#define SH1106
#define VERTICAL_FLIP
#define HORIZONTAL_FLIP
#endif

#endif