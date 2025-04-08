#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb/usb_common.h"
#include "drivers/usb.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_cdc.h"


const uint8_t usbDeviceDescriptorFull[] = {
    0x12, // bLength
    0x01, // device descriptor type
    0x00,
    0x02, //bcdUSB
    0x02, // device class
    0x02, // device subclass
    0x00, // device protocol
    0x40, // max endpoint0 size
    0xfe, // vendor id, lsb
    0xca, // vendor id, msb
    0x42, // product id, lsb
    0x41, // product id, msb
    0x00,
    0x02, //bcdDevice
    0x01, // manufacturer string id
    0x02, // product string id
    0x03, // serial string id
    0x01, // max number of configurations
    } ;
    
    const uint8_t usbConfigurationDescriptorFull[] = {
    // ------------------------------------
    // configuration header
    // ------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_CONFIGURATION, // descriptor type configuration
    67, // configation descriptor size, lsb
    0x00, // configurator descriptor size, msb
    0x02, // bNumInterfaces
    0x01, // bConfigurationValue
    0x00, // configuration string id
    0xC0, // bmAttributes, 0xC0 self powered, 0x80 bus powered
    0xFA, // bus power
    //-------------------------------------
    //-------------------------------------
    // Interface Descriptor: Control interface
    //-------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, // bDescriptorType: descriptor type interface
    0x00, // bInterfaceNumber: interface nr
    0x00, // bAlternateSetting: alternate setting
    0x01, // bNumEndpoints: just one
    0x02, // bInterfaceClass: Communication Interface Class
    0x02, // bInterfaceSubClass: Abstract Control Model
    0x01, // bInterfaceProtocol: standard AT commands
    0x04, // interface description string id
      // Header Functional Descriptor 
    0x05, // bLength: Endpoint Descriptor size 
    0x24, // bDescriptorType: CS_INTERFACE 
    0x00, // bDescriptorSubtype: Header Func Desc 
    0x10, // bcdCDC: spec release number 
    0x01,
    
      // Call Management Functional Descriptor 
    0x05, // bFunctionLength 
    0x24, // bDescriptorType: CS_INTERFACE 
    0x01, // bDescriptorSubtype: Call Management Func Desc 
    0x00, // bmCapabilities: D0+D1 
    0x01, // bDataInterface 
    
      // ACM Functional Descriptor 
    0x04, // bFunctionLength 
    0x24, // bDescriptorType: CS_INTERFACE 
    0x02, // bDescriptorSubtype: Abstract Control Management desc 
    0x02, // bmCapabilities 
    
      // Union Functional Descriptor 
    0x05, // bFunctionLength 
    0x24, // bDescriptorType: CS_INTERFACE 
    0x06, // bDescriptorSubtype: Union func desc 
    0x00, // bMasterInterface: Communication class interface 
    0x01, // bSlaveInterface0: Data Class Interface 
    //-------------------------------------
    //-------------------------------------
    // Endpoint 2 descriptor
    //-------------------------------------
    0x07, //bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT, //bDescriptorType: Endpoint 
    (ENDPOINT_DIR_IN << ENDPOINT_DIR_POS) | 0x02, // endpoint nr and direction
    ENDPOINT_ATTR_TRANSFERTYPE_INTERRUPT,
    0x08, // packet size, lsb
    0x00, // packet size, msb
    0x10, // bInterval: interval for polling the endpoint in ms
    //-------------------------------------
    //-------------------------------------
    // Interface Descriptor: Data Interface
    //-------------------------------------
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    0x01, // bInterfaceNumber: interface nr
    0x00, // bAlternateSetting: alternate setting
    0x02, // bNumEndpoints: two (out and in)
    0x0A, // Interface class: CDC
    0x00, // interface subclass
    0x00, // interface protocol
    0x05, // interface description string id
    //-------------------------------------
    //-------------------------------------
    // Endpoint OUT descriptor
    //-------------------------------------
    0x07, // bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT, 
    (ENDPOINT_DIR_OUT << ENDPOINT_DIR_POS) | 0x01, // endpoint address: 1 OUT,
    0x02, // bmAttributes: Bulk
    0x40, // packet size, lsb
    0x00, // packet size, msb
    0x00, // bInterval
    //-------------------------------------
    //-------------------------------------
    // Endpoint IN descriptor
    //-------------------------------------
    0x07, // bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT, 
    (ENDPOINT_DIR_IN << ENDPOINT_DIR_POS) | 0x01, // endpoint address: 1 IN,
    0x02, // bmAttributes: Bulk
    0x40, // packet size, lsb
    0x00, // packet size, msb
    0x00, // bInterval
    };
    
    
    UsbString0DescriptorType string0Descriptor = {
        .bLength = 4,
        .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING,
        .wLangId = 0x0409
    };
    
    UsbStringDescriptorType stringDescriptors[] = {
        {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = 12+2, .bString = "StoneRose35"},
        {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = 9+2, .bString = "PiPicoFX"},
        {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = 11+2, .bString = "3457456234"},
        {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = 9+2, .bString = "Data Interface"},
    };
    

