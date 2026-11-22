#include "stdint.h"
#include "drivers/usb.h"
#include "usb/usb_common.h"
#include "usb/usb_vendor_specific_dfu_capable.h"
#include "usb/usb_dfu.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/helpers.h"
#include "drivers/systick.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "gen/versionDef.h"

#define USB_AUDIO_DFU_INTERFACE 0
#define USB_AUDIO_VENDOR_INTERFACE 1
#define USB_AUDIO_CONTROL_INTERFACE 2
#define USB_AUDIO_PLAYBACK_INTERFACE 3
#define USB_AUDIO_CAPTURE_INTERFACE 4

#define USB_AUDIO_VENDOR_OUT_EP 1
#define USB_AUDIO_VENDOR_IN_EP 1
#define USB_AUDIO_PLAYBACK_OUT_EP 3
#define USB_AUDIO_CAPTURE_IN_EP 4
#define USB_AUDIO_FEEDBACK_IN_EP 5
extern volatile uint8_t bmUsbStatus; 

static const uint8_t usbDeviceDescriptorFull[] = {
    0x12, // bLength
    0x01, // device descriptor type
    0x10,
    0x02, //bcdUSB: 2.10
    0xEF, // device class
    0x02, // device subclass
    0x01, // device protocol
    0x40, // max endpoint0 size
    __LOBYTE(USB_VENDOR_ID), // vendor id, lsb
    __HIBYTE(USB_VENDOR_ID), // vendor id, msb
    __LOBYTE(USB_PRODUCT_ID), // product id, lsb
    __HIBYTE(USB_PRODUCT_ID), // product id, msb
    0x04,
    0x00, // bcdDevice
    0x01, // manufacturer string id
    0x02, // product string id
    0x03, // serial string id
    0x01, // max number of configurations
    };
static const uint16_t usbDeviceDescriptorFullSize = sizeof(usbDeviceDescriptorFull);

static const uint8_t deviceQualifierDescriptor[] = {
    0x0a, // bLength: 10
    0x06, // bDescriptorType: device qualifier
    0x10,
    0x02, // bcdUSB: USB Version 2.10
    0xEF, // bDeviceClass
    0x02, // bDeviceSubclass
    0x01, // bDeviceProtocol
    0x40, // bMaxPacketSize0
    0x01, // bNumConfigurations
    0x00 // bReserved: 0
};

static const uint16_t deviceQualifierDescriptorSize = sizeof(deviceQualifierDescriptor);


static const uint8_t bosDescriptor[]={
    //---------------------------------------
    // Binary device Object Store descriptor
    //---------------------------------------
    //---------------------------------------
    0x05, //bLength
    0x0f, //bDescriptorType: Binary device Object Store descriptor
    57, //wTotalLength, lsb: total length of this series of descriptors
    0x00, // wTotalLength, msb
    0x02, //bNumDeviceCaps, number of device capability descriptors on the BOS
    //---------------------------------------
    // WebUSB platform capability descriptor
    //---------------------------------------
    //---------------------------------------
    0x18, // bLength
    0x10, // bDescriptorType, device capability descriptor
    0x05, //bDevCapabilityType, platform capability descriptor
    0x00, //bReserved
    0x38, //PlatformCapabilityUUID
    0xB6,
    0x08,
    0x34,
    0xA9,
    0x09,
    0xA0,
    0x47,
    0x8B,
    0xFD,
    0xA0,
    0x76,
    0x88,
    0x15,
    0xB6,
    0x65,
    0x00, //bcdVersion, WebUSB descriptor version 1.0
    0x01, //bcdVersion
    0x01, //bVendorCode, bRequest value for WebUSB
    0x01,  //iLandingPage
    //--------------------------------------------------
    // Microsoft OS 2.0 platform capability descriptor
    //--------------------------------------------------
    //--------------------------------------------------
    0x1c, //bLength
    0x10, //bDescriptorType
    0x05, //bDevCapabilityType
    0x00, // bReserved
    0xdf, // PlatformCapabilityUUID Microsoft OS 2.0 platform compatibility descriptor
    0x60,
    0xdd,
    0xd8,
    0x89,
    0x45,
    0xc7,
    0x4c,
    0x9c,
    0xd2,
    0x65,
    0x9d,
    0x9e,
    0x64,
    0x8a,
    0x9f,
    0x00, // dwWindowsVersion, lsb encoded
    0x00,
    0x03,
    0x06,
    0xB2, //wMSODODescriptorTotalLength
    0x00,
    0x02, // bMS_VendorCode
    0x00 //bAltEnumCode, no alternate enumeration

};

