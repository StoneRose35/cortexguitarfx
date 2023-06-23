#ifndef _BWGRAPHICS_H_
#define _BWGRAPHICS_H_
#include <stdint.h>
#include "graphics/gfxfont.h"
#ifndef RP2040_FEATHER
#include "math.h"
float fsqrt(float a);
int32_t float2int(float a);
float int2float(int32_t a);
float fcos(float x);
float fsin(float x);
#endif

#define BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES 0
#define BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES 1
typedef struct BwImageStruct
{
	uint8_t * data; // data is arrange x axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
	uint8_t type;
} BwImageType;

typedef struct BwImageBufferStruct
{
	uint8_t data[1024]; // data is arrange x axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
} BwImageBufferType;


uint8_t getPixel(int32_t px,int32_t py,const BwImageType*img);
void drawLine(float spx,float spy,float epx, float epy,BwImageType* img);
void clearLine(float spx,float spy,float epx, float epy,BwImageType* img);
void setPixel(int32_t px,int32_t py,BwImageType*img);
void clearPixel(int32_t px,int32_t py,BwImageType*img);

void drawOval(float ax,float ay,float cx,float cy,BwImageType*img);
void clearOval(float ax,float ay,float cx,float cy,BwImageType*img);
void clearSquare(float spx, float spy,float epx, float epy,BwImageType* img);
void drawSquare(float spx, float spy,float epx, float epy,BwImageType* img);
void drawHorizontal(uint8_t yval,int8_t sx, int8_t ex, BwImageType*img);
void clearHorizontal(uint8_t yval,int8_t sx, int8_t ex, BwImageType*img);
void drawVertical(uint8_t xval,int8_t sy, int8_t ey, BwImageType*img);
void clearVertical(uint8_t xval,int8_t sy, int8_t ey, BwImageType*img);
uint8_t drawChar(uint8_t px, uint8_t py, char c,BwImageType* img,const void* font);
uint8_t drawCharGFXFont(uint8_t px, uint8_t py, char c,BwImageType* img,const GFXfont* font);
uint8_t drawCharOLedFont(uint8_t px, uint8_t py,char c, BwImageType* img);
void drawText(uint8_t px, uint8_t py,const char * txt,BwImageType* img,const void* font);
void drawImage(uint8_t px, uint8_t py,const BwImageType * img, BwImageType* imgBuffer);
void clearImage(BwImageType*img);
#endif