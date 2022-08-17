#ifndef _HIBERNATE_SERVICE_H_
#define _HIBERNATE_SERVICE_H_
#include <stdint.h>

#define HIBERNATE_WAKEUP 1
#define HIBERNATE_SLEEP 2

#define HIBERNATE_SLEEPING 1
#define HIBERNATE_AWAKE 0

typedef struct hibernateServiceData
{
    uint32_t lastActionTick;
    uint32_t lastEncoderVal;
    uint8_t lastSwitchVal;
    uint8_t hibernateState;
    uint8_t lastBrightnessValue;
} HibernateServiceDataType;

volatile HibernateServiceDataType * getHibernateServiceData();
uint8_t initHibernateService();

uint8_t hibernateServiceLoop();
#endif