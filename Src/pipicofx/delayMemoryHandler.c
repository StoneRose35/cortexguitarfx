#include "stdint.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "pipicofx/delayMemoryHandler.h"

volatile int16_t delayMemory[DELAY_LINE_LENGTH];

static volatile uint8_t takenDelayMemoryBlocksCnt;
static volatile uint32_t globalStart;
static volatile uint32_t globalEnd;
static delayMemoryBlockType takenDelayMemoryBlocks[32];


void setStart(uint32_t strt)
{
    globalStart = strt;
    globalEnd = strt + DELAY_LINE_LENGTH;
}

void initDelayMemoryHandler(void)
{
    takenDelayMemoryBlocksCnt = 0;
    globalStart = (uint32_t)delayMemory;
    globalEnd = (uint32_t)(delayMemory + (DELAY_LINE_LENGTH<<1));
}

int16_t * mallocDelayMemory(uint32_t size)
{
    uint32_t initialAddress= globalEnd;
    uint8_t indexToEnter=0;
    if (takenDelayMemoryBlocksCnt == 0 && size < globalEnd-globalStart)
    {
        initialAddress -= size;
        takenDelayMemoryBlocks[0].startAddress = initialAddress;
        takenDelayMemoryBlocks[0].endAddress = initialAddress + size;
        takenDelayMemoryBlocksCnt += 1;
        return (int16_t*)(takenDelayMemoryBlocks[0].startAddress);
    }
    for (int16_t c=takenDelayMemoryBlocksCnt-1;c>=0;c--)
    {
        if (takenDelayMemoryBlocks[c].endAddress +size < initialAddress)
        {
            initialAddress = takenDelayMemoryBlocks[c].endAddress+size;
            indexToEnter = c+1;
            break;
        }
        else
        {
            initialAddress = takenDelayMemoryBlocks[c].startAddress;
        }
    }
    if (initialAddress-size>=globalStart)
    {
        for (uint8_t c=takenDelayMemoryBlocksCnt;c>indexToEnter;c--)
        {
            takenDelayMemoryBlocks[c].startAddress = takenDelayMemoryBlocks[c-1].startAddress;
            takenDelayMemoryBlocks[c].endAddress = takenDelayMemoryBlocks[c-1].endAddress;
        }
        takenDelayMemoryBlocks[indexToEnter].startAddress = initialAddress-size;
        takenDelayMemoryBlocks[indexToEnter].endAddress = initialAddress;
        takenDelayMemoryBlocksCnt +=1;
        return (int16_t*)takenDelayMemoryBlocks[indexToEnter].startAddress;
    }

    return (int16_t*)0;
}

void freeDelayMemory(int16_t * ptr)
{
    // get size to free up
    for (uint8_t c=0;c<takenDelayMemoryBlocksCnt;c++)
    {
        if (takenDelayMemoryBlocks[c].startAddress == (uint32_t)ptr)
        {
            for (uint8_t cc=c;cc<takenDelayMemoryBlocksCnt-1;cc++)
            {
                takenDelayMemoryBlocks[cc].startAddress = takenDelayMemoryBlocks[cc+1].startAddress;
                takenDelayMemoryBlocks[cc].endAddress = takenDelayMemoryBlocks[cc+1].endAddress;
            }
            takenDelayMemoryBlocksCnt -= 1;
            break;
        }
    }
}