static const uint8_t landingPageDescriptor[]={
    36, // bLength
    0x03, // bDescriptorType, URL Descriptor
    0x01, //bScheme, https://
    's',
    't',
    'o',
    'n',
    'e',
    'r',
    'o',
    's',
    'e',
    '3',
    '5',
    '.',
    'c',
    'h',
    '/',
    'd',
    'u',
    'n',
    'g',
    'e',
    'o',
    'n',
    '/',
    'w',
    'e',
    'b',
    'e',
    'd',
    'i',
    't',
    'o',
    'r',
    '/'    
};

static const uint8_t msOS20Descriptor[] = {
    // Microsoft OS 2.0 descriptor set header
    0x0A, //wLength
    0x00,
    0x00, //wDescriptorType
    0x00,
    0x00, // dwWindowsVersion
    0x00,
    0x03,
    0x06,
    0xB2, //wTotalLength
    0x00,
    // Microsoft OS 2.0 configuration subset header
    0x08, //wLength
    0x00,
    0x01, // wDescriptorType
    0x00,
    0x00, //bConfigurationValue, applies to configuration 1
    0x00, //bReserved
    0xA8, // wTotalLength
    0x00,
    // Microsoft OS 2.0 function subset header
    0x08, //wLength
    0x00,
    0x02, //wDescriptorType
    0x00,
    0x01, //bFirstInterface
    0x00, //bReserved
    0xA0, //wSubsetLength
    0x00,
    // Microsoft OS 2.0 compatible ID descriptor
    0x14, //wLength
    0x00,
    0x03, //wDescriptorType
    0x00, 
    'W',//CompatibleID
    'I',
    'N',
    'U',
    'S',
    'B',
    0x00,
    0x00,
    0x00, //SubCompatibleID
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    // Microsoft OS 2.0 registry property descriptor
    0x84, //wLength
    0x00,
    0x04, // wDescriptorType
    0x00,
    0x07, // wPropertyDataType: REG_MULTI_SZ
    0x00, 
    0x2a, // wPropertyNameLength
    0x00,
    // bPropertyName: DeviceInterfaceGUID in UTF16-LE
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 
    'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 
    0x00, 0x00,
    0x50, // wPropertyDataLength
    0x00, 
    // wPropertyData: just a random UUID propertly encoded: "{F202C9DE-E6A6-44D8-8159-3053C3B10D21}" as UTF16-LE
    // padded with 0 zeros for a total size of 80
    '{', 0x00, 'F', 0x00, '2', 0x00, '0', 0x00, '2', 0x00, 'C', 0x00, '9', 0x00, 'D', 0x00, 'E', 0x00, '-', 0x00,
    'E', 0x00, '6', 0x00, 'A', 0x00, '6', 0x00, '-', 0x00, '4', 0x00, '4', 0x00, 'D', 0x00, '8', 0x00, '-', 0x00,
    '8', 0x00, '1', 0x00, '5', 0x00, '9', 0x00, '-', 0x00, '3', 0x00, '0', 0x00, '5', 0x00, '3', 0x00, 'C', 0x00,
    '3', 0x00, 'B', 0x00, '1', 0x00, '0', 0x00, 'D', 0x00, '2', 0x00, '1', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00

};
    
//static const uint16_t usbConfigurationDescriptorFullSize = sizeof(usbConfigurationDescriptorFull);

