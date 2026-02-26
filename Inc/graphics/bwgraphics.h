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

#ifndef __QSPI_CODE
#define __QSPI_CODE
#endif

#define BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES 0
#define BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES 1

#define BWIMAGE_FRAMEBUFFER_HORIZONTAL
//#define BWIMAGE_FRAMEBUFFER_VERTICAL
typedef struct BwImageStruct
{
	uint8_t * data; // data is arrange x axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
	uint8_t type; // either WIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES if bytes run along the vertical axis, or 
	              // BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES
	uint16_t byteSize; //size in bytes as calculated by sx*sy/8, used to avoid writing outside the buffer
} BwImageType;

typedef struct BwImageStructConst
{
	const uint8_t * data; // data is arrange x axis first, starting from top left
	const uint8_t sx; // size in pixels
	const uint8_t sy; // size in pixels
	const uint8_t type; // either WIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES if bytes run along the vertical axis, or 
	              // BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES
} BwImageTypeConst;

typedef struct BwImageBufferStruct
{
	uint8_t data[1024]; // data is arrange x axis first, starting from top left
	uint8_t sx; // size in pixels
	uint8_t sy; // size in pixels
	uint8_t type;
} BwImageBufferType;


__QSPI_CODE
uint8_t getPixel(int32_t px,int32_t py,const BwImageTypeConst*img);
__QSPI_CODE
void drawLineFloat(float spx,float spy,float epx, float epy,BwImageType* img);
__QSPI_CODE
void clearLineFloat(float spx,float spy,float epx, float epy,BwImageType* img);
__QSPI_CODE
void setPixel(int32_t px,int32_t py,BwImageType*img);
__QSPI_CODE
void clearPixel(int32_t px,int32_t py,BwImageType*img);

__QSPI_CODE
void drawOval(float ax,float ay,float cx,float cy,BwImageType*img);
__QSPI_CODE
void clearOval(float ax,float ay,float cx,float cy,BwImageType*img);
__QSPI_CODE
void clearSquare(float spx, float spy,float epx, float epy,BwImageType* img);
__QSPI_CODE
void drawSquare(float spx, float spy,float epx, float epy,BwImageType* img);
__QSPI_CODE
void clearSquareInt(uint8_t spx, uint8_t spy,uint8_t epx, uint8_t epy,BwImageType* img);
__QSPI_CODE
void drawSquareInt(uint8_t spx, uint8_t spy,uint8_t epx, uint8_t epy,BwImageType* img);
__QSPI_CODE
void drawHorizontal(uint8_t yval,int8_t sx, int8_t ex, BwImageType*img);
__QSPI_CODE
void clearHorizontal(uint8_t yval,int8_t sx, int8_t ex, BwImageType*img);
__QSPI_CODE
void drawVertical(uint8_t xval,int8_t sy, int8_t ey, BwImageType*img);
__QSPI_CODE
void clearVertical(uint8_t xval,int8_t sy, int8_t ey, BwImageType*img);
__QSPI_CODE
void drawLine(uint8_t xstart, uint8_t ystart,uint8_t xend, uint8_t yend,BwImageType*img);
__QSPI_CODE
void clearLine(uint8_t xstart,uint8_t xend, uint8_t ystart, uint8_t yend,BwImageType*img);
__QSPI_CODE
uint8_t drawChar(uint8_t px, uint8_t py, char c,BwImageType* img,const void* font);
__QSPI_CODE
uint8_t drawCharGFXFont(uint8_t px, uint8_t py, char c,BwImageType* img,const GFXfont* font);
__QSPI_CODE
uint8_t drawCharOLedFont(uint8_t px, uint8_t py,char c, BwImageType* img);
__QSPI_CODE
void drawText(uint8_t px, uint8_t py,const char * txt,BwImageType* img,const void* font);
__QSPI_CODE
void drawImage(uint8_t px, uint8_t py,const BwImageTypeConst * img, BwImageType* imgBuffer);
__QSPI_CODE
void clearImage(BwImageType*img);
__QSPI_CODE
void drawRectFrame(uint8_t xstart,uint8_t ystart,uint8_t xend,uint8_t yend,BwImageType*img);
__QSPI_CODE
void clearRectFrame(uint8_t xstart,uint8_t ystart,uint8_t xend,uint8_t yend,BwImageType*img);
#endif