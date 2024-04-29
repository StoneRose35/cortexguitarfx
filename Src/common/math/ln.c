#include <stdint.h>
#define TOL 0.000001f
// taken from https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
__attribute__((section (".qspi_code")))
float toLn(float x) {
  float xStacked = x;
  float * xPtr = &xStacked;
  uint32_t* bx = ((uint32_t*)(xPtr));
  uint32_t ex = *bx >> 23;
  int32_t t = (int32_t)ex-(int32_t)127;
  //uint32_t s = (t < 0) ? (-t) : t;
  *bx = 1065353216 | (*bx & 8388607);
  xPtr = (float *) (bx);
  return -1.49278f+(2.11263f+(-0.729104f+0.10969f**xPtr)**xPtr)**xPtr+0.6931471806f*(float)t;

}

// claculates 20*log10(x), using a 2nd order polynomial approach obtained using the remez algo
__attribute__((section (".qspi_code")))
float toDb(float x) {
  float xStacked = x;
  float * xPtr = &xStacked;
  uint32_t* bx =  (uint32_t*) (xPtr);
  uint32_t ex = *bx >> 23;
  int32_t t = (int32_t)ex-(int32_t)127;
  //uint32_t s = (t < 0) ? (-t) : t;
  *bx = 1065353216 | (*bx & 8388607);
  xPtr = (float *) (bx);
  return -10.08376785f + 12.18970261f**xPtr + -2.07619445f**xPtr**xPtr +6.020599913279f*(float)t;
}

__attribute__((section (".qspi_code")))
// computes a linear value given a db value using the bisection algorithm
float toLin(float y)
{
  float x1=0.000001f, x2=1.0f;
  float xnew,ynew;
  if (y<-120.0f)
  {
    return 0.f;
  }

  while (x2-x1 > TOL)
  {
    xnew = (x1 + x2)*0.5f;
    ynew = toDb(xnew)-y;
    if (ynew < 0 )
    {
      x1 = xnew;
    }
    else
    {
      x2 = xnew;
    }
  }
  return x1;
}