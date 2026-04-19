#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_


#define EXTENSION_BOARD

// #define WAVESHARE_64X128_DISPLAY
//#define JOYIT_128X64_DISPLAY
#define DOGM128_DISPLAY

//#define INTERNAL_CODEC 
#define EXTERNAL_CODEC
//#define WM8731_CODEC
#define PCM3060_CODEC_EXTERNAL


#define USB_VENDOR_ID 0x4A37 // usb vendor id in normal mode
#define USB_PRODUCT_ID 0x35D2 // usb product id in normal mode

#define USB_VENDOR_ID_BOOTMODE 0x0483 // usb vendor id when in rescue mode (boot pressed while reset is toggled)
#define USB_PRODUCT_ID_BOOTMODE 0xdf11 // usb product id when in rescue mode (boot pressed while reset is toggled)
// debug swiches (return information over UART@115200 8-n-1 when enabled)
#undef USB_DBG
#undef  I2C_DBG

#undef DFU_SIM //simulate flashing using DFU (device firmware update)
//#define FORCE_TEST_MODE
//#define ENCODER_TUNE //enable debug output to tune the encoder acceleration

//#define AVR_FLASHER // enables the flasher for the avr (board version 0.3.0 or higher)


// audio ui is updated every UI_UPDATE_IN_SAMPLE_BUFFER buffers
#define UI_UPDATE_IN_SAMPLE_BUFFERS 256 

// sampling frequency in Hz
#define F_SAMPLING 48000

// the size of and audio buffer, effectively defines latency as F_SAMPLING/AUDIO_BUFFER_SIZE
#define AUDIO_BUFFER_SIZE 16 // in stereo 16bit samples, thus size of one audio buffer in bytes is this value*4

// the minimal duration of a long press in 10ms, a value of 100 would result in 1s duration
#define LONGPRESS_DURATION_SYSTICKS 130

// the alpha value for the lowpass filter for the adc values, the effective alpha is ADC_LOWPASS/1024
#define ADC_LOWPASS 60

// how close the knob should be respetive to the given parameter to unlock the knob
#define KNOB_HYSTERESIS 64

// how much the knob should move before it is unlocked
#define KNOB_HYSTERESIS_2 20

#define UI_LATENCY_IN_SAMPLES (UI_UPDATE_IN_SAMPLE_BUFFERS*AUDIO_BUFFER_SIZE)
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

#ifdef DOGM128_DISPLAY
#define HORIZONTAL_DISPLAY
#define VERTICAL_FLIP
#define HORIZONTAL_FLIP
#endif

#if !defined EXTERNAL_CODEC && !defined INTERNAL_CODEC
    #error "define either EXTERNAL_CODED or INTERNAL_CODEC"
#endif 

#if defined PCM3060_CODEC_EXTERNAL || defined PCM3060_CODEC_INTERNAL
#define PCM3060_CODEC
#endif

#endif