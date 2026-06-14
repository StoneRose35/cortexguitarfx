#ifndef USB_VENDOR_SPECIFIC_DFU_CAPABLE_H_
#define USB_VENDOR_SPECIFIC_DFU_CAPABLE_H_

#include "stdint.h"
#include "stdlib.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_common.h"


#define USB_VS_CONTROL_EP_PACKETSIZE 8
#define USB_VS_DATA_IN_PACKETSIZE 64
#define USB_VS_DATA_OUT_PACKETSIZE 64

#define USB_CDC_FLUSH_TIMEOUT 100

#define USB_VS_TransferInProgress_Pos 1
#define USB_VS_Configured_Pos 0

#define USB_VS_RR_BUFFER_SIZE 0x1FF

void usbVendorSpecificSendData(uint8_t * data,uint16_t dlen,uint8_t blocking);

void USBVendorSpecificIFInit();
uint8_t usbVendorSpecificIFSetConfiguration(uint16_t confNr);
uint8_t usbVendorSpecificIFHandleClassSetupRequest(const UsbSetupPacketType* packet);
uint8_t usbVendorSpecificIFSetInterfaceHandler(uint16_t alternateSetting,uint16_t interfaceIndex);
void UsbVendorSpecificIFTransferDone(void);
void UsbVendorSpecificIFDataReceived(void* dataPtr,uint16_t len);
void UsbVendorSpecificIFEp0OutHandler(void* dataPtr,uint16_t len);
uint16_t getUsbVendorSpecificReceivedDataLevel();
uint16_t readUsbVendorSpecificData(uint8_t * data,uint16_t startIndex);
void handleUsbVendorSpecificIFVendorSetupRequest(const UsbSetupPacketType* packet);
void usbVendorSpecificIFSuspendedHandler();
#endif