#ifndef _DELAY_MEMORY_HANDLER_H_
#define _DELAY_MEMORY_HANDLER_H_
#include "stdint.h"

#ifdef RP2040_FEATHER
typedef uint32_t ptr;
#else
typedef uint64_t ptr;
#endif

typedef struct 
{
    ptr startAddress;
    ptr endAddress;
} delayMemoryBlockType;

void initDelayMemoryHandler(void);
int16_t * mallocDelayMemory(ptr size);
void freeDelayMemory(int16_t * ptr);
void setStart(ptr strt); // only used for testing on a 64bit system
#endif