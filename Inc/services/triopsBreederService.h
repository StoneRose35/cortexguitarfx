#ifndef _TRIOPS_BREEDER_SERVICE_H_
#define _TRIOPS_BREEDER_SERVICE_H_
#include <stdint.h>

typedef struct TriopsBreeder
{
    uint16_t heaterValue; // from 0 to 1023
    uint16_t temperature; //encoded as 4bit fractional fixed point
    uint16_t tLower; // temperature limit when heating should stop
    uint16_t tTarget; // target temperature, should be around 25°C
    uint16_t cIntegral; // integral factor of the P-I controller
    int32_t integralTempDeviation; // summed up temperature deviation
    uint8_t hourOn; // hour when light should go on
    uint8_t minuteOn; // minute when light should go on
    uint8_t hourOff; // hour when light should go off
    uint8_t minuteOff; // minute when light should go off   
    uint8_t errorFlags; //flags showing error or warnings
    uint8_t lampState; // 0 or 1 
    uint16_t brightnessThreshhold; // brightness threshold below which the lamp should be turned on
    uint16_t currentBrightness; // measured brightness
    uint16_t totalMinutesOn; // on time in minutes
    uint16_t totalMinutesOff; // off time in minutes
    uint32_t serviceInterval; // update interval, 0 is off
} TriopsControllerType;

#define TC_ERROR_THERMOMETER 1
#define TC_ERROR_FILESYSTEM 2


TriopsControllerType * getTriopsController();
uint8_t initTriopBreederService();

void triopBreederServiceLoop();

#endif