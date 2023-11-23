#ifndef _I2S_H_
#define _I2S_H_
#include <stdint.h>

#define F_SAMPLING 48000

#define AUDIO_BUFFER_SIZE 16 // in stereo 16bit samples, thus size of one audio buffer in bytes is this value*4


void initI2S();
void initI2SSlave();
void enableAudioEngine();
void disableAudioEngine();
void toggleAudioBuffer();
int16_t* getEditableAudioBuffer();
#ifdef I2S_INPUT
int16_t* getInputAudioBuffer();
void toggleAudioInputBuffer();
#endif
void retriggerInput();
#endif