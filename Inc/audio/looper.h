#ifndef _LOOPER_H_
#define _LOOPER_H_
#include "stdint.h"
#define LOOPER_STATE_STOPPED 0
#define LOOPER_STATE_RECORDING 1
#define LOOPER_STATE_PLAYING 2
#define LOOPER_STATE_OVERDUBBING 3
#define LOOPER_INDEX_NULL 0xFFFFFFFF

#define LOOPER_FUNCTION_RETRIGGER 0
#define LOOPER_FUNCTION_EXIT 1
typedef struct {
    float * memoryPointer;
    float playVolume;
    float recordingVolume;
    uint32_t indexStart;
    uint32_t indexEnd;
    uint32_t currentPosition;
    uint8_t looperState;
    uint8_t looperFunction;
} LooperDataType;


void LooperInit(LooperDataType*);
float LooperProcessSample(float, LooperDataType*);
void LooperStartPlaying(LooperDataType*);
void LooperStartRecording(LooperDataType*);
void LooperStopRecording(LooperDataType*);
void LooperStop(LooperDataType*);
void LooperPause(LooperDataType*);
void LooperDelete(LooperDataType*);
#endif