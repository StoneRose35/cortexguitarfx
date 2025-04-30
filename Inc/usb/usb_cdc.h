#ifndef USB_CDC_H_
#define USB_CDC_H_

#include "stdint.h"
#include "stdlib.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_common.h"


#define SETUP_PACKET_REQ_CDC_GET_LINE_CODING 0x20
#define SETUP_PACKET_REQ_CDC_SET_LINE_CODING 0x21
#define SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE 0x22

#define USB_CDC_CONTROL_EP_PACKETSIZE 8
#define USB_CDC_DATA_IN_PACKETSIZE 64
#define USB_CDC_DATA_OUT_PACKETSIZE 64

#define USB_CDC_FLUSH_TIMEOUT 100

#define USB_CDC_TransferInProgress_Pos 1
#define USB_CDC_Configured_Pos 0



void sendOverUsb(uint8_t * data,uint16_t dlen,uint8_t blocking);

void UsbCdcDataReceived(void* dtaPtr,uint16_t len);
void UsbCdcEp0OutHandler(void* dataPtr,uint16_t len);
void UsbCdcTransferDone(void);

uint16_t getUsbCdcReceivedDataLevel();
uint16_t readUsbCdcData(uint8_t *);
void USBCDCInit();
uint8_t usbCdcHandleClassSetupRequest(const UsbSetupPacketType* packet);

uint8_t usbCdcSetConfiguration(uint16_t confNr);
#endif