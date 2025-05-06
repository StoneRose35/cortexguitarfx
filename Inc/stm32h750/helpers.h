#ifndef _HELPERS_H_
#define _HELPERS_H_
void short_nop_delay();

void nop_wait(uint32_t cycles);
float convolve(const float*coeffs,float*data,uint32_t offset);


#endif