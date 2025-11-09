#include "stdint.h"
#include "./inc/bmplib.h"
#include "./../Inc/graphics/bwgraphics.h"
#include "./../Inc/fonts/GFXfonts.h"
#include "./../Inc/images/pipicofx_param_2_scaled.h"
#include "./../Inc/images/pipicofx_param_1_scaled.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#define F_MIN_VALUE 0.0f
#define F_MAX_VALUE 32.0f
#define F_VALUE 1500


void bmImageToBitmapStruct(BwImageType*bwImg,BitmapFileHeaderType*bmp)
{
    uint8_t pixelVal;
    initBmpFile(bmp,bwImg->sy,bwImg->sx);
    for (uint16_t cy=0;cy<bmp->dibHeader.height;cy++)
    {
        for(uint16_t cx=0;cx<bmp->dibHeader.width;cx++)
        {
            pixelVal = getPixel(cx,cy,bwImg);
            if (pixelVal != 0)
            {
                bmpLibSetPixel(cx,cy,bmp);
            } 
            else
            {
                bmpLibClearPixel(cx,cy,bmp);
            }
        }
    }
}

void fontTest(void)
{
    BwImageType testImg;
    BitmapFileHeaderType bmp;
    const char testtext[]="Hallo";
    char *  testImageFilename = "./out/screen0.bmp";
    testImg.sx=128;
    testImg.sy=64;
    testImg.data=(uint8_t*)malloc(1024);
    for (uint8_t c=0;c<0xFF;c++)
    {
        *(((uint32_t*)testImg.data) + c)=0;
    }
    drawText(12,15,testtext,(BwImageType*)&testImg,&FreeMono12pt7b);
    bmImageToBitmapStruct(&testImg,&bmp);
    writeBmp(testImageFilename,&bmp);
    free(testImg.data);
}

void gfxFontDemo(void)
{
    BwImageType imgPtr;
    BitmapFileHeaderType bmp;
    const char testtext[]="Megadeth";
    char fname[32];
    imgPtr.sx=128;
    imgPtr.sy=64;
    imgPtr.type = BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES;
    imgPtr.data=(uint8_t*)malloc(1024);
    for (uint8_t f=0;f<(sizeof(gfxfonts)/sizeof(GFXfont*));f++)
    {
        for (uint8_t c=0;c<0xFF;c++)
        {
            *(((uint32_t*)imgPtr.data) + c)=0;
        }
        sprintf(fname,"./out/%s.bmp",gfxfontNames[f]);
        drawText(0,32,testtext,&imgPtr,gfxfonts[f]);
        bmImageToBitmapStruct(&imgPtr,&bmp);
        writeBmp(fname,&bmp);
    }
    free(imgPtr.data);
}

void potentiometerImageTest()
{
    float cx,cy,px,py,fValue;
    BwImageType testImg;
    BitmapFileHeaderType bmp;
    char *  testImageFilename = "./out/screen%d.bmp";
    char fname[16];
    FILE * fid;
    testImg.sx = 128;
    testImg.sy = 64;
    testImg.type = BWIMAGE_BW_IMAGE_STRUCT_HORIZONTAL_BYTES;
    testImg.data = (uint8_t)malloc(1024);
    for (uint16_t c=0;c<32;c++)
    {
        fValue = (float)c;
        testImg.sx=pipicofx_param_1_scaled_streamimg.sx;
        testImg.sy=pipicofx_param_1_scaled_streamimg.sy;
        sprintf(fname,testImageFilename,c);

        
        for (uint16_t c=0;c<510;c++)
        {
            testImg.data[c]=pipicofx_param_1_scaled_streamimg.data[c];
        }
        

        fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - F_MIN_VALUE)/(F_MAX_VALUE - F_MIN_VALUE); //fValue is now an angle in radians from 45° to 315°
        // center is at 51/24
        px = 51.0f - fsin(fValue)*14.0f;
        py = 24.0f + fcos(fValue)*14.0f;
        cx = 51.0f;
        cy = 24.0f;
        drawLine(cx,cy,px,py,&testImg);
        bmImageToBitmapStruct(&testImg,&bmp);

        writeBmp(fname,&bmp);
    }
    free(testImg.data);
}

int main(int argc,char** argv)
{

    gfxFontDemo();
    return 0;
}