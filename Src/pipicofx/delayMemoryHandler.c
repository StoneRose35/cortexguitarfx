#include "stdint.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "pipicofx/delayMemoryHandler.h"


// TODO: handle SDRAM and RAM delay memories properly

volatile float delayMemory[DELAY_LINE_RAM_LENGTH];

static volatile uint8_t takenDelayMemoryBlocksCnt;
static volatile ptr globalStart;
static volatile ptr globalEnd;
static delayMemoryBlockType takenDelayMemoryBlocks[32];


void setStart(ptr strt)
{
    globalStart = strt;
    globalEnd = strt + (DELAY_LINE_RAM_LENGTH<<2);
}

void initDelayMemoryHandler(void)
{
    takenDelayMemoryBlocksCnt = 0;
    globalStart = (ptr)delayMemory;
    globalEnd = globalStart + (DELAY_LINE_RAM_LENGTH<<2);
}

float * mallocDelayMemory(ptr size)
{
    ptr initialAddress= globalEnd;
    uint8_t indexToEnter=0;
    if (takenDelayMemoryBlocksCnt == 0 && size <= globalEnd-globalStart)
    {
        initialAddress -= size;
        takenDelayMemoryBlocks[0].startAddress = initialAddress;
        takenDelayMemoryBlocks[0].endAddress = initialAddress + size;
        takenDelayMemoryBlocksCnt += 1;
        return (float*)(takenDelayMemoryBlocks[0].startAddress);
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
        return (float*)(takenDelayMemoryBlocks[indexToEnter].startAddress);
    }
    return (float*)0;
}

void freeDelayMemory(float * pointer)
{
    // get size to free up
    for (uint8_t c=0;c<takenDelayMemoryBlocksCnt;c++)
    {
        if (takenDelayMemoryBlocks[c].startAddress == (ptr)pointer)
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