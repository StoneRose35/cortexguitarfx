#include "audio/threebandeq.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"

/*
__DTCM_DATA
static SecondOrderIirFilterType filterLow={
    .coeffA = {-30948.0f/16384.0f, 14660.0f/16384.0f},
    .coeffB = {23.0f/16384.0f, 47.0f/16384.0f, 23.0f/16384.0f}
};

__DTCM_DATA
static SecondOrderIirFilterType filterMid={
    .coeffA = {-29699.0f/16384.0f, 13625.0f/16384.0f},
    .coeffB = {1378.0f/16384.0f, 0.0f, -1378.0f/16384.0f}
};

__DTCM_DATA
static SecondOrderIirFilterType filterHigh=
{
    .coeffA = {-26753.0f/16384.0f, 11314.0f/16384.0f},
    .coeffB = {13612.0f/16384.0f, -27225.0f/16384.0f, 13612.0f/16384.0f}
};
*/
__QSPI_CODE
void initThreeBandEq(ThreeBandEQType*data)
{
    initSecondOrderIirFilter(&data->lowShelf);
    initSecondOrderIirFilter(&data->midBand);
    initSecondOrderIirFilter(&data->highShelf);
    data->lowFactor = 0.0f;
    data->midFactor = 0.0f;
    data->highFactor = 0.0f;
}

/**
 * sets the low, mid and high factors given a single params
 * ranging from 0 to 4095
 * 0,: neutral (all factors 0)
 * 682 : bass cut (-1,0,0)
 * 1365 : bass and high cut, mid boost (-1,4,-1)
 * 2046 : high cut, (0,0,-1)
 * 2728 : mid cut, bass boost (4,-1,0)
 * 3410 : mid cut, high boost (0,-1,4)
 * 4095: all boost (4,4,4)
 */
__QSPI_CODE
void setSingleParam(uint16_t param, ThreeBandEQType*data)
{
    if (param < 682)
    {
        data->lowFactor = 0.0f-(float)param/682.0f;
        data->midFactor = 0.0f;
        data->highFactor = 0.0f;
    }
    else if (param < 1365)
    {
        data->lowFactor = -1.0f;
        data->midFactor =((float)param - 682.0f)/682.0f*4.0f;
        data->highFactor = -((float)param - 682.0f)/682.0f;
    }
    else if (param < 2046)
    {
        data->lowFactor = -1.0f + ((float)param - 1365.0f)/682.0f;
        data->midFactor = 4.0f - 4.0f*((float)param - 1365.0f)/682.0f;
        data->highFactor = -1.0f;
    }
    else if (param < 2728)
    {
        data->lowFactor = ((float)param - 2046.0f)/682.0f*4.0f;
        data->midFactor = -((float)param - 2046.0f)/682.0f;
        data->highFactor = -1.0f + ((float)param - 2046.0f)/682.0f;
    }
    else if (param < 3410)
    {
        data->lowFactor = 4.0f - ((float)param - 2728.0f)/682.0f*4.0f;
        data->midFactor = -1.0f;
        data->highFactor = ((float)param - 2728.0f)/682.0f*4.0f;
    }
    else
    {
        data->lowFactor = ((float)param - 3410.0f)/682.0f*4.0f;
        data->midFactor = -1.0f + 5.0f*((float)param - 3410.0f)/682.0f;
        data->highFactor = 4.0f;
    }
}

__ITCM_CODE
float threeBandEqProcessSample(float sampleIn,ThreeBandEQType*data)
{
    float lp, bp,hp, sampleOutw;
    lp = secondOrderIirFilterProcessSample(sampleIn,&data->lowShelf);
    bp = secondOrderIirFilterProcessSample(sampleIn,&data->midBand);
    hp = secondOrderIirFilterProcessSample(sampleIn,&data->highShelf);

    lp = (lp*data->lowFactor);
    bp = (bp*data->midFactor);
    hp = (hp*data->highFactor);

    sampleOutw = sampleIn + lp + bp + hp;
    return sampleOutw;
}

__QSPI_CODE
void threeBandEqReset(ThreeBandEQType*data)
{
    secondOrderIirFilterReset(&data->highShelf);
    secondOrderIirFilterReset(&data->lowShelf);
    secondOrderIirFilterReset(&data->midBand);
}