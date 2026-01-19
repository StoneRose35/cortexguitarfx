#include "audio/genericDistortion.h"
#include "matrixMath.h"
#include "memoryRegions.h"


__ITCM_CODE
float gdGetValue(float x,GenericDistortionType*data)
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
        float xi = -x;
        if (xi < data->endA)
        {
            return -data->slopeA*xi;
        }
        else if (xi < data->endB)
        {
            return -(data->coeffsB[0] + data->coeffsB[1]*xi + data->coeffsB[2]*xi*xi + data->coeffsB[3]*xi*xi*xi);
        }
        else if ( xi < data->endC)
        {
            return -((xi - data->endB)*data->slopeC + data->offsetC);
        }
        else if ( xi < data->endD)
        {
            return -(data->coeffsD[0] + data->coeffsD[1]*xi + data->coeffsD[2]*xi*xi + data->coeffsD[3]*xi*xi*xi);
        }
        else
        {
            return -((xi - data->endD)*(1.0f - data->offsetE)/(1.0f - data->endD) + data->offsetE);
        }
    }
    if (x < data->endA)
    {
        return data->slopeA*x;
    }
    else if ( x < data->endB)
    {
        return data->coeffsB[0] + data->coeffsB[1]*x + data->coeffsB[2]*x*x + data->coeffsB[3]*x*x*x;
    }
    else if ( x < data->endC)
    {
        return (x - data->endB)*data->slopeC + data->offsetC;
    }
    else if ( x < data->endD)
    {
        return data->coeffsD[0] + data->coeffsD[1]*x + data->coeffsD[2]*x*x + data->coeffsD[3]*x*x*x;
    }
    else
    {
        return (x - data->endD)*(1.0f - data->offsetE)/(1.0f - data->endD) + data->offsetE;
    }
}

void gdGetPoint(uint8_t pointIdx,float*res,GenericDistortionType*data)
{
    switch(pointIdx)
    {
        case GD_POINT_A:
            res[0] = data->endA;
            res[1]= data->slopeA*data->endA;
            break;
        case GD_POINT_B:
            res[0]=data->endB;
            res[1]=data->offsetC;
            break;
        case GD_POINT_C:
            res[0] = data->endC;
            res[1] = (data->endC - data->endB)*data->slopeC + data->offsetC;
            break;
        case GD_POINT_D:
            res[0] = data->endD;
            res[1] = data->offsetE;
            break;
        default:
            break;
    }
}

