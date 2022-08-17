#include "../inc/bmplib.h"
#include "stdlib.h"
#include "stdio.h"

#ifdef BBMP_LIB_STANDALONE
int main(int argc,char ** argv)
{
    BitmapFileHeaderType exampleBmp;
    BitmapFileHeaderType exampleReadBmp;
    uint16_t checkerLineCnt=0;
    uint16_t checkerCnt=0;
    initBmpFile(&exampleBmp,64,128);
    for (uint16_t y=0;y<64;y++)
    {
        for (uint16_t x=0;x<128;x++)
        {
            if ((checkerCnt & 0x1) == 0)
            {
                 bmpLibSetPixel(x,y,&exampleBmp);
            }
            else
            {
                bmpLibClearPixel(x,y,&exampleBmp);
            }
            if( (checkerLineCnt & 0x1) == 0)
            {
                if(x % 3 == 0)
                {
                    checkerCnt++;
                }
            }
            else
            {
                if(x % 4 == 0)
                {
                    checkerCnt++;
                }
            }
        }
        if ((checkerLineCnt & 0x1) == 0)
        {
            if(y%3==0)
            {
                checkerLineCnt++;
            }
        }
        else
        {
            if(y%4==0)
            {
                checkerLineCnt++;
            }
        }
    }
    writeBmp("./checkerExample.bmp",&exampleBmp);
    readBmpHeaders("./checkerExample1.bmp",&exampleReadBmp);
    return 0;
}
#endif

int initBmpFile(BitmapFileHeaderType * bitmapFileHeader,uint16_t height,uint16_t width)
{
    uint32_t rowSize = ((width+31) >> 5)*4;


    bitmapFileHeader->bmpHeader.size = 14 + 40 + 2*4 + height*rowSize ;
    bitmapFileHeader->bmpHeader.offset = 14 + 40 + 2*4; // header + dib + palette containing two colors
    bitmapFileHeader->bmpHeader.reserved1=0;
    bitmapFileHeader->bmpHeader.reserved2=0;
    
    bitmapFileHeader->dibHeader.headerSize = 40;
    bitmapFileHeader->dibHeader.width = width;
    bitmapFileHeader->dibHeader.height = height;
    bitmapFileHeader->dibHeader.nColorPlanes = 1;
    bitmapFileHeader->dibHeader.bitPerPixel = 1;
    bitmapFileHeader->dibHeader.compressionValue = 0;
    bitmapFileHeader->dibHeader.imageDataSize = 0;
    bitmapFileHeader->dibHeader.horizontalResolution = 11811;
    bitmapFileHeader->dibHeader.verticalResolution = 11811;
    bitmapFileHeader->dibHeader.colorPaletteSize = 2;
    bitmapFileHeader->dibHeader.importantColors = 2;

    // setup color palette
    bitmapFileHeader->colorPalette = (ColorAlphaType*)malloc(2*4);
    (bitmapFileHeader->colorPalette + 1)->alpha = 0x0;
    (bitmapFileHeader->colorPalette + 1)->r = 0xff;
    (bitmapFileHeader->colorPalette + 1)->g = 0xff;
    (bitmapFileHeader->colorPalette + 1)->b = 0xff;
    (bitmapFileHeader->colorPalette + 0)->alpha = 0x0;
    (bitmapFileHeader->colorPalette + 0)->r = 0x0;
    (bitmapFileHeader->colorPalette + 0)->g = 0x0;
    (bitmapFileHeader->colorPalette + 0)->b = 0x0;

    // setup image data
    bitmapFileHeader->imageData =  (uint8_t**)malloc(height*sizeof(uint8_t*));
    for (uint32_t c=0;c<height;c++)
    {
        *(bitmapFileHeader->imageData + c) = (uint8_t*)malloc(rowSize);
        for (uint16_t cc=0;cc<rowSize;cc++)
        {
            *(*(bitmapFileHeader->imageData + c)+cc) = 0x0;
        }
    }
    return 0;
}

