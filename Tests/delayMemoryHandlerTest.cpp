#include "pipicofx/delayMemoryHandler.h"
#include "stdint.h"
#include "stdio.h"
#include "audio/delay.h"


// check if a small fraction is placed from end to start
int delayMemoryHandlerTest1(void)
{
    int16_t* ptrs[4];
    initDelayMemoryHandler();
    setStart(0x0);
    ptrs[0] = mallocDelayMemory(4096);
    // ------------------------------------ / ptrs[0] ---/
    ptrs[1] = mallocDelayMemory(2048);
    // ----------------------/ ptrs[1]----- / ptrs[0] ---/

    freeDelayMemory(ptrs[0]);
    // ----------------------/ ptrs[1]----- /------------/
    ptrs[2] = mallocDelayMemory(4000);
    // ----------------------/ ptrs[1]----- / ptrs[2]--/-/

    if ((uint32_t)ptrs[2] >= DELAY_LINE_LENGTH-4096)
    {
        printf("delay memory handler test 1 succeeded\r\n");
        return 0;
    }
    return 1;
}


// check if a larger fraction is placed at the beginning
int delayMemoryHandlerTest2(void)
{
    int16_t* ptrs[4];
    initDelayMemoryHandler();
    setStart(0x0);
    ptrs[0] = mallocDelayMemory(4096);
    // ------------------------------------ / ptrs[0] ---/
    ptrs[1] = mallocDelayMemory(2048);
    // ----------------------/ ptrs[1]----- / ptrs[0] ---/

    freeDelayMemory(ptrs[0]);
    // ----------------------/ ptrs[1]----- /------------/
    ptrs[2] = mallocDelayMemory(8000);
    // ---------/ ptrs[2]----/ ptrs[1]----- /------------/

    if ((uint32_t)ptrs[2] < DELAY_LINE_LENGTH-4096-2048)
    {
        printf("delay memory handler test 2 succeeded\r\n");
        return 0;
    }
    return 1;
}

// check if a ptrs[3] lies in the section originally occupied by ptrs[1]
int delayMemoryHandlerTest3(void)
{
    int16_t* ptrs[4];
    initDelayMemoryHandler();
    setStart(0x0);
    ptrs[0] = mallocDelayMemory(4096);
    // ------------------------------------ / ptrs[0] ---/
    ptrs[1] = mallocDelayMemory(4096);
    // ----------------------/ ptrs[1]----- / ptrs[0] ---/
    ptrs[2] = mallocDelayMemory(4096);
    // ---------/ ptrs[2]----/ ptrs[1]----- / ptrs[0] ---/

    freeDelayMemory(ptrs[1]);
    // ---------/ ptrs[2]----/ ------------ / ptrs[0] ---/
    ptrs[3] = mallocDelayMemory(256);
    // ---------/ ptrs[2]----/ ptrs[3]-/--- / ptrs[0] ---/
    if ((uint32_t)ptrs[3] >= DELAY_LINE_LENGTH - 8192)
    {
        printf("delay memory handler test 3 succeeded\r\n");
        return 0;
    }
    return 1;
}

int main(int argc,char ** argv)
{
    return delayMemoryHandlerTest3();
}

// allocate three blocks, free the second allocated
// allocate one with a size equal or smaller to the second, make sure it 
// is place in-between the first and the third