static UsbStringDescriptorType stringDescriptors[] = {
    {.bLength = (11*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "StoneRose35"},
    {.bLength = (8*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PiPicoFX"},
    {.bLength = ((sizeof(PI_PICO_FX_VNR)-1)*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = PI_PICO_FX_VNR},
    {.bLength = (17*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Control Interface"},
    {.bLength = (14*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Data Interface"},
    {.bLength = (25*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "DFU Programming Interface"},
    {.bLength = (12*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Default Conf"},
    {.bLength = (18*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Audio Control"},
    {.bLength = (19*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Input Channels"},
    {.bLength = (19*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Software Input"},   
    {.bLength = (16*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Speaker Out"}, 
    {.bLength = (22*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Hiz and Mic Input"},  
    {.bLength = (19*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Physical Input"},  
    {.bLength = (20*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Software Output"},   
    {.bLength = (37*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Streaming IF, Aternate Setting 0"},  
    {.bLength = (37*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PPFX Streaming IF, Aternate Setting 1"},   
     
};


void USBVendorSpecificIFInit()
{
    //setUsbConfigurationDescriptor(usbAudioConfigurationDescriptor,usbConfigurationDescriptorFullSize);
    setUsbDeviceDescriptor(usbDeviceDescriptorFull,usbDeviceDescriptorFullSize);
    setUsbStringDescriptors(stringDescriptors);
    setBOSDescriptor(bosDescriptor,*((uint16_t*)(bosDescriptor+2)));
    setDeviceQualifierDescriptor(deviceQualifierDescriptor,deviceQualifierDescriptorSize);
    addConfigurationHandler(&usbVendorSpecificIFSetConfiguration);
    setClassSpecificSetupHandler(&usbVendorSpecificIFHandleClassSetupRequest,USB_AUDIO_VENDOR_INTERFACE);
    setVendorSpecificSetupHandler(&handleUsbVendorSpecificIFVendorSetupRequest);
    setSetInterfaceHandler(&usbVendorSpecificIFSetInterfaceHandlerDfu,2);
    setSetInterfaceHandler(&usbVendorSpecificIFSetInterfaceHandlerVS,1);
    setSuspendedHandler(&usbVendorSpecificIFSuspendedHandler);
    setTransferDoneHandler(&UsbTransferDone,0);   
}


static volatile uint8_t  receivedDataBfr[512];
static volatile uint16_t receivedDataLevel=0;
static volatile uint16_t receivedDataLevelSnapshot=0;
static volatile uint8_t isActive = 0;
static volatile uint8_t rearmCdcReceiver=0;

uint8_t usbVendorSpecificIFSetConfiguration(uint16_t confNr)
{
    
    (void)confNr;
    USB_OTG_INEndpointTypeDef * inEndpoint;
    USB_OTG_OUTEndpointTypeDef * outEndpoint;

    uint16_t offset;
    uint16_t cnt;
    // OUT Endpoint 1: Bulk, Packet Size USB_CDC_DATA_OUT_PACKETSIZE
    setupEndpoint(1,EP_DIRECTION_OUT,USB_VS_DATA_OUT_PACKETSIZE);
    outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 1*0x20));
    outEndpoint->DOEPCTL =  (1 << (USB_OTG_DOEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DOEPCTL_EPTYP_Pos))
                            | (USB_VS_DATA_OUT_PACKETSIZE << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_OEPM_Pos+1));

    // IN Endpoint 1: Bulk, Packet Size USB_CDC_DATA_OUT_PACKETSIZE, fifo 1
    // define fifo ram area
    offset = USB2_OTG_FS->GRXFSIZ + ((USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ >> 16) & 0xFFFF);
    USB2_OTG_FS->DIEPTXF[1-1] = offset | ((USB_VS_DATA_IN_PACKETSIZE << 3) << USB_OTG_DIEPTXF_INEPTXFD_Pos);
    // flush fifo
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U && cnt < USB_CDC_FLUSH_TIMEOUT)
    {
        nop_wait(480000);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        // timeout when flushing IN endpoint 1, return set configuration failed
        return 1;
    }
    USB2_OTG_FS->GRSTCTL = ((1 << USB_OTG_GRSTCTL_TXFFLSH_Pos) | (0x1 << 6));
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH)
    {
        nop_wait(480000);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        return 1;
    }
    setupEndpoint(1,EP_DIRECTION_IN,USB_VS_DATA_IN_PACKETSIZE);
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 1*0x20));
    inEndpoint->DIEPCTL =   (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (1 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (USB_VS_DATA_IN_PACKETSIZE << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_IEPM_Pos+1));

    // IN Endpoint 2: Interrupt, Packet Size 8, fifo 2
    // define fifo ram area
    offset = USB2_OTG_FS->GRXFSIZ + ((USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ >> 16) & 0xFFFF) + ((USB2_OTG_FS->DIEPTXF[1-1] >> 16) & 0xFFFF);
    USB2_OTG_FS->DIEPTXF[2-1] = offset | ((USB_VS_CONTROL_EP_PACKETSIZE << 3) << USB_OTG_DIEPTXF_INEPTXFD_Pos);
    // flush fifo
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U && cnt < USB_CDC_FLUSH_TIMEOUT)
    {
        nop_wait(480000);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        // timeout when flushing IN endpoint 1, return set configuration failed
        return 1;
    }
    USB2_OTG_FS->GRSTCTL = ((1 << USB_OTG_GRSTCTL_TXFFLSH_Pos) | (0x2 << 6));
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH)
    {
        nop_wait(480000);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        return 1;
    }
    setupEndpoint(2,EP_DIRECTION_IN,USB_VS_CONTROL_EP_PACKETSIZE);
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 2*0x20));
    inEndpoint->DIEPCTL =  (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_INTERRUPT << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (2 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (USB_VS_CONTROL_EP_PACKETSIZE  << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_IEPM_Pos+2));


    isActive = 1;
    // wire up endpoint reception handler
    setEndpointOutHandler(&UsbVendorSpecificIFDataReceived,1);
    // wire up transfer done handler (used for blocking transfer)
    setTransferDoneHandler(&UsbTransferDone,1);
    // enable reception of data at endpoint 1
    prepareUSBReception(1,USB_VS_DATA_OUT_PACKETSIZE);

    
    return 0;
}

uint8_t usbVendorSpecificIFHandleClassSetupRequest(const UsbSetupPacketType* packet)
{
    switch (packet->bRequest)
    {
    case SETUP_REQUEST_DFU_DETACH:
        usbDfuHandleClassSetupRequest(packet);
        break;
    default:
        
        break;
    }
    return 0;
}

uint8_t usbVendorSpecificIFSetInterfaceHandlerDfu(uint16_t alternateSetting)
{
    if (alternateSetting==0) // match dfu interface with alternate setting 0
    {
        setClassSpecificSetupHandler(usbDfuHandleClassSetupRequest,0); // switch over to dfu mode (required for dfu-util)
        prepareUSBTransfer(0,0,0,0);
    }
    return 0;
}

uint8_t usbVendorSpecificIFSetInterfaceHandlerVS(uint16_t alternateSetting)
{
    if (alternateSetting==0) // usb vendor speific interface, acknowledge
    {
        prepareUSBTransfer(0,0,0,0);
    }
    return 0;
}



void UsbVendorSpecificIFDataReceived(void* dataPtr,uint16_t len)
{
    uint16_t c=0;

    while (((receivedDataLevel - receivedDataLevelSnapshot) & USB_VS_RR_BUFFER_SIZE) < USB_VS_RR_BUFFER_SIZE && c < len)
    {
        *(receivedDataBfr + (receivedDataLevel++)) = *((uint8_t*)dataPtr + c);
        receivedDataLevel &= USB_VS_RR_BUFFER_SIZE; 
        c++;
    }
    
    if (((receivedDataLevel - receivedDataLevelSnapshot) & USB_VS_RR_BUFFER_SIZE) < (USB_VS_RR_BUFFER_SIZE + 1) - USB_VS_DATA_OUT_PACKETSIZE)
    {
        prepareUSBReception(1,USB_VS_DATA_OUT_PACKETSIZE);
    }
    else // rearm reception once data has been read from the buffer
    {
        rearmCdcReceiver = 1;
    }
}

void UsbVendorSpecificIFEp0OutHandler(void* dataPtr,uint16_t len)
{
    (void)dataPtr;
    (void)len;
    prepareUSBReception(0,64);
}


uint16_t getUsbVendorSpecificReceivedDataLevel()
{
    return (receivedDataLevel - receivedDataLevelSnapshot) & USB_VS_RR_BUFFER_SIZE;
}


uint16_t readUsbVendorSpecificData(uint8_t * data,uint16_t startIndex)
{
    uint16_t c=0;
    if (!isActive)
    {
        return 0;
    }
    while(receivedDataLevelSnapshot != receivedDataLevel)
    {
        *(data + ((c+startIndex) & USB_VS_RR_BUFFER_SIZE)) = *(receivedDataBfr + receivedDataLevelSnapshot++);
        receivedDataLevelSnapshot &= USB_VS_RR_BUFFER_SIZE;
        c++;
    }
    if (rearmCdcReceiver == 1)
    {
        prepareUSBReception(1,USB_VS_DATA_OUT_PACKETSIZE);
        rearmCdcReceiver = 0;
    }
    return c;
}

void handleUsbVendorSpecificIFVendorSetupRequest(const UsbSetupPacketType* packet)
{
    if (packet->bRequest == 0x01 // vendor code: WebUSB
        && packet->wValue == 0x01  // iLandingPage
        && packet->wIndex == 0x02) // GET_URL
    {
        
        prepareUSBTransfer(0,landingPageDescriptor,(uint16_t)*landingPageDescriptor,0);
    }
    else if (packet->bRequest == 0x02 // vendor code: MS OS 2.0 Descriptors
        //&& packet->wValue == 0x00 
        && packet->wIndex == 0x07 // get MS OS 2.0 Descriptors
    )
    {
        usbSendData(0,msOS20Descriptor,packet->wLength,0); 
    }
}

void usbVendorSpecificIFSuspendedHandler()
{
    bmUsbStatus = 0;
}


void usbVendorSpecificSetEditorHandshake(uint8_t val)
{
    if (val)
    {
        bmUsbStatus |= (1 << USB_VS_Editor_Handshake_Pos);
    }
    else
    {
        bmUsbStatus &= ~(1 << USB_VS_Editor_Handshake_Pos);
    }
}

void usbVendorSpecificDeactive(void)
{
    isActive = 0;
}

uint8_t usbVendorSpecificGetEditorHandshake()
{
    return (bmUsbStatus & (1 << USB_VS_Editor_Handshake_Pos)) >> USB_VS_Editor_Handshake_Pos;
}

uint8_t usbVendorSpecifiIFIsConfigured()
{
    return (bmUsbStatus & (1 << USB_Configured_Pos)) >> USB_Configured_Pos;
}