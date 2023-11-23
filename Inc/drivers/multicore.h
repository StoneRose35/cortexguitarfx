#ifndef _MULTICORE_H_
#define _MULTICORE_H_
#include <stdint.h>

void startCore1(void(*secondMain)());

void __sev();

#endif