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

#define USB_VS_Editor_Handshake_Pos 7 // set to one if the webeditor initiated transfer


#define USB_VS_RR_BUFFER_SIZE 0x1FF


void USBVendorSpecificIFInit();
uint8_t usbVendorSpecificIFSetConfiguration(uint16_t confNr);
uint8_t usbVendorSpecificIFHandleClassSetupRequest(const UsbSetupPacketType* packet);
uint8_t usbVendorSpecificIFSetInterfaceHandlerDfu(uint16_t alternateSetting);
uint8_t usbVendorSpecificIFSetInterfaceHandlerVS(uint16_t alternateSetting);
void UsbVendorSpecificIFDataReceived(void* dataPtr,uint16_t len);
void UsbVendorSpecificIFEp0OutHandler(void* dataPtr,uint16_t len);
uint16_t getUsbVendorSpecificReceivedDataLevel();
uint16_t readUsbVendorSpecificData(uint8_t * data,uint16_t startIndex);
void handleUsbVendorSpecificIFVendorSetupRequest(const UsbSetupPacketType* packet);
void usbVendorSpecificIFSuspendedHandler();
uint8_t usbVendorSpecifiIFIsConfigured();
void usbVendorSpecificSetEditorHandshake(uint8_t val);
uint8_t usbVendorSpecificGetEditorHandshake();
void usbVendorSpecificDeactive(void);
#endif