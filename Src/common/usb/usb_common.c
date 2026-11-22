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
#include "usb/usb_dfu.h"

    
const uint8_t  string0Descriptor[] = {
    (uint8_t)0x04, //bLength
    (uint8_t)0x03, //bDescriptorType
    (uint8_t)0x09, // lo byte of 1033
    (uint8_t)0x04}; // hi byte of 1033


static const uint8_t * currentConfigurationDescriptor;
static volatile uint16_t currentConfigurationDescriptorSize;

static const uint8_t * currentBOSDescriptor;
static volatile uint16_t currentBOSDescriptorSize;

static const volatile uint8_t * currentDeviceDescriptor;
static volatile uint16_t currentDeviceDescriptorSize;

static const uint8_t * currentDeviceQualifierDescriptor;
static volatile uint16_t currentDeviceQualifierDescriptorSize;

static volatile UsbStringDescriptor currentStringDescriptors;
static uint8_t configurationHandlersCount = 0;
static uint8_t(*currentConfigurationHandler[8])(uint16_t) = {0,0,0,0,0,0,0,0};

// array index is the interface index
static uint8_t(*currentUsbClassSpecificSetupHandler[8])(const volatile UsbSetupPacketType*)={0,0,0,0,0,0,0,0};
// array index is the interface index
static uint8_t(*currentUsbSetInterfaceHandler[8])(uint16_t alternateSetting)={0,0,0,0,0,0,0,0};

static void(*currentVendorSpecificSetupHandler)(const UsbSetupPacketType* packet);

volatile uint8_t bmUsbStatus=0; 
volatile uint8_t bmTransferProgress=0;
static volatile uint16_t sendDataSize=0;
static uint8_t  sendDataBfr[512];
static volatile uint8_t chunksSent=0;

