#include "audio/genericDistortionSimple.h"

void gdsGetPoint(uint8_t pointIdx,float*res,GenericDistortionSimpleType*data)
{
    switch (pointIdx)
    {
    case GDS_POINT_A:
        res[0]=data->endA;
        res[1]=data->slopeA*data->endA;
        break;
    case GDS_POINT_B:
        res[0]=data->endB;
        res[1]=data->offsetB + data->slopeB*data->endB;
        break;
    default:
        break;
    }
}

uint8_t gdsSetPoint(uint8_t pointIdx,float*point,GenericDistortionSimpleType*data) // point[0] is x coordinate, point[1] y coordinate
{
    switch (pointIdx)
    {
    case GDS_POINT_A:
        data->endA=point[0];
        data->slopeA = point[1]/point[0];
        break;
    case GDS_POINT_B:
        data->endB = point[0];
        data->slopeB = (point[1] - data->slopeA*data->endA)/(data->endB - data->endA);
        data->offsetB = -data->slopeB*data->endA + data->slopeA*data->endA;
        data->slopeC = (1.0f - (data->endB*data->slopeB+data->offsetB))/(1.0f-data->endB);
        data->offsetC = data->endB*data->slopeB + data->offsetB - data->endB*data->slopeC; 
        break;
    default:
        break;
    }
    return 0;
}


uint8_t gdsSetAllPoints(float*points,GenericDistortionSimpleType*data)
{
    gdsSetPoint(GDS_POINT_A,points,data);
    gdsSetPoint(GDS_POINT_B,points+2,data);
    return 0;
}

float gdsGetValue(float x,GenericDistortionSimpleType*data)
{
    if (x > 1.0f)
    {
        return 1.0f;
    }
    if (x < -1.0f)
    {
        return -1.0f;
    }
    if (x < 0.0f)
    {
        float xi=-x;
        if (xi < data->endA)
        {
            return -xi*data->slopeA;
        }
        if (xi < data->endB)
        {
            return -(xi*data->slopeB + data->offsetB);
        }
        return -(xi*data->slopeC + data->offsetC);
    }
    if (x < data->endA)
    {
        return x*data->slopeA;
    }
    if (x < data->endB)
    {
        return (x*data->slopeB + data->offsetB);
    }
    return (x*data->slopeC + data->offsetC);
}


uint8_t gdsCheckData(GenericDistortionSimpleType*data)
{
    float ptA = gdsGetValue(data->endA,data);
    float ptB = gdsGetValue(data->endB,data);
    return 
    ptA <= 1.0f &&
    ptA >= 0.0f &&
    ptB <= 1.0f &&
    ptB >= 0.0f;
}