#ifndef _CLI_API_TASK_H_
#define _CLI_API_TASK_H_
#include <stdint.h>
void initCliApi(BufferedInputType * bfr,ConsoleType * console,ApiType* api,CommBuffer commBfr, bufferConsumerType consumer);
void cliApiTask(BufferedInput);

#endif