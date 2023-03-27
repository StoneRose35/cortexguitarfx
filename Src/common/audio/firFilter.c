#include "audio/firFilter.h"
#ifdef RP2040_FEATHER
#include "multicore.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
#endif

#ifdef STM32
#include "stm32h750/helpers.h"
#else
float convolve(const float * coefficients,float*data,uint32_t offset)
{
    float res=0.0f;
    uint32_t dataIndex=offset;
    for(uint32_t c=0;c<64;c++)
    {
        res += coefficients[c]*data[dataIndex];
        dataIndex++;
        dataIndex &= 63;
    }
    return res;
}
#endif
#ifndef FLOAT_AUDIO


void addSample(int16_t sampleIn,FirFilterType*data)
{
    *(data->delayBuffer + data->delayPointer--)=sampleIn;
    data->delayPointer &= (uint8_t)(data->filterLength-1);
}

int16_t firFilterProcessSample(int16_t sampleIn,FirFilterType*data)
{
    volatile int16_t firstHalf,secondHalf;
    addSample(sampleIn,data);
    #ifdef RP2040_FEATHER
    while ((*SIO_FIFO_ST & ( 1 << SIO_FIFO_ST_RDY_LSB)) == 0);
    // send pointer to data to core1 to indicate that a fir calculation has to be made
    *SIO_FIFO_WR = (uint32_t)&data;
    secondHalf = processSecondHalf(data);
    // wait for core1 to be finished, core 1 sends the result of the second half back
    while ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB)) != (1 << SIO_FIFO_ST_VLD_LSB));
    firstHalf = (int16_t)(*SIO_FIFO_RD & 0xFFFF);
    #else
    secondHalf = processSecondHalf(data);
    firstHalf = processFirstHalf(data);
    #endif
    firstHalf += secondHalf;
    //firstHalf=secondHalf;
    return firstHalf;
}

int16_t processFirstHalf(FirFilterType*data)
{
    int32_t res=0;
    uint8_t runningPtr=(data->delayPointer+1) & (data->filterLength-1);
    for(uint8_t c=0;c<(data->filterLength >> 1);c++)
    {
        res += ((uint32_t)data->delayBuffer[runningPtr]*(uint32_t)data->coefficients[c]);
        runningPtr += 1;
        runningPtr &= (data->filterLength-1);
    }
    res >>= 15;
    return (int16_t)res;
}

int16_t processSecondHalf(FirFilterType*data)
{
    int32_t res=0;
    uint8_t runningPtr=(data->delayPointer+1+(data->filterLength>>1)) & (data->filterLength-1);
    for(uint8_t c=0;c<(data->filterLength >> 1);c++)
    {
        res += ((uint32_t)data->delayBuffer[runningPtr]*(uint32_t)data->coefficients[c+(data->filterLength >> 1)]);
        runningPtr += 1;
        runningPtr &= (data->filterLength-1);
    }
    res >>= 15;
    return (int16_t)res;
}

void initfirFilter(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0;
    }
}
#else

void addSample(float sampleIn,FirFilterType*data)
{
    data->delayPointer--;
    data->delayPointer &= (uint8_t)(data->filterLength-1);
    *(data->delayBuffer + data->delayPointer)=sampleIn;
}

float processFirstHalf(FirFilterType*data)
{
    //convolve(data->coefficients,data->delayBuffer,data->delayPointer);
    return 0.0f;
}

float processSecondHalf(FirFilterType*data)
{
    //convolve(data->coefficients,data->delayBuffer,(data->delayPointer + (data->filterLength >> 1)) & (data->filterLength-1));
    return 0.0f;
}

__attribute__ ((section (".qspi_code")))
float firFilterProcessSample(float sampleIn,FirFilterType*data)
{
    addSample(sampleIn,data);
    float res;
    res = convolve(data->coefficients,data->delayBuffer,data->delayPointer);
    return res;
}

void initfirFilter(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
}
#endif