// Function to decode a USB setup packet
void ProcessUsbSetupPackage(const UsbSetupPacketType *packet) {

    uint8_t requestType = (packet->bmRequestType & 0x60) >> 5;
    if (requestType == SETUP_PACKET_REQTYPE_STD) 
    {
        switch (packet->bRequest) {
            case 0x00: // GET_STATUS
                uint16_t data;
                uint8_t recipient = packet->bmRequestType & 0x1F;
                switch (recipient)
                {
                    case REQUEST_TYPE_RECIPIENT_DEVICE:
                        data = 1;
                        sendUSBData(0,&data,2,0);
                        break;
                    case REQUEST_TYPE_RECIPIENT_INTERFACE:
                        data = 0;
                        sendUSBData(0,&data,2,0);
                        break;
                    case REQUEST_TYPE_RECIPIENT_ENDPOINT:
                        uint16_t endpointNr = packet->wIndex;
                        if ((endpointNr & 0x80)== 0x80) // OUT endpoints
                        {
                            data = ((((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_OUT_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DOEPCTL >> USB_OTG_DOEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                                sendUSBData(0,&data,2,0);
                        }
                        else
                        {
                            data = ((((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_IN_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DIEPCTL >> USB_OTG_DIEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                            sendUSBData(0,&data,2,0);
                        }
                        break;

                }
                break;
            case 0x01: // CLEAR_FEATURE
            
                break;
            case 0x02: // SET_FEATURE
                
                break;
            case 0x03: // SET_ADDRESS
                setPendingAddress((uint8_t)packet->wValue);
                break;
            case 0x05: // SET_CONFIGURATION
                setUsbConfiguration(packet->wValue);
                break;
            case 0x06: // GET_DESCRIPTOR
                switch (packet->wValue >> 8) {
                    case 0x01: // DEVICE                    
                        sendUSBData(0, usbDeviceDescriptorFull, sizeof(usbDeviceDescriptorFull) , 0);
                        break;
                    case 0x02: // CONFIGURATION
                        sendUSBData(0, usbConfigurationDescriptorFull, sizeof(usbConfigurationDescriptorFull), 0);
                        break;
                    case 0x03: // STRING
                        if ((packet->wValue & 0xFF)!= 0) {
                            // Request for string descriptor 1
                            sendUSBData(0,&stringDescriptors[((packet->wValue  & 0xFF) - 1)], stringDescriptors[(packet->wValue  & 0xFF) - 1].bLength, 0);
                        } else {
                            // Request for string descriptor 0
                            sendUSBData(0, &string0Descriptor, string0Descriptor.bLength, 0);
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    else if (requestType == SETUP_PACKET_REQTYPE_CLASS)
    {
        handleClassSetupRequest(packet);
    }
    else if (requestType == SETUP_PACKET_REQTYPE_VENDOR)
    {
        handleVendorSetupRequest(packet);
    }

    // Decode bmRequestType
    //uint8_t direction = (packet->bmRequestType & 0x80) >> 7;
    //uint8_t type = (packet->bmRequestType & 0x60) >> 5;
    //uint8_t recipient = packet->bmRequestType & 0x1F;

    //printf("Direction: %s\n", direction ? "Device-to-Host" : "Host-to-Device");
    //printf("Type: ");
    //switch (type) {
    //    case 0: printf("Standard\n"); break;
    //    case 1: printf("Class\n"); break;
    //    case 2: printf("Vendor\n"); break;
    //    default: printf("Reserved\n"); break;
    //}
    //printf("Recipient: ");
    //switch (recipient) {
    //    case 0: printf("Device\n"); break;
    //    case 1: printf("Interface\n"); break;
    //    case 2: printf("Endpoint\n"); break;
    //    case 3: printf("Other\n"); break;
    //    default: printf("Reserved\n"); break;
    //}
}