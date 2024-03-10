#ifndef _FAST_EXP_LOG_H_
#define _FAST_EXP_LOG_H_

#include "stdint.h"
extern const int16_t * logtable;
extern const int16_t * exptable;
float fastlog(float  x);
float fastexp(float x);

#endif