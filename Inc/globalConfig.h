#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_

#define AUDIO_SAMPLING_RATE 48000

// uncomment if compiling for useage with the stomp extension board
#define EXTENSION_BOARD

// #define WAVESHARE_64X128_DISPLAY
//#define JOYIT_128X64_DISPLAY
#define DOGM128_DISPLAY

// use CS4270 audio codec
//#define CS4270_AUDIO_CODEC

// use PCM3060 audio codec
#define PCM3060_AUDIO_CODEC

// uncomment to bypass the audio codec triggering the UI update
// the audio codec clocks are held at reset in this case
//#define TRIGGER_UI_BY_CORE_0

// uncomment to enable UART over USB
#undef USB_UART

// set 0 if the clipping LED's are connected between pin and Ground (RP2040 as source)
// set 1 if the clipping LED's are connected between pin and 3.3V (RP2040 as sink)
#define CLIPPING_LED_POLARITY 1

// comment in to flash a test program useful for hardware integrity check
//#define FORCE_TEST_MODE

//#define AVR_FLASHER // enables the flasher for the avr (board version 0.3.0 or higher)

//------------------------------------------
// derived definitions
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
#define VERTICAL_FLIP
#define HORIZONTAL_FLIP
#endif

#if defined(CS4270_AUDIO_CODEC) && defined(PCM3060_AUDIO_CODEC)
#error Multiple Audio Codecs defined, select only one.
#endif

#if !defined(CS4270_AUDIO_CODEC) && !defined(PCM3060_AUDIO_CODEC)
#error Choose an audio codec.
#endif

#ifdef USB
#define USB_CDC_DRIVER
#define USB_HW_DRIVER
#endif

#endif