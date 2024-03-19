#include "audio/threebandeq.h"
#include "audio/audiotools.h"
static SecondOrderIirFilterType filterLow={
    .coeffA = {-30948.0f/16384.0f, 14660.0f/16384.0f},
    .coeffB = {23.0f/16384.0f, 47.0f/16384.0f, 23.0f/16384.0f}
};
static SecondOrderIirFilterType filterMid={
    .coeffA = {-29699.0f/16384.0f, 13625.0f/16384.0f},
    .coeffB = {1378.0f/16384.0f, 0.0f, -1378.0f/16384.0f}
};
static SecondOrderIirFilterType filterHigh=
{
    .coeffA = {-26753.0f/16384.0f, 11314.0f/16384.0f},
    .coeffB = {13612.0f/16384.0f, -27225.0f/16384.0f, 13612.0f/16384.0f}
};

__attribute__ ((section (".qspi_code")))
void initThreeBandEq(ThreeBandEQType*data)
{
    initSecondOrderIirFilter(&filterLow);
    initSecondOrderIirFilter(&filterMid);
    initSecondOrderIirFilter(&filterHigh);
    data->lowShelf = &filterLow;
    data->midBand = &filterMid;
    data->highShelf = &filterHigh;
    data->lowFactor = 0.0f;
    data->midFactor = 0.0f;
    data->highFactor = 0.0f;
}

__attribute__ ((section (".qspi_code")))
float threeBandEqProcessSample(float sampleIn,ThreeBandEQType*data)
{
    float lp, bp,hp, sampleOutw;
    lp = secondOrderIirFilterProcessSample(sampleIn,data->lowShelf);
    bp = secondOrderIirFilterProcessSample(sampleIn,data->midBand);
    hp = secondOrderIirFilterProcessSample(sampleIn,data->highShelf);

    lp = (lp*data->lowFactor);
    bp = (bp*data->midFactor);
    hp = (hp*data->highFactor);

    sampleOutw = sampleIn + lp + bp + hp;
    return sampleOutw;
}

__attribute__ ((section (".qspi_code")))
void threeBandEqReset(ThreeBandEQType*data)
{
    secondOrderIirFilterReset(data->highShelf);
    secondOrderIirFilterReset(data->lowShelf);
    secondOrderIirFilterReset(data->midBand);
}