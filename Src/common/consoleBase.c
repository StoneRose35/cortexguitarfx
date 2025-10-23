#include <stdint.h>
#include "system.h"
#include "bufferedInputHandler.h"

extern uint8_t context; // used by printf to decide where a certain information should be output

extern CommBufferType usbCommBuffer __attribute__((aligned (256)));

extern CommBufferType btCommBuffer;

// TODO: properly handle the blocked case
// it seems data is sent multiple time when the output buffer would run over
void printf(const char* data)
{
	appendStringToOutputBuffer(&usbCommBuffer,data);

}