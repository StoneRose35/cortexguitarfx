#ifndef __QSPI_DATA
#define __QSPI_DATA
#endif
#ifndef __QSPI_CODE
#define __QSPI_CODE
#endif
#include "stdint.h"
#include "stdlib.h"
#include "./inc/bmplib.h"
#include "./../Inc/graphics/bwgraphics.h"
#include "./../Inc/images/pipicofx_param_2_scaled.h"
#include "./../Inc/images/pipicofx_param_1_scaled.h"
#include "./../Inc/fonts/oled_font_5x7.h"
#include "math.h"
#include "stdio.h"
#include "string.h"

#define F_MIN_VALUE 0.0f
#define F_MAX_VALUE 32.0f
#define F_VALUE 1500

void bmImageToBitmapStruct(BwImageTypeConst*bwImg,BitmapFileHeaderType*bmp)
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

int main(int argc,char** argv)
{
    float cx,cy,px,py,fValue;
    BwImageBufferType testImg;
    BwImageType imgPtr;
    BitmapFileHeaderType bmp;
    char *  testImageFilename = "out/screen%d.bmp";
    char fname[16];
    FILE * fid;



    imgPtr.sx = 128;
    imgPtr.sy = 64;

    imgPtr.data=(uint8_t*)malloc(1024);
    for (uint16_t c=0;c<32;c++)
    {
        
        for (uint8_t c=0;c<0xFF;c++)
        {
            *(((uint32_t*)imgPtr.data) + c)=0;
        }
        fValue = (float)c;
        imgPtr.sx=pipicofx_param_1_scaled_streamimg.sx;
        imgPtr.sy=pipicofx_param_1_scaled_streamimg.sy;
        imgPtr.type = 0;
        sprintf(fname,testImageFilename,c);

        for (uint16_t c=0;c<510;c++)
        {
            imgPtr.data[c]=pipicofx_param_1_scaled_streamimg.data[c];
        }

        fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - F_MIN_VALUE)/(F_MAX_VALUE - F_MIN_VALUE); //fValue is now an angle in radians from 45° to 315°
        // center is at 51/24
        px = 51.0f - fsin(fValue)*14.0f;
        py = 24.0f + fcos(fValue)*14.0f;
        cx = 51.0f;
        cy = 24.0f;
        drawLine(cx,cy,px,py,&imgPtr);
        bmImageToBitmapStruct((BwImageTypeConst*)&imgPtr,&bmp);

        writeBmp(fname,&bmp);
    }
    free(imgPtr.data);

    return 0;
}