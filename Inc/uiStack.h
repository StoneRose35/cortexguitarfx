#ifndef _UI_STACK_H_
#define _UI_STACK_H_
#include <stdint.h>

#define UI_BRIGHTNESS_INITIAL 0x40
void uiStackTask(uint8_t hibernateChange);
void setPagePtr(uint32_t ptr);
void display();
void initUiStack();


void setBrightness(uint8_t b);
uint8_t getBrightness();

/**
 * @brief draws at 2 pixel tall bar spaced 16 pixel vertically apart
 * 
 * @param posx position in pixel
 * @param posy position in pixels
 * @param width width in pixels
 */
void drawSelectMarkers(uint8_t posx,uint8_t posy,uint8_t width,uint8_t spacing);


void clearSelectMarkers(uint8_t posx,uint8_t posy,uint8_t width,uint8_t spacing);

void drawSelectFrame(uint8_t posx,uint8_t posy);

void clearSelectFrame(uint8_t posx,uint8_t posy);

void drawArrows(uint8_t px, uint8_t py,uint8_t spacing);

void clearArrows(uint8_t px, uint8_t py,uint8_t spacing);

typedef struct 
{
    void(*encoderSwitchCallback)(int16_t encoderIncrement,int8_t switchChange); // switch change 1: pushed, -1:released
    void(*display)(void*);
    void(*loop)(void*);
    void* data;
} SubApplicationType;

typedef struct 
{
    uint8_t posx;
    uint8_t posy;
    uint8_t spacing;
    uint8_t width;
    uint8_t page;
    void(*editCallback)(int16_t encoderChange);
    void(*switchCallback)();
} SelectionPositionType;

volatile uint8_t brightness;

#endif