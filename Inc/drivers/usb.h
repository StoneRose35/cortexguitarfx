#ifndef _USB_H_
#define _USB_H_
#include "stdint.h"
#define USB2_OTG_FS_DEVICE ((USB_OTG_DeviceTypeDef*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))

#define PKSTS_GLOBAL_OUT_NAK 1
#define PKSTS_OUT_DATA_PACKET_RECEIVED 2
#define PKSTS OUT_TRANSFER_COMPLETED 3
#define PKSTS_SETUP_TRANSACTION_COMPLETED 4
#define PKSTS_SETUP_DATA_PACKET_RECEIVED 6

#define EPTYP_CONTROL 0
#define EPTYP_ISOCHRONOUS 1
#define EPTYP_BULK 2
#define EPTYP_INTERRUPT 3

#define EP_DIRECTION_OUT 0
#define EP_DIRECTION_IN 1

typedef  void(*endPointHandler)(void*,uint16_t) ;
// initializes the hardware part for usb communication
void initUSB();


uint8_t * getEp0InDataBfr();
void prepareUSBTransfer(uint8_t epNr,uint8_t*data,uint16_t dlen);
void prepareUSBReception(uint8_t epNr,uint16_t dataSize);
void setAddress(uint8_t address);
void setEndpointOutHandler(endPointHandler handler,uint8_t epNr);
void setTransferDoneHandler(void(*handler)(void),uint8_t epNr);

// sets the software buffers, doens't affect the hardware
void setupEndpoint(uint8_t epNr,uint8_t direction,uint16_t maxPacketSize);
#endif