#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "sai.h"
#include "timer.h"
#include "system.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "audio/audiotools.h"
#include "audio/looper.h"
#include "pipicofx/pipicofxui.h"

#define AVERAGING_LOWPASS_CUTOFF 0.000305f // 10/32768
#define UI_UPDATE_IN_SAMPLE_BUFFERS 256

volatile int16_t fadeCounter;
volatile uint16_t bufferCnt;

__ITCM_CODE
void processAudioBuffers(void)
{
    int32_t inputSampleInt,outputSampleInt;
    uint32_t ticStart,ticEnd;
    int32_t* audioBufferPtr,*audioBufferInputPtr;
    float inputSample,outputSample,avgIn,avgOut;


    extern volatile uint8_t programChangeState;
    extern volatile uint16_t audioState;
    extern volatile uint16_t audioTransferState;
    extern volatile uint32_t cpuLoad;
    extern float avgInOld,avgOutOld;
    extern uint32_t task;
    extern PiPicoFxUiType piPicoUiController;
    extern LooperDataType looper;
    ticStart = getTimeLW();
    audioBufferPtr = getEditableAudioBufferHiRes();
    audioBufferInputPtr = getInputAudioBufferHiRes();
    for (uint32_t c=0;c<AUDIO_BUFFER_SIZE*2;c+=2) // count in frame of 4 bytes or two  24bit samples
    {
        
        // convert raw input to float
        #ifdef EXTENSION_BOARD
            inputSampleInt = ((int32_t)(((uint32_t)*(audioBufferInputPtr + c + 1)) << 8) >> 8) + 
                             ((int32_t)(((uint32_t)*(audioBufferInputPtr + c)) << 8) >> 8);
        #else

            #ifdef PCM3060_CODEC_INTERNAL
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


        if (inputSample < 0.0f)
        {
            avgIn = -inputSample;
        }
        else
        {
            avgIn = inputSample;
        }
        avgInOld = AVERAGING_LOWPASS_CUTOFF*avgIn + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgInOld);

        if (programChangeState != 3) // processing
        {
            outputSample = piPicoUiController.currentProgram->processSample(inputSample);
        }
        else
        {
            outputSample = 0.0f;
        }

        outputSample = LooperProcessSample(outputSample,&looper);

        if (programChangeState == 2)// fadeout
        {
            outputSample = (((float)(32767 - fadeCounter)*inputSample) + (((float)fadeCounter*outputSample)))/32767.0f;
            fadeCounter -= 256;
            if (fadeCounter < 0)
            {
                fadeCounter = 0;
                programChangeState=3;
            }
        }
        else if (programChangeState==4) // fadein
        {
            outputSample = (((float)(32767 - fadeCounter)*inputSample) + (((float)fadeCounter*outputSample)))/32767.0f;
            fadeCounter += 256;
            if (fadeCounter < 0) // overrun
            {
                programChangeState = 0;
            }
        }
        if (programChangeState == 1)
        {
            fadeCounter = 32767;
            programChangeState = 2;
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
        //inputSampleInt = (((inputSampleInt << 8) & 0xFFFF) << 16) | (((inputSampleInt << 8) & 0xFFFF0000L) >> 16);
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

#ifdef __cplusplus
}
#endif
