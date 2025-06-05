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

#define USB_DFU_APP_IDLE 0
#define USB_DFU_APP_DETACH 1
#define USB_DFU_IDLE 2
#define USB_DFU_DNLOAD_SYNC 3
#define USB_DFU_DNBUSY 4
#define USB_DFU_DNLOAD_IDLE 5
#define USB_DFU_MANIFEST_SYNC 6 
#define USB_DFU_MANIFEST 7
#define USB_DFU_MANIFEST_WAIT_RESET 8
#define USB_DFU_UPLOAD_IDLE 9
#define USB_DFU_ERROR 10

#define USB_DFU_STATUS_OK 0x00
#define USB_DFU_STATUS_ERR_TARGET 0x01
#define USB_DFU_STATUS_ERR_FILE 0x02
#define USB_DFU_STATUS_ERR_WRITE 0x03
#define USB_DFU_STATUS_ERR_ERASE 0x04
#define USB_DFU_STATUS_ERR_CHECK_ERASE 0x05 
#define USB_DFU_STATUS_ERR_PROG 0x06
#define USB_DFU_STATUS_ERR_VERIFY 0x07
#define USB_DFU_STATUS_ERR_ADDRESS 0x08
#define USB_DFU_STATUS_ERR_NOTDONE 0x09
#define USB_DFU_STATUS_ERR_FIRMWARE 0x0A
#define USB_DFU_STATUS_ERR_VENDOR 0x0B
#define USB_DFU_STATUS_ERR_USBR 0x0C
#define USB_DFU_STATUS_ERR_POR 0x0D
#define USB_DFU_STATUS_ERR_UNKNOWN 0x0E
#define USB_DFU_STATUS_ERR_STALLEDPKT 0x0F

typedef struct __attribute__((__packed__)){
    uint8_t bStatus;
    uint8_t bwPollTimeout[3];
    uint8_t bState;
    uint8_t iString;

} UsbDfuStatusType;

void prepareSystemForDFU();

uint8_t setUsbConfigurationDfu(uint16_t confNr);

void setUsbDfuStatus(volatile UsbDfuStatusType*statusStruct,uint8_t status,uint8_t nextState,uint32_t timeout);

void endPoint0DfuHandler(void*data,uint16_t dataSize);

void usbDfuResetHandler(void);

uint8_t usbDfuHandleClassSetupRequest(const UsbSetupPacketType* packet);

void usbDfuEndManifestation();