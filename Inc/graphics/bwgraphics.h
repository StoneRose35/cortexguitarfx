#ifndef _BWGRAPHICS_H_
#define _BWGRAPHICS_H_
#include <stdint.h>

#ifndef RP2040_FEATHER
#include "math.h"
float fsqrt(float a);
int32_t float2int(float a);
float int2float(int32_t a);
float fcos(float x);
float fsin(float x);

#endif
typedef struct BwImageStruct
{
	uint8_t * data; // data is arrange y axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
} BwImageType;

typedef struct BwImageBufferStruct
{
	uint8_t data[1024]; // data is arrange y axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
} BwImageBufferType;


uint8_t getPixel(int32_t px,int32_t py,BwImageBufferType*img);
void drawLine(float spx,float spy,float epx, float epy,BwImageBufferType* img);
void setPixel(int32_t px,int32_t py,BwImageBufferType*img);
void clearPixel(int32_t px,int32_t py,BwImageBufferType*img);

void drawOval(float ax,float ay,float cx,float cy,BwImageBufferType*img);
void clearOval(float ax,float ay,float cx,float cy,BwImageBufferType*img);
void clearSquare(float spx, float spy,float epx, float epy,BwImageBufferType* img);
void drawSquare(float spx, float spy,float epx, float epy,BwImageBufferType* img);

#endif