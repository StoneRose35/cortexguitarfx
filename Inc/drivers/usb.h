#ifndef _USB_H_
#define _USB_H_
#include "stdint.h"
#define USB2_OTG_FS_DEVICE ((USB_OTG_DeviceTypeDef*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))

#define PKSTS_GLOBAL_OUT_NAK 1
#define PKSTS_OUT_DATA_PACKET_RECEIVED 2
#define PKSTS OUT_TRANSFER_COMPLETED 3
#define PKSTS_SETUP_TRANSACTION_COMPLETED 4
#define PKSTS_SETUP_DATA_PACKET_RECEIVED 6
// initializes the hardware part for usb communication
void initUSB();

void sendUSBData(uint8_t epNr,const void*data,uint16_t dlen,uint8_t setDATA0);
void setPendingAddress(uint8_t address);

#endif