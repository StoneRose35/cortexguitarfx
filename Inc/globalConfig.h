#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_


// uncomment if compiling for useage with the stomp extension board
#define EXTENSION_BOARD

// #define WAVESHARE_64X128_DISPLAY
#define JOYIT_128X64_DISPLAY

// use CS4270 audio codec
#define CS4270_AUDIO_CODEC

// use PCM3060 audio codec
//#define PCM3060_AUDIO_CODEC

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

#if defined(CS4270_AUDIO_CODEC) && defined(PCM3060_AUDIO_CODEC)
#error Multiple Audio Codecs defined, select only one.
#endif

#if !defined(CS4270_AUDIO_CODEC) && !defined(PCM3060_AUDIO_CODEC)
#error Choose an audio codec.
#endif


#endif