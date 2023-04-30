#include "audio/threebandeq.h"
#include "audio/audiotools.h"
static SecondOrderIirFilterType filterLow={
    .coeffA = {-30948, 14660},
    .coeffB = {23, 47, 23}
};
static SecondOrderIirFilterType filterMid={
    .coeffA = {-29699, 13625},
    .coeffB = {1378, 0, -1378}
};
static SecondOrderIirFilterType filterHigh=
{
    .coeffA = {-26753, 11314},
    .coeffB = {13612, -27225, 13612}
};

void initThreeBandEq(ThreeBandEQType*data)
{
    initSecondOrderIirFilter(&filterLow);
    initSecondOrderIirFilter(&filterMid);
    initSecondOrderIirFilter(&filterHigh);
    data->lowShelf = &filterLow;
    data->midBand = &filterMid;
    data->highShelf = &filterHigh;
    data->lowFactor = 0;
    data->midFactor = 0;
    data->highFactor = 0;
}

int16_t threeBandEqProcessSample(int16_t sampleIn,ThreeBandEQType*data)
{
    int32_t lp, bp,hp, sampleOutw;
    volatile uint32_t * audioState = getAudioStatePtr(); 
    lp = secondOrderIirFilterProcessSample(sampleIn,data->lowShelf);
    bp = secondOrderIirFilterProcessSample(sampleIn,data->midBand);
    hp = secondOrderIirFilterProcessSample(sampleIn,data->highShelf);

    lp = ((lp*data->lowFactor) >> 12);
    bp = ((bp*data->midFactor) >> 12);
    hp = ((hp*data->highFactor) >> 12);

    sampleOutw = clip((int32_t)sampleIn + lp + bp + hp,audioState);
    return (int16_t)sampleOutw;
}

void threeBandEqReset(ThreeBandEQType*data)
{
    secondOrderIirFilterReset(data->highShelf);
    secondOrderIirFilterReset(data->lowShelf);
    secondOrderIirFilterReset(data->midBand);
}