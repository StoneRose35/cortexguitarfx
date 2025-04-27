#ifndef USB_DFU_H
#define USB_DFU_H
#include "stdint.h"

// setup 

#endif // USB_DFU_H
#define SETUP_REQUEST_DFU_DETACH 0
#define SETUP_REQUEST_DFU_DNLOAD 1
#define SETUP_REQUEST_DFU_UPLOAD 2
#define SETUP_REQUEST_DFU_GETSTATUS 3
#define SETUP_REQUEST_DFU_CLRSTATUS 4
#define SETUP_REQUEST_DFU_GETSTATE 5
#define SETUP_REQUEST_DFU_ABORT 6

void prepareSystemFroDFU();

uint8_t setUsbConfiguration(uint8_t confNr);