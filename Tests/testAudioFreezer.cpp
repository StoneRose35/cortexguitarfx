
#include "pipicofx/005_Delay.hpp"
#include <cmath>


extern "C" {
void zeroString(char*data,int16_t len)
{
    for (uint16_t c=0;c<len;c++)
    {
        *(data+c)=0;
    }
}

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float fln(float a)
{
    return logf(a);
}


float convolve(const float*coeffs,float*data,uint32_t offset)
{
    float res=0.0f;
    for(uint32_t c=0;c<64;c++)
    {
        res += *(coeffs + c) * *(data + ((offset + c)&0x3F));
    }
    return res;
}

}
int main(int argc,char** argv)
{
    initDelayMemoryHandler();
    Delay::Delay * delay= new Delay::Delay();
    float sampleIn;
    for(uint16_t c=0;c<256;c++)
    {
        sampleIn = sinf(((float)c)*432.0/48000.0*2*3.14159f);
        delay->processSample(sampleIn);
        if (c==12)
        {
            delay->toggleOn();
        }
        if (c==54)
        {
            delay->freeze();
        }

        if (c==100)
        {
            delay->toggleOn();
        }

        if (c==212)
        {
            delay->toggleOn();
        }
        
    }
    delete delay;
}