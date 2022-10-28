#include "stringFunctions.h"
#include "stdint.h"


float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

int main(char ** argv,int argc)
{
    int16_t testnr=1;
    char outstring[32];

    decimalInt16ToChar(testnr,outstring,2);
}