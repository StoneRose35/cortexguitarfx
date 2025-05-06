#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_


#define EXTENSION_BOARD

// #define WAVESHARE_64X128_DISPLAY
//#define JOYIT_128X64_DISPLAY
#define DOGM128_DISPLAY

#define INTERNAL_CODEC 
//#define EXTERNAL_CODEC
#define WM8731_CODEC
// #define PCM3060_CODEC


#define USB_VENDOR_ID 0x4A37
#define USB_PRODUCT_ID 0x35D2
// debug swiches (return information over UART@115200 8-n-1 when enabled)
#undef USB_DBG
#undef  I2C_DBG





// --------------------------
// derived preprocessor directives

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

#if !defined EXTERNAL_CODEC && !defined INTERNAL_CODEC
    #error "define either EXTERNAL_CODED or INTERNAL_CODEC"
#endif 

#endif