// Function to decode a USB setup packet
__RAMFUNC
void ProcessUsbSetupPackage(const UsbSetupPacketType *packet) {

    uint8_t requestType = (packet->bmRequestType & 0x60) >> 5;
    uint16_t data;
    uint16_t endpointNr = packet->wIndex;
    uint8_t* dataPtr=0;
    uint16_t descrLength=0;
    uint8_t getDescriptorStalled=0;
    uint16_t effLength;
    uint8_t recipient = packet->bmRequestType & 0x1F;
    uint8_t retval=0;
    if (requestType == SETUP_PACKET_REQTYPE_STD) 
    {
        switch (packet->bRequest) {
            case 0x00: // GET_STATUS
                switch (recipient)
                {
                    case REQUEST_TYPE_RECIPIENT_DEVICE:
                        data = 1;
                        #ifdef USB_DBG
                        sendStringBlocking("GET_STATUS, device\r\n");
                        #endif
                        prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength,0);
                        break;
                    case REQUEST_TYPE_RECIPIENT_INTERFACE:
                        data = 0;
                        #ifdef USB_DBG
                        sendStringBlocking("GET_STATUS, interface\r\n");
                        #endif
                        prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength,0);
                        break;
                    case REQUEST_TYPE_RECIPIENT_ENDPOINT:
                        if ((endpointNr & 0x80)== 0x80) // OUT endpoints
                        {
                            #ifdef USB_DBG
                            sendStringBlocking("GET_STATUS, endpoint OUT\r\n");
                            #endif
                            data = ((((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_OUT_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DOEPCTL >> USB_OTG_DOEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                                prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength,0);
                        }
                        else
                        {
                            #ifdef USB_DBG
                            sendStringBlocking("GET_STATUS, endpoint IN\r\n");
                            #endif
                            data = ((((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE 
                                + USB_OTG_IN_ENDPOINT_BASE
                                + 0x20*(endpointNr & 0x7F)))->DIEPCTL >> USB_OTG_DIEPCTL_USBAEP_Pos) & 0x1) ^ 0x1;
                            prepareUSBTransfer(0,(uint8_t*)&data,packet->wLength,0);
                        }
                        break;

                }
                break;
            case 0x01: // CLEAR_FEATURE
                #ifdef USB_DBG
                sendStringBlocking("CLEAR_FEATURE\r\n");
                #endif
                prepareUSBTransfer(0,0,packet->wLength,0); // status package
                break;
            case 0x03: // SET_FEATURE
                #ifdef USB_DBG
                sendStringBlocking("SET_FEATURE\r\n");
                #endif
                prepareUSBTransfer(0,0,packet->wLength,0); // status package
                break;
            case 0x05: // SET_ADDRESS
                #ifdef USB_DBG
                sendStringBlocking("SET_ADDRESS\r\n");
                #endif
                setAddress((uint8_t)packet->wValue);
                prepareUSBTransfer(0,0,packet->wLength,0);
                break;
            case 0x09: // SET_CONFIGURATION
                #ifdef USB_DBG
                sendStringBlocking("SET_CONFIGURATION\r\n");        
                #endif

                for (uint8_t c=0;c<configurationHandlersCount;c++)
                {
                    retval += currentConfigurationHandler[c](packet->wValue);
                }
                if (retval == 0)
                {
                    bmUsbStatus |= (1 << USB_Configured_Pos);
                    prepareUSBTransfer(0,0,packet->wLength,0); // status package
                }
                break;
            case 0x06: // GET_DESCRIPTOR

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
                    sendStringBlocking((char*)stringchar);
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
                else if ((packet->wValue >> 8)==0x0F) // BOS Descriptor
                {
                    dataPtr = (uint8_t*)currentBOSDescriptor;
                    descrLength = currentBOSDescriptorSize;
                } 
                else if ((packet->wValue >> 8)==0x06) // Device Qualifier
                {
                    dataPtr = (uint8_t*)currentDeviceQualifierDescriptor;
                    descrLength=currentDeviceQualifierDescriptorSize;
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
                    usbSendData(0,dataPtr,effLength,0); // sends out data if effLength > 0, an empty/status package otherwise
                }
                break;
            case 0x08: // GET_CONFIGURATION
                if (bmUsbStatus & (1 << USB_Configured_Pos))
                {
                    retval = 1;
                }
                else
                {
                    retval = 0;
                }
                usbSendData(0,&retval,1,0);
                break;
            case 0x0B: // SET_INTERFACE
                if (currentUsbSetInterfaceHandler[packet->wIndex] != 0)
                {
                    if (currentUsbSetInterfaceHandler[packet->wIndex](packet->wValue) != 0U)
                    {
                        stallInEndpoint(0);
                    }
                }
                else
                {
                    if (packet->wValue == 0 && packet->wIndex == 0)
                    {
                        setClassSpecificSetupHandler(&usbDfuHandleClassSetupRequest,0);
                    }
                    prepareUSBTransfer(0,0,0,0);
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
        if (currentUsbClassSpecificSetupHandler[packet->wIndex] != 0)
        {
            if (currentUsbClassSpecificSetupHandler[packet->wIndex](packet) != 0U)
            {
                stallInEndpoint(0);
            }
        }
        else
        {
            stallInEndpoint(0);
        }
    }
    else if (requestType == SETUP_PACKET_REQTYPE_VENDOR)
    {
        #ifdef USB_DBG
        sendStringBlocking("Setup Packet, vendor-type request\r\n");
        #endif
        if (currentVendorSpecificSetupHandler != 0)
        {
            currentVendorSpecificSetupHandler(packet);
        }
    }
}



/**
 * send data over usb 
 * data: pointer to the data to be sent
 * dlen: total length in bytes to send
 * blocking: doesn't return until data has been sent of blocking is 1
 * 
 * note: blocks if the dlen is larger than the internal buffer size (512)
 */
__RAMFUNC
void usbSendData(uint8_t epNr,const uint8_t * data,uint16_t dlen,uint8_t blocking)
{
    uint16_t bytesSent=0;
    chunksSent = 0;
    sendDataSize =dlen;
    if ((bmTransferProgress & (1 << epNr)) != 0 )
    {
        while((bmTransferProgress & (1 << epNr)) != 0);
    }
    bmUsbStatus &= ~(7 << USB_Endpoint_Pos);
    bmUsbStatus |= (epNr << USB_Endpoint_Pos);
    while (bytesSent < dlen)
    {
        for (uint16_t c=0;c<dlen && c < 512;c++)
        {
            *(sendDataBfr + c) = *(data + c);
            bytesSent++;
        }
        if (((bmUsbStatus & (1 << USB_Configured_Pos)) || epNr == 0) != 0 && (bmTransferProgress & (1 << epNr)) == 0)
        {
            if (sendDataSize > 64)
            {
                prepareUSBTransfer(epNr,sendDataBfr,64,0);
                sendDataSize -= 64;
            }
            else
            {
                prepareUSBTransfer(epNr,sendDataBfr,dlen,0);
                sendDataSize = 0;
            }
            bmTransferProgress |= (1 << epNr);
            if (bytesSent < dlen)
            {
                while((bmTransferProgress & (1 << epNr)) != 0);
            }
        }
    }
    
    if (blocking != 0 )
    {
        while((bmTransferProgress & (1 << epNr)) != 0);
    }
}


// returns 1 if the transfer is still in progress, 0 if done
__RAMFUNC
uint8_t UsbTransferDone(uint8_t epNr)
{
    chunksSent += 1;
    if (sendDataSize > 64)
    {
        prepareUSBTransfer(epNr,sendDataBfr+(chunksSent << 6),64,0);   
        sendDataSize -=64;
    }
    else if (sendDataSize > 0)
    {
        prepareUSBTransfer(epNr,sendDataBfr+(chunksSent << 6),sendDataSize,0);
        sendDataSize = 0;
    }
    else
    {
        bmTransferProgress &= ~(1 << epNr);
    }
    return (bmTransferProgress & (1 << epNr)) >> epNr;
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
void addConfigurationHandler(uint8_t(*confHandler)(uint16_t))
{
    currentConfigurationHandler[configurationHandlersCount++] = confHandler;
}

__RAMFUNC
void setClassSpecificSetupHandler(uint8_t(*handler)(const UsbSetupPacketType*),uint8_t interfaceNr)
{
    currentUsbClassSpecificSetupHandler[interfaceNr] = handler;
}

__RAMFUNC
void setSetInterfaceHandler(uint8_t(*handler)(uint16_t),uint8_t interfaceIdx)
{
    currentUsbSetInterfaceHandler[interfaceIdx] = handler;    
}

__RAMFUNC
void setVendorSpecificSetupHandler(void(*handler)(const UsbSetupPacketType* packet) )
{
    currentVendorSpecificSetupHandler = handler;
}

__RAMFUNC
void setBOSDescriptor(const uint8_t * descr,uint16_t descrSize)
{
    currentBOSDescriptor = descr;
    currentBOSDescriptorSize = descrSize;
}

__RAMFUNC
void setDeviceQualifierDescriptor(const uint8_t * descr,uint16_t descrSize)
{
    currentDeviceQualifierDescriptor = descr;
    currentDeviceQualifierDescriptorSize = descrSize;
}
