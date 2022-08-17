#include "stdint.h"
#include "./inc/bmplib.h"
#include "./../Inc/graphics/bwgraphics.h"
#include "./../Inc/images/pipicofx_param_2_scaled.h"
#include "./../Inc/images/pipicofx_param_1_scaled.h"
#include "math.h"
#include "stdio.h"
#include "string.h"

#define F_MIN_VALUE 0.0f
#define F_MAX_VALUE 32.0f
#define F_VALUE 1500


void bmImageToBitmapStruct(BwImageBufferType*bwImg,BitmapFileHeaderType*bmp)
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
    BitmapFileHeaderType bmp;
    char *  testImageFilename = "screen%d.bmp";
    char fname[16];
    FILE * fid;

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

    return 0;
}