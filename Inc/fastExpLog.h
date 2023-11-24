#ifndef _FAST_EXP_LOG_H_
#define _FAST_EXP_LOG_H_

#include "stdint.h"
extern const int16_t * logtable;
extern const int16_t * exptable;
int16_t fastlog(int16_t  x);
int16_t fastexp(int16_t x);
int16_t asDb(int16_t x);
#endif