#ifndef _DELAY_MEMORY_HANDLER_H_
#define _DELAY_MEMORY_HANDLER_H_
#include "stdint.h"
typedef struct 
{
    uint32_t startAddress;
    uint32_t endAddress;
} delayMemoryBlockType;

void initDelayMemoryHandler(void);
int16_t * mallocDelayMemory(uint16_t size);
void freeDelayMemory(int16_t * ptr);
void setStart(uint32_t strt); // only used for testing on a 64bit system
#endif