/**
 * @brief Sets a pixel, with (0/0) being the upper left corner
 * 
 * @param x horizontal pixel position going from left to right
 * @param y vertical pixel position going from bottom to top
 * @param bmp the image to set the pixel to
 */
void bmpLibSetPixel(uint16_t x,uint16_t y,BitmapFileHeaderType*bmp)
{
    uint16_t xTransformed,yTransformed;
    xTransformed = x;
    yTransformed = bmp->dibHeader.height - y -1;
    bmp->imageData[yTransformed][x >> 3] |= (uint8_t)(1 << (0x7 - (x & 0x7)));
}

/**
 * @brief Clears a pixel, with (0/0) being the upper left corner
 * 
 * @param x horizontal pixel position going from left to right
 * @param y vertical pixel position going from bottom to top
 * @param bmp the image to set the pixel to
 */
void bmpLibClearPixel(uint16_t x,uint16_t y,BitmapFileHeaderType*bmp)
{
    uint16_t xTransformed,yTransformed;
    xTransformed = x;
    yTransformed = bmp->dibHeader.height - y - 1;
    *(*(bmp->imageData + yTransformed) + (x >> 3)) &= (uint8_t)(~(1 << (0x7 - (x & 0x7))));
}

void writeBmp(const char*filename, BitmapFileHeaderType*bmp)
{
    FILE*fid;
    const char magic[2]={'B', 'M'};
    uint32_t rowSize = ((bmp->dibHeader.width+31) >> 5)*4;
    if ((rowSize & 0x3) != 0)
    {
        rowSize += (4 - (rowSize & 0x3));
    }

    fid = fopen(filename,"wb");
    fwrite(magic,2,1,fid);
    fwrite(&(bmp->bmpHeader),12,1,fid);
    fwrite(&(bmp->dibHeader),40,1,fid);
    fwrite(bmp->colorPalette,4*2,1,fid);
    for (uint16_t cy=0;cy<bmp->dibHeader.height;cy++)
    {
        fwrite((*(bmp->imageData + cy)),rowSize,1,fid);
    }
    fclose(fid);
}

int readBmpHeaders(const char *filename,BitmapFileHeaderType*bmp)
{
    FILE*fid;
    uint8_t magic[2];
    fid = fopen(filename,"rb");
    fread(magic,2,1,fid);
    if (!(magic[0]=='B' && magic[1]=='M'))
    {
        fclose(fid);
        return 1;
    }
    fread(&(bmp->bmpHeader),12,1,fid);
    fread(&(bmp->dibHeader),40,1,fid);
    if(bmp->dibHeader.colorPaletteSize == 0)
    {
        switch (bmp->dibHeader.bitPerPixel)
        {
            case 8:
                bmp->colorPalette = (ColorAlphaType*)malloc(256*sizeof(ColorAlphaType));
                fread(bmp->colorPalette,256*sizeof(ColorAlphaType),1,fid);
                break;
            case 4:
                bmp->colorPalette = (ColorAlphaType*)malloc(16*sizeof(ColorAlphaType));
                fread(bmp->colorPalette,16*sizeof(ColorAlphaType),1,fid);
                break;
            case 2:
                bmp->colorPalette = (ColorAlphaType*)malloc(4*sizeof(ColorAlphaType));
                fread(bmp->colorPalette,4*sizeof(ColorAlphaType),1,fid);
                break;
            break;
            case 1:
                bmp->colorPalette = (ColorAlphaType*)malloc(2*sizeof(ColorAlphaType));
                fread(bmp->colorPalette,2*sizeof(ColorAlphaType),1,fid);
                break;
        }
    }
    else
    {
        bmp->colorPalette = (ColorAlphaType*)malloc(bmp->dibHeader.bitPerPixel*sizeof(ColorAlphaType));
        fread(bmp->colorPalette,(1 << bmp->dibHeader.bitPerPixel)*4,1,fid);
    }
    fclose(fid);
    return 0;
}
