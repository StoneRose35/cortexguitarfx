extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "pipicofx/pipicofxui.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/019_XAmp.hpp"
#include "audio/genericDistortion.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
}

#define OFFSET_DISTORTION_GRAPH 60

#define OD_EDIT_POINT01_POS_X 0 
#define OD_EDIT_POINT01_POS_Y 1
#define OD_EDIT_POINT01_SPREAD_X 2 
#define OD_EDIT_POINT01_SPREAD_Y 3

#define OD_EDIT_POINT23_POS_X 4 
#define OD_EDIT_POINT23_POS_Y 5
#define OD_EDIT_POINT23_SPREAD_X 6 
#define OD_EDIT_POINT23_SPREAD_Y 7

#define OD_MODE_SELECTING 0 
#define OD_MODE_EDITING 1

#define ENCODER_DELTA_SCALE 0.003f
static  GenericDistortionType * distortion=(GenericDistortionType*)0;
static volatile uint8_t editType = OD_EDIT_POINT01_POS_X;
static volatile uint8_t selectionMode = OD_MODE_SELECTING;

static void create(PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    if (stringEquals(data->currentProgram->getName(),"XAmp")==1)
    {
        PiPicoFX::XAmp::XAmp *xamp = static_cast<PiPicoFX::XAmp::XAmp*>(data->currentProgram);
        distortion = &xamp->distortion;
    }
    clearImage(imgBuffer);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    uint8_t oldy=0;
    clearSquareInt(0,0,128,64,imgBuffer);
    if (distortion==(GenericDistortionType*)0)
    {
        drawText(4,32,"NPE: distortion",imgBuffer,(void*)0);
        return;
    }
    // vertical axis
    drawVertical(OFFSET_DISTORTION_GRAPH+1,0,63,imgBuffer);
    setPixel((OFFSET_DISTORTION_GRAPH),1,imgBuffer);
    setPixel((OFFSET_DISTORTION_GRAPH+2),1,imgBuffer);
    
    // horizontal axis
    drawHorizontal(62,OFFSET_DISTORTION_GRAPH,127,imgBuffer);
    setPixel(126,61,imgBuffer);
    setPixel(126,63,imgBuffer);

    // the curve itself
    for (uint8_t ix=0;ix<63;ix++)
    {
        float fx = ((float)ix)/62.0f;
        float fy = gdGetValue(fx,distortion)*62.0f;
        uint8_t iy = (uint8_t)fy;
        if (ix > 0)
        {
            drawLine(ix-1,ix,oldy,iy,imgBuffer);
        }
        oldy = iy;
    }

    // edit type
    // draw to arrows to move, one single arrow to stretch
    switch(editType)
    {
        case OD_EDIT_POINT01_POS_X:
            // arrow to the right
            drawHorizontal(2,1,9,imgBuffer);
            setPixel(8,1,imgBuffer);
            setPixel(7,0,imgBuffer);
            setPixel(8,3,imgBuffer);
            setPixel(7,4,imgBuffer);

            // arrow to the left
            drawHorizontal(8,1,9,imgBuffer);
            setPixel(2,7,imgBuffer);
            setPixel(3,6,imgBuffer);
            setPixel(2,9,imgBuffer);
            setPixel(3,10,imgBuffer);
            drawText(4,32,"Lower",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT01_POS_Y:
            // arrow down
            drawVertical(2,1,9,imgBuffer);
            setPixel(1,8,imgBuffer);
            setPixel(0,7,imgBuffer);
            setPixel(3,8,imgBuffer);
            setPixel(4,7,imgBuffer);

            // arrow up
            drawVertical(8,1,9,imgBuffer);
            setPixel(7,2,imgBuffer);
            setPixel(6,3,imgBuffer);
            setPixel(9,2,imgBuffer);
            setPixel(10,3,imgBuffer);
            drawText(4,32,"Lower",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT01_SPREAD_X:
            // horizontal two-ended arrow
            drawHorizontal(4,1,9,imgBuffer);
            drawHorizontal(5,1,9,imgBuffer);
            setPixel(8,3,imgBuffer);
            setPixel(7,2,imgBuffer);
            setPixel(8,6,imgBuffer);
            setPixel(7,7,imgBuffer);

            setPixel(2,3,imgBuffer);
            setPixel(3,2,imgBuffer);
            setPixel(2,6,imgBuffer);
            setPixel(3,7,imgBuffer);
            drawText(4,32,"Lower",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT01_SPREAD_Y:
            //vertical two-ended arrow
            drawVertical(4,1,9,imgBuffer);
            drawVertical(5,1,9,imgBuffer);
            setPixel(3,8,imgBuffer);
            setPixel(2,7,imgBuffer);
            setPixel(6,8,imgBuffer);
            setPixel(7,7,imgBuffer);
            setPixel(3,2,imgBuffer);
            setPixel(2,3,imgBuffer);
            setPixel(6,2,imgBuffer);
            setPixel(7,3,imgBuffer);
            drawText(4,32,"Lower",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT23_POS_X:

            // arrow to the right
            drawHorizontal(2,1,9,imgBuffer);
            setPixel(8,1,imgBuffer);
            setPixel(7,0,imgBuffer);
            setPixel(8,3,imgBuffer);
            setPixel(7,4,imgBuffer);

            // arrow to the left
            drawHorizontal(8,1,9,imgBuffer);
            setPixel(2,7,imgBuffer);
            setPixel(3,6,imgBuffer);
            setPixel(2,9,imgBuffer);
            setPixel(3,10,imgBuffer);
            drawText(4,32,"Upper",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT23_POS_Y:
            // arrow down
            drawVertical(2,1,9,imgBuffer);
            setPixel(1,8,imgBuffer);
            setPixel(0,7,imgBuffer);
            setPixel(3,8,imgBuffer);
            setPixel(4,7,imgBuffer);

            // arrow up
            drawVertical(8,1,9,imgBuffer);
            setPixel(7,2,imgBuffer);
            setPixel(6,3,imgBuffer);
            setPixel(9,2,imgBuffer);
            setPixel(10,3,imgBuffer);
            drawText(4,32,"Upper",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT23_SPREAD_X:
            // horizontal two-ended arrow
            drawHorizontal(4,1,9,imgBuffer);
            drawHorizontal(5,1,9,imgBuffer);
            setPixel(8,3,imgBuffer);
            setPixel(7,2,imgBuffer);
            setPixel(8,6,imgBuffer);
            setPixel(7,7,imgBuffer);

            setPixel(2,3,imgBuffer);
            setPixel(3,2,imgBuffer);
            setPixel(2,6,imgBuffer);
            setPixel(3,7,imgBuffer);
            drawText(4,32,"Upper",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
        case OD_EDIT_POINT23_SPREAD_Y:
            //vertical two-ended arrow
            drawVertical(4,1,9,imgBuffer);
            drawVertical(5,1,9,imgBuffer);
            setPixel(3,8,imgBuffer);
            setPixel(2,7,imgBuffer);
            setPixel(6,8,imgBuffer);
            setPixel(7,7,imgBuffer);
            setPixel(3,2,imgBuffer);
            setPixel(2,3,imgBuffer);
            setPixel(6,2,imgBuffer);
            setPixel(7,3,imgBuffer);
            drawText(4,32,"Upper",imgBuffer,(void*)0);
            drawText(4,40,"Knee",imgBuffer,(void*)0);
            break;
    }
    if (selectionMode == OD_MODE_EDITING)
    {
        drawHorizontal (16,2,12,imgBuffer);
        drawHorizontal (18,2,12,imgBuffer);
    }
}


static void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
    data->currentProgram->getParameter(0)->parameterCallback(val);
}

static void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
    data->currentProgram->getParameter(1)->parameterCallback(val);
}

static void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
    data->currentProgram->getParameter(2)->parameterCallback(val);
}

static void enterCallback(PiPicoFxUiType*data) 
{
    selectionMode ^=1;
}

static void exitCallback(PiPicoFxUiType*data)
{
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    uint8_t edtType;
    float points[8];
    if (selectionMode == OD_MODE_SELECTING)
    {
        if (encoderDelta > 0)
        {
            edtType = editType + 1;
            edtType &= 0x7;
            editType = edtType;
        }
        else
        {
            edtType = editType - 1;
            edtType &= 0x7;
            editType = edtType;
        }
    }
    else
    {
        gdGetPoint(0,points,distortion);
        gdGetPoint(1,points+2,distortion);
        gdGetPoint(2,points+4,distortion);
        gdGetPoint(3,points+6,distortion);

        switch (editType)
        {
            case OD_EDIT_POINT01_POS_X:
                points[0] += ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                points[2] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                break;
            case OD_EDIT_POINT01_POS_Y:
                points[1] += ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                points[3] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                break;
            case OD_EDIT_POINT01_SPREAD_X:
                points[0] -= ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                points[2] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                break;
            case OD_EDIT_POINT01_SPREAD_Y:
                points[1] -= ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                points[3] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                break;

            case OD_EDIT_POINT23_POS_X:
                points[4] += ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                points[6] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                break;
            case OD_EDIT_POINT23_POS_Y:
                points[5] += ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                points[7] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE;
                break;
            case OD_EDIT_POINT23_SPREAD_X:
                points[4] -= ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                points[6] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                break;
            case OD_EDIT_POINT23_SPREAD_Y:
                points[5] -= ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                points[7] +=  ((float)encoderDelta)*ENCODER_DELTA_SCALE*0.5f;
                break;
            default:
                break;
        }
        
        gdSetAllPoints(points,distortion);
    }
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
}

void enterLevel9(PiPicoFxUiType*data)
{
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerStompswitch1PressedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompswitch2Callback);
    registerStompswitch3PressedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

