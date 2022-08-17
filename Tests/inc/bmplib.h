#ifndef _BMPLIB_H_
#define _BMPLIB_H_
#include "stdint.h"

typedef struct {
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BmpHeaderType;

typedef struct {
    uint32_t headerSize; // 40 as  default
    int32_t width;
    int32_t height;
    uint16_t nColorPlanes; // 1 as default
    uint16_t bitPerPixel; // 1 as default
    uint32_t compressionValue; // 0 as default
    uint32_t imageDataSize; // dummy 0 for uncompressed images
    int32_t horizontalResolution; // pixels per meter
    int32_t verticalResolution; // pixels per meter
    uint32_t colorPaletteSize; // number of colors in the palettem 0 for 2^pixelsize
    uint32_t importantColors; // 0 by default
} DibHeaderType;

typedef struct 
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColorType;


typedef struct 
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t alpha;
} ColorAlphaType;

typedef struct {
    BmpHeaderType bmpHeader;
    DibHeaderType dibHeader;
    ColorAlphaType * colorPalette; 
    uint8_t ** imageData;
} BitmapFileHeaderType;

int initBmpFile(BitmapFileHeaderType * bitmapFileHeader,uint16_t height,uint16_t width);
void bmpLibSetPixel(uint16_t x,uint16_t y,BitmapFileHeaderType*bmp);
void bmpLibClearPixel(uint16_t x,uint16_t y,BitmapFileHeaderType*bmp);
void writeBmp(const char*filename, BitmapFileHeaderType*bmp);
int readBmpHeaders(const char *filename,BitmapFileHeaderType*bmp);

#endif