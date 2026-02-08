#ifndef _GENERIC_DISTORTION_SIMPLE_H_
#define _GENERIC_DISTORTION_SIMPLE_H_
#include "stdint.h"

#define GDS_POINT_A 0 
#define GDS_POINT_B 1 

typedef struct 
{
    float slopeA; // section A maps x values from 0 to endA, it is a linear segment with slope slopeA and offset 0
    float endA;
    float offsetB;
    float endB;
    float slopeB; 
    float offsetC;
    float slopeC;
} GenericDistortionSimpleType;


void gdsGetPoint(uint8_t pointIdx,float*res,GenericDistortionSimpleType*data);
uint8_t gdsSetPoint(uint8_t pointIdx,float*point,GenericDistortionSimpleType*data); // point[0] is x coordinate, point[1] y coordinate
uint8_t gdsSetAllPoints(float*points,GenericDistortionSimpleType*data);

float gdsGetValue(float x,GenericDistortionSimpleType*data);
uint8_t gdsCheckData(GenericDistortionSimpleType*data); 
#endif