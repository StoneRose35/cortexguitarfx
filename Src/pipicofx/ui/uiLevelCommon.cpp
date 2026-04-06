extern "C" {
#include "stdlib.h"
#include "pipicofx/pipicofxui.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/display128x64.h"
#include "audio/looper.h"

#include "images/rt_2s_p_16x16.h"
#include "images/rt_2p_sl_16x16.h"
#include "images/rt_s_2p_16x16.h"
#include "images/rt_parallel_16x16.h"
#include "images/rt_serial_16x16.h"
#include "images/mode_preset_16x16.h"
#include "images/mode_pedalboard_16x16.h"
#include "images/mode_stompbox_16x16.h"
#include "images/mode_parameter_16x16.h"
#include "images/mode_looper_16x16.h"
#include "fonts/TomThumb.h"
#include "ln.h"
#include "gen/version.h"
#include "stringFunctions.h"
}

#include "pipicofx/MultiAudioProcessor.hpp"


void drawMasterVolume(int32_t currentVolume,BwImageType*img)
{
    float fValue,fMaxValue,fMinValue,px,py;
    drawOval(10.f,10.f,64.f,22.f,img);
    clearOval(8.f,8.f,64.f,22.f,img);

    fValue = int2float((int32_t)(currentVolume & 0xFF));
    fMaxValue = int2float((int32_t)(1 << 8));
    fMinValue = int2float((int32_t)0);
    fValue = 0.7853981633974483f + 4.71238898038469f*(fValue - fMinValue)/(fMaxValue-fMinValue); //fValue is now an angle in radians from 45° to 315°
    px = 64.0f - fsin(fValue)*9.0f;
    py = 22.0f + fcos(fValue)*9.0f;
    drawLine(64.f,22.f,px,py,img);
}

void drawAbout(BwImageType* imgBuffer)
{
    char strbfr[24];
    clearSquareInt(0,0,128,43,imgBuffer);
    *strbfr=0;
    appendToString(strbfr,"About PiPicoFX");
    drawText(0,8,strbfr,imgBuffer,(void*)0);
    drawText(0,16,PI_PICO_FX_VERSION_NR,imgBuffer,(void*)0);
    drawText(0,24,PI_PICO_FX_MCU_BOARD,imgBuffer,(void*)0);
    *strbfr=0;
    appendToString(strbfr,"built ");
    appendToString(strbfr,PI_PICO_FX_BUILD_DATE);
    drawText(0,32,strbfr,imgBuffer,(void*)0);
    *strbfr=0;
    appendToString(strbfr,"      ");
    appendToString(strbfr,PI_PICO_FX_BUILD_TIME);
    drawText(0,40,strbfr,imgBuffer,(void*)0);
}