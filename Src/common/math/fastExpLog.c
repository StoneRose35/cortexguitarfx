#include "fastExpLog.h"

int16_t fastlog(int16_t  x)
{
    return *(logtable + (uint16_t)x);
}


int16_t fastexp(int16_t x)
{
    return *(exptable + (uint16_t)x);
}


/**
 * @brief display the current logarithm value in db as fixed point with one decimal, so -816 is -81.6
 *        the value 0 is mapped to -999.9
 * @param x logarithm value on 32768 + 7256.64*log10(x) mapping  x from 1/32768 to 32767/32768 to 0 to 32767
 * @return int16_t value in db
 */
int16_t asDb(int16_t x)
{
    int32_t interm;
    if (x==0)
    {
        return -9999;
    }
    else
    {
        interm = (x-1)*903;
        interm >>= 15;
        return (int16_t)interm;
    }
}