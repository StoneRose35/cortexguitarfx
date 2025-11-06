#include "audio/looper.h"
#include "memoryRegions.h"

__SDRAM_BSS
static float looperMemory[1<<22];

void LooperInit(LooperDataType* data)
{
    data->indexStart = 0x0;
    data->indexEnd= LOOPER_INDEX_NULL;
    data->looperState = LOOPER_STATE_STOPPED;
    data->memoryPointer = looperMemory;
    data->playVolume = 1.0f;
    data->recordingVolume = 1.0f;
    data->currentPosition = 0;

}

__ITCM_CODE
float LooperProcessSample(float sampleIn, LooperDataType* data)
{
    float sampleOut;
    if (data->looperState == LOOPER_STATE_STOPPED)
    {
        return sampleIn;
    }
    if (data->looperState != LOOPER_STATE_RECORDING)
    {
        sampleOut = sampleIn + *(data->memoryPointer + data->currentPosition)*data->playVolume;
    }
    else
    {
        sampleOut = sampleIn;
    }
    
    if (data->looperState == LOOPER_STATE_RECORDING)
    {

        *(data->memoryPointer + data->currentPosition) = sampleIn*data->recordingVolume; 
        
    }
    if (data->looperState == LOOPER_STATE_OVERDUBBING)
    {
        *(data->memoryPointer + data->currentPosition) += sampleIn*data->recordingVolume; 
    }

    if (++data->currentPosition >= data->indexEnd)
    {
        data->currentPosition = data->indexStart;
    }
    return sampleOut;
}

void LooperStartPlaying(LooperDataType*data)
{
    if (data->indexEnd != LOOPER_INDEX_NULL) // only play if material is recorded
    {
        data->currentPosition = data->indexStart;
        data->looperState = LOOPER_STATE_PLAYING;
    }
}

void LooperStartRecording(LooperDataType*data)
{
    if (data->looperState == LOOPER_STATE_PLAYING)
    {
        data->looperState = LOOPER_STATE_OVERDUBBING;
        return;
    }
    data->looperState = LOOPER_STATE_RECORDING;
}

void LooperStopRecording(LooperDataType*data)
{
    if (data->looperState == LOOPER_STATE_RECORDING)
    {
        data->looperState = LOOPER_STATE_PLAYING;
    }
    data->indexEnd = data->currentPosition;
}

void LooperStop(LooperDataType*data)
{
    data->currentPosition = data->indexStart;
    data->looperState = LOOPER_STATE_STOPPED;
}

void LooperPause(LooperDataType*data)
{
    data->looperState = LOOPER_STATE_STOPPED;
}

void LooperDelete(LooperDataType* data)
{
    data->looperState = LOOPER_STATE_STOPPED;
    data->indexEnd = LOOPER_INDEX_NULL;
    data->currentPosition = 0;
    data->indexStart=0;
}