uint8_t gdSetPoint(uint8_t pointIdx,float*point,GenericDistortionType*data)
{
    float endCY ;
    switch(pointIdx)
    {
        case GD_POINT_A:
            data->slopeA = point[1]/point[0];
            data->endA = point[0];
            return gdCalculateSplines(data);
        case GD_POINT_B:
            endCY = (data->endC - data->endB)*data->slopeC + data->offsetC; 
            data->endB = point[0];
            data->slopeC = (endCY - point[1])/(data->endC - data->endB);
            data->offsetC = point[1];
            return gdCalculateSplines(data);
        case GD_POINT_C:
            data->endC =point[0];
            data->slopeC = (point[1] - data->offsetC)/(data->endC-data->endB);
            return gdCalculateSplines(data);
        case GD_POINT_D:
            data->offsetE = point[1];
            data->endD = point[0];
            return gdCalculateSplines(data);
        default:
            break;
    }
    return 0;
}
/*
sets all points at once, should be called when initializing the distortion curve
points is a matrix, first index is the point number, second index is coords as x, y
so points[2][1] is the y coordinate of the third point
*/
uint8_t gdSetAllPoints(float*points,GenericDistortionType*data)
{
    data->slopeA = points[1]/points[0];
    data->endA = points[0];
    data->endB = points[2];
    data->offsetC = points[3];
    data->endC = points[4];
    data->offsetE = points[7];
    data->endD = points[6];
    data->slopeC = (points[5] - data->offsetC)/(data->endC-data->endB);
    return gdCalculateSplines(data);
}
uint8_t gdCalculateSplines(GenericDistortionType*data)
{
    Mat4x4Type a,ainv;
    float q[4];
    uint8_t status = 0, retstatus = 0;
    // solving a*coeffs=q -> coeffs = a^-1 *q
    a.mat[0][0] = 1.0f;
    a.mat[0][1] = data->endA;
    a.mat[0][2] = data->endA*data->endA;
    a.mat[0][3] = data->endA*data->endA*data->endA;
    a.mat[1][0] = 0.0f;
    a.mat[1][1] = data->endA;
    a.mat[1][2] = 2.0f*data->endA;
    a.mat[1][3] = 3.0f*data->endA*data->endA;
    a.mat[2][0] = 1.0f;
    a.mat[2][1] = data->endB;
    a.mat[2][2] = data->endB*data->endB;
    a.mat[2][3] = data->endB*data->endB*data->endB;
    a.mat[3][0] = 0.0f;
    a.mat[3][1] = data->endB;
    a.mat[3][2] = 2.0f*data->endB;
    a.mat[3][3] = 3.0f*data->endB*data->endB;
    q[0] = data->slopeA*data->endA; // y[endA]
    q[1] = data->slopeA; // y'[endA]
    q[2] = data->offsetC;// y[endB]
    q[3] = data->slopeC; // y'[endB]
    status = minv(&a,&ainv);
    if (status == 0)
    {
        data->coeffsB[0]= ainv.mat[0][0]*q[0] + ainv.mat[0][1]*q[1] + ainv.mat[0][2]*q[2] + ainv.mat[0][3]*q[3];
        data->coeffsB[1]= ainv.mat[1][0]*q[0] + ainv.mat[1][1]*q[1] + ainv.mat[1][2]*q[2] + ainv.mat[1][3]*q[3];
        data->coeffsB[2]= ainv.mat[2][0]*q[0] + ainv.mat[2][1]*q[1] + ainv.mat[2][2]*q[2] + ainv.mat[2][3]*q[3];
        data->coeffsB[3]= ainv.mat[3][0]*q[0] + ainv.mat[3][1]*q[1] + ainv.mat[3][2]*q[2] + ainv.mat[3][3]*q[3];
    }
    retstatus += status;


    a.mat[0][0] = 1.0f;
    a.mat[0][1] = data->endC;
    a.mat[0][2] = data->endC*data->endC;
    a.mat[0][3] = data->endC*data->endC*data->endC;
    a.mat[1][0] = 0.0f;
    a.mat[1][1] = data->endC;
    a.mat[1][2] = 2.0f*data->endC;
    a.mat[1][3] = 3.0f*data->endC*data->endC;
    a.mat[2][0] = 1.0f;
    a.mat[2][1] = data->endD;
    a.mat[2][2] = data->endD*data->endD;
    a.mat[2][3] = data->endD*data->endD*data->endD;
    a.mat[3][0] = 0.0f;
    a.mat[3][1] = data->endD;
    a.mat[3][2] = 2.0f*data->endD;
    a.mat[3][3] = 3.0f*data->endD*data->endD;
    q[0] = (data->endC - data->endB)*data->slopeC + data->offsetC ; // y[endC]
    q[1] = data->slopeC; // y'[endC]
    q[2] = data->offsetE;// y[endD]
    q[3] = (1.0f - data->offsetE)/(1.0f - data->endD); // y'[endD]
    status = minv(&a,&ainv);
    if (status == 0)
    {
        data->coeffsD[0]= ainv.mat[0][0]*q[0] + ainv.mat[0][1]*q[1] + ainv.mat[0][2]*q[2] + ainv.mat[0][3]*q[3];
        data->coeffsD[1]= ainv.mat[1][0]*q[0] + ainv.mat[1][1]*q[1] + ainv.mat[1][2]*q[2] + ainv.mat[1][3]*q[3];
        data->coeffsD[2]= ainv.mat[2][0]*q[0] + ainv.mat[2][1]*q[1] + ainv.mat[2][2]*q[2] + ainv.mat[2][3]*q[3];
        data->coeffsD[3]= ainv.mat[3][0]*q[0] + ainv.mat[3][1]*q[1] + ainv.mat[3][2]*q[2] + ainv.mat[3][3]*q[3];
    }
    retstatus += status;

    return retstatus;
}


uint8_t gdCheckData(GenericDistortionType*data)
{
    uint8_t res=0;

    if (data->endA <= data->endB && data->endB <= data->endC && data->endC <= data->endD)
    {
        res |= (1 << GD_FUNCTION_POS);
    }
    if (data->slopeA > 0.0f && data->slopeC > 0.0f && (1.0f - data->offsetE)/(1.0f - data->endD) > 0.0f)
    {
        res |= (1 << GD_MONOTONICALLY_POS);
    }
    return res;
}