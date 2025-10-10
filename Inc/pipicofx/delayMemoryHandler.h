#ifndef _DELAY_MEMORY_HANDLER_H_
#define _DELAY_MEMORY_HANDLER_H_
#include "stdint.h"
#define DELAY_LINE_SDRAM_LENGTH (1<<23)
#define DELAY_LINE_RAM_LENGTH (1<<17)
#define MAX_DELAY_SINGLEBUFFER (1<<15)

#ifdef STM32
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
float * mallocDelayMemory(ptr size);
void freeDelayMemory(float * ptr);
void setStart(ptr strt); // only used for testing on a 64bit system
#endif