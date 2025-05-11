#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb/usb_common.h"
#include "drivers/usb.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_cdc.h"
#include "usb/usb_config.h"
#include "uart.h"
#include "globalConfig.h"
#include "memoryRegions.h"


    
const uint8_t  string0Descriptor[] = {
    (uint8_t)0x04, //bLength
    (uint8_t)0x03, //bDescriptorType
    (uint8_t)0x09, // lo byte of 1033
    (uint8_t)0x04}; // hi byte of 1033


static const uint8_t * currentConfigurationDescriptor;
static volatile uint16_t currentConfigurationDescriptorSize;

static const volatile uint8_t * currentDeviceDescriptor;
static volatile uint16_t currentDeviceDescriptorSize;

static volatile UsbStringDescriptor currentStringDescriptors;
static uint8_t(*currentConfigurationHandler)(uint16_t);

static uint8_t(*currentUsbClassSpecificSetupHandler)(const volatile UsbSetupPacketType*)=0;

// Function to decode a USB setup packet
__RAMFUNC
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
                        #ifdef USB_DBG
                        sendStringBlocking("GET_STATUS, device\r\n");
                        #endif
                        prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength);
                        break;
                    case REQUEST_TYPE_RECIPIENT_INTERFACE:
                        data = 0;
                        #ifdef USB_DBG
                        sendStringBlocking("GET_STATUS, interface\r\n");
                        #endif
                        prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength);
                        break;
                    case REQUEST_TYPE_RECIPIENT_ENDPOINT:
                        uint16_t endpointNr = packet->wIndex;
                        if ((endpointNr & 0x80)== 0x80) // OUT endpoints
                        {
                            #ifdef USB_DBG
                            sendStringBlocking("GET_STATUS, endpoint OUT\r\n");
                            #endif
                            data = ((((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_OUT_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DOEPCTL >> USB_OTG_DOEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                                prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength);
                        }
                        else
                        {
                            #ifdef USB_DBG
                            sendStringBlocking("GET_STATUS, endpoint IN\r\n");
                            #endif
                            data = ((((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_IN_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DIEPCTL >> USB_OTG_DIEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                            prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength);
                        }
                        break;

                }
                break;
            case 0x01: // CLEAR_FEATURE
                #ifdef USB_DBG
                sendStringBlocking("CLEAR_FEATURE\r\n");
                #endif
                prepareUSBTransfer(0,0,packet->wLength); // status package
                break;
            case 0x03: // SET_FEATURE
                #ifdef USB_DBG
                sendStringBlocking("SET_FEATURE\r\n");
                #endif
                prepareUSBTransfer(0,0,packet->wLength); // status package
                break;
            case 0x05: // SET_ADDRESS
                #ifdef USB_DBG
                sendStringBlocking("SET_ADDRESS\r\n");
                #endif
                setAddress((uint8_t)packet->wValue);
                prepareUSBTransfer(0,0,packet->wLength);
                break;
            case 0x09: // SET_CONFIGURATION
                #ifdef USB_DBG
                sendStringBlocking("SET_CONFIGURATION\r\n");        
                #endif
                if (currentConfigurationHandler(packet->wValue) == 0)
                {
                    prepareUSBTransfer(0,0,packet->wLength); // status package
                }
                break;
            case 0x06: // GET_DESCRIPTOR
                uint8_t* dataPtr=0;
                uint16_t descrLength=0;
                uint8_t getDescriptorStalled=0;
                uint16_t effLength;
                if ((packet->wValue >> 8)==0x01) {
                //case 0x01: // DEVICE  
                    #ifdef USB_DBG
                    sendStringBlocking("GET_DESCRIPTOR, device\r\n");
                    #endif     
                    dataPtr = (uint8_t*)currentDeviceDescriptor;  
                    descrLength = currentDeviceDescriptorSize;
                }
                else if ((packet->wValue >> 8)==0x02)
                { //case 0x02: // CONFIGURATION
                    #ifdef USB_DBG
                    sendStringBlocking("GET_DESCRIPTOR, configuration\r\n");
                    #endif  
                    dataPtr = (uint8_t*)currentConfigurationDescriptor;
                    descrLength = currentConfigurationDescriptorSize;
                }
                else if ((packet->wValue >> 8)==0x03) 
                { //  case 0x03: // STRING
                    #ifdef USB_DBG
                    uint8_t stringchar[2]={((uint8_t)packet->wValue)+0x30,0};
                    sendStringBlocking("GET_DESCRIPTOR, string ");
                    sendStringBlocking(stringchar);
                    sendStringBlocking("\r\n");
                    #endif
                    if ((packet->wValue & 0xFF)!= 0) {
                        dataPtr = (uint8_t*)getEp0InDataBfr();
                        descrLength = serializeStringDescriptor(dataPtr,currentStringDescriptors + ((packet->wValue  & 0xFF) - 1));
                        // Request for string descriptor 1
                    } else {
                        // Request for string descriptor 0
                        dataPtr = (uint8_t*)string0Descriptor;
                        descrLength = string0Descriptor[0];
                    }
                }
                else
                {
                    stallInEndpoint(0);
                    getDescriptorStalled=1;
                }
                if (getDescriptorStalled == 0)
                {
                    if (packet->wLength < descrLength)
                    {
                        effLength = packet->wLength;
                    }
                    else
                    {
                        effLength = descrLength;
                    }
                    prepareUSBTransfer(0,dataPtr,effLength); // sends out data if effLength > 0, an empty/status package otherwise
                }
                break;
            default:
                break;
        }
    }
    else if (requestType == SETUP_PACKET_REQTYPE_CLASS)
    {
        #ifdef USB_DBG
        sendStringBlocking("Setup Packet, class-type request\r\n");
        #endif
        if (currentUsbClassSpecificSetupHandler != 0)
        {
            currentUsbClassSpecificSetupHandler(packet);
        }
    }
    else if (requestType == SETUP_PACKET_REQTYPE_VENDOR)
    {
        #ifdef USB_DBG
        sendStringBlocking("Setup Packet, vendor-type request\r\n");
        #endif
        handleVendorSetupRequest(packet);
    }
}

/**
 * generates a valid string descript package and converts an ascii strig to the required unicode
 * format
 */
__RAMFUNC
uint16_t serializeStringDescriptor(uint8_t * dataPtr, UsbStringDescriptor descr)
{
    uint16_t c=0;
    while (*(descr->bString+(c >> 1))!=0)
    {
        *(dataPtr + c + 2)= *(descr->bString+(c >> 1));
        c++;
        *(dataPtr + c + 2)=0;
        c++;

    }
    *dataPtr=c+2;
    *(dataPtr+1)=SETUP_PACKET_DESCR_TYPE_STRING;
    return c+2;
}

__RAMFUNC
void setUsbDeviceDescriptor(const uint8_t * deviceDescr,const uint16_t size)
{
    currentDeviceDescriptor = deviceDescr;
    currentDeviceDescriptorSize = size;
}

__RAMFUNC
void setUsbConfigurationDescriptor(const uint8_t * confDescr,const uint16_t size)
{
    currentConfigurationDescriptor = confDescr;
    currentConfigurationDescriptorSize = size;
}

__RAMFUNC
void setUsbStringDescriptors(UsbStringDescriptor stringDescrs)
{
    currentStringDescriptors = stringDescrs;
}

__RAMFUNC
void setConfigurationHandler(uint8_t(*confHandler)(uint16_t))
{
    currentConfigurationHandler = confHandler;
}

__RAMFUNC
void setClassSpecificSetupHandler(uint8_t(*handler)(const UsbSetupPacketType*))
{
    currentUsbClassSpecificSetupHandler = handler;
}