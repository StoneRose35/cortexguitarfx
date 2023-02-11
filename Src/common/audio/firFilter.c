#include "audio/firFilter.h"
#include "audio/audiotools.h"
#ifdef RP2040_FEATHER
#include "multicore.h"
#include "gpio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
#endif

void initfirFilter(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0;
    }
}

void addSample(int16_t sampleIn,FirFilterType*data)
{
    *(data->delayBuffer + data->delayPointer--)=sampleIn;
    data->delayPointer &= (uint8_t)(data->filterLength-1);
}

int16_t firFilterProcessSample(int16_t sampleIn,FirFilterType*data)
{
    int32_t firstHalf,secondHalf;
    addSample(sampleIn,data);
    #ifdef RP2040_FEATHER
    while ((*SIO_FIFO_ST & ( 1 << SIO_FIFO_ST_RDY_LSB)) == 0);
    // send pointer to data to core1 to indicate that a fir calculation has to be made
    *SIO_FIFO_WR = (uint32_t)&data;
    secondHalf = processSecondHalf(data);
    // wait for core1 to be finished, core 1 sends the result of the second half back
    while ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB)) != (1 << SIO_FIFO_ST_VLD_LSB));
    firstHalf = *SIO_FIFO_RD;
    #else
    secondHalf = processSecondHalf(data);
    firstHalf = processFirstHalf(data);
    #endif
    firstHalf += secondHalf;
    firstHalf = clip(firstHalf);
    return (int16_t)firstHalf;
}


int32_t processFirstHalf(FirFilterType*data)
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
    return res;
}

int32_t processSecondHalf(FirFilterType*data)
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
    return res;
}