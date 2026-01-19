#ifndef _GENERIC_DISTORTION_H_
#define _GENERIC_DISTORTION_H_
#include "stdint.h"

#define GD_FUNCTION_POS 0
#define GD_MONOTONICALLY_POS 1

#define GD_POINT_A 0 
#define GD_POINT_B 1 
#define GD_POINT_C 2 
#define GD_POINT_D 3 
/*
   |
   |
   |
   |
   |
   |
   |
   |
   |
   |                                      x
   |                              E   x  
   |                      D       x   
   |                          x
   |                       x  
   |                     x 
   |                    x
   |                   x
   |               C  x
   |                 x
   |                x
   |               x 
   |     A     B  x
   |            x
   |          x  
   |     x  
   |x________________________________________________________________________________________________________________
   A: linear section from (0/0) to pointA
   B: third order polynomial from pointA t pointB
   C: linear section from pointB to pointC
   D: third order polynomial from pointC to pointD
   E: linear section from pointD to (1/1)
*/
typedef struct 

{
    float slopeA; // section A maps x values from 0 to endA, it is a linear segment with slope slopeA and offset 0
    float endA;
    float endB;
    float coeffsB[4]; // coefficients for the spline segmente from endA to endB, y = coeffs[0] +coeffs[1]*x + coeffs[2]*x*x + coeffs[3]*x*x*x
    float endC;
    float slopeC; // section C maps values from endB to endC, it is a linear segment with slope slopeC and an offset offsetC, y= (x-endB)*slopeC + offsetC
    float offsetC; // offsetC is generally larger than slopeA*endA, (endC-endB)*slopeC + offsetC must be smaller of equal 1
    float endD;
    float coeffsD[4]; // section D is a spline segment mapping segment C to E
    float offsetE; // section E is the last linear segment mapping values from endD to 1, y = (x-endD)*(1-offsetE)/(1-endD) + offsetE
} GenericDistortionType;

void gdGetPoint(uint8_t pointIdx,float*res,GenericDistortionType*data);
uint8_t gdSetPoint(uint8_t pointIdx,float*point,GenericDistortionType*data); // point[0] is x coordinate, point[1] y coordinate
uint8_t gdSetAllPoints(float*points,GenericDistortionType*data);// points[0] is point a_x, points[1] is a_y, points[2] point b_x etc.

float gdGetValue(float x,GenericDistortionType*data);
uint8_t gdCalculateSplines(GenericDistortionType*data); // used to calculate the spline coefficients when slopes, offset or ends change
uint8_t gdCheckData(GenericDistortionType*data); // checks whether data data represent a function, and if so: if the function is monotonically rising


#endif