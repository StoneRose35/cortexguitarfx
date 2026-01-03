#include <stdint.h>
#include "memoryRegions.h"
#define TOL 0.00001f
// taken from https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
__ITCM_CODE
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
__ITCM_CODE
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

__ITCM_CODE
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

__ITCM_CODE
// computes exp(y) by calculating 2 ^ (x * log2(e)) and placing exponent and matissa directly
float toExp(float x)
{
  float nr;
  int32_t exponentInteger;
  float * nrPtr= &nr;
  *((uint32_t*)nrPtr)=0;  
  x = x*1.4426950f; // x = x*log2(e)
  if (x < 0)
  {
    exponentInteger = ((int32_t)x) -1;
  }
  else
  {
    exponentInteger = (int32_t)x;
  }
  *((uint32_t*)nrPtr)= (exponentInteger+127) << 23;
  float exponentFraction = x - (float)exponentInteger;
  float twoFractionalPower = (0.00247606f + 0.65104678f*exponentFraction + 0.34400111f*exponentFraction*exponentFraction);
  *((uint32_t*)nrPtr) |= (uint32_t)(twoFractionalPower*8388607.0f);
  return *nrPtr;

}