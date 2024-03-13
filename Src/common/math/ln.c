#include <stdint.h>

// taken from https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
float fln(float x) {
  uint32_t bx = * (uint32_t*) (&x);
  uint32_t ex = bx >> 23;
  int32_t t = (int32_t)ex-(int32_t)127;
  uint32_t s = (t < 0) ? (-t) : t;
  bx = 1065353216 | (bx & 8388607);
  x = * (float *) (&bx);
  return -1.49278f+(2.11263f+(-0.729104f+0.10969f*x)*x)*x+0.6931471806f*t;
}