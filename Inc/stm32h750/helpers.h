#ifndef _HELPERS_H_
#define _HELPERS_H_
void short_nop_delay();

float convolve(const float*coeffs,float*data,uint32_t offset);
#endif