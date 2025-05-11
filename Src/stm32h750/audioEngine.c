
#include "stdint.h"
#include "sai.h"
#include "timer.h"
#include "system.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "audio/audiotools.h"
#include "pipicofx/pipicofxui.h"

#define AVERAGING_LOWPASS_CUTOFF 0.000305f // 10/32768
#define UI_UPDATE_IN_SAMPLE_BUFFERS 256

volatile uint16_t bufferCnt;

extern volatile int16_t *currentSamplePointer;
extern volatile uint8_t sampleSelectorVal;
extern volatile uint32_t currentSamplePosition;
extern int16_t **samplePointers;
extern uint32_t *sampleLengths;
__ITCM_CODE
void processAudioBuffers(void)
{
    int32_t outputSampleInt;
    uint32_t ticStart,ticEnd;
    int32_t* audioBufferPtr;
    float outputSample,avgOut;

    extern volatile uint16_t audioTransferState;
    extern volatile uint32_t cpuLoad;
    extern float avgOutOld;
    extern uint32_t task;


    ticStart = getTimeLW();
    audioBufferPtr = getEditableAudioBufferHiRes();
    for (uint32_t c=0;c<AUDIO_BUFFER_SIZE*2;c+=2) // count in frame of 4 bytes or two  24bit samples
    {
        
        // convert raw input to float
        /*
        #ifdef EXTENSION_BOARD
            inputSampleInt = ((int32_t)(((uint32_t)*(audioBufferInputPtr + c + 1)) << 8) >> 8) + 
                             ((int32_t)(((uint32_t)*(audioBufferInputPtr + c)) << 8) >> 8);
        #else

            #ifdef PCM3060_CODEC
            inputSampleInt = ((int32_t)(((uint32_t)*(audioBufferInputPtr + c + 1)) << 8) >> 8);
            #else
            inputSampleInt = ((int32_t)(((uint32_t)*(audioBufferInputPtr + c)) << 8) >> 8);
            #endif
        #endif
        inputSample=(float)inputSampleInt;
        inputSample /= 8388608.0f;
        #ifdef EXTENSION_BOARD
        if (inputSample < -1.0f || inputSample > 1.0f)
        {
            audioState |= AUDIO_STATE_INPUT_CLIPPED;
        }
        #endif
        */

        if (((uint32_t)currentSamplePointer)!=0xFFFFFFFF)
        {
            outputSample = ((float)*(currentSamplePointer +currentSamplePosition++)) / 32768.0f;
            if (currentSamplePosition >= *(sampleLengths+sampleSelectorVal))
            {
                currentSamplePointer = (int16_t*)0xFFFFFFFF;
                currentSamplePosition=0;
            }
        }
        else
        {
            outputSample = 0.0f;
        }

        if (outputSample < 0.0f)
        {
            avgOut = -outputSample;
        }
        else
        {
            avgOut = outputSample;
        }
        avgOutOld = AVERAGING_LOWPASS_CUTOFF*avgOut + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgOutOld);
        outputSample=clip(outputSample,getAudioStatePtr());
        outputSampleInt=((int32_t)(outputSample*8388607.0f));
        *(audioBufferPtr+c) = outputSampleInt;  
        *(audioBufferPtr+c+1) = outputSampleInt;
    }
    task &= ~((1 << TASK_PROCESS_AUDIO) | (1 << TASK_PROCESS_AUDIO_INPUT));
    bufferCnt++;
    if (bufferCnt == UI_UPDATE_IN_SAMPLE_BUFFERS)
    {
        bufferCnt = 0;
        task |= (1 << TASK_UPDATE_AUDIO_UI);
    }

    ticEnd = getTimeLW();
    if(ticEnd > ticStart)
    {
        cpuLoad = ticEnd-ticStart;
        cpuLoad = cpuLoad*196; // *256*256*F_SAMPLING/AUDIO_BUFFER_SIZE/1000000;
        cpuLoad = cpuLoad >> 8;
    }
    audioTransferState=0;
}