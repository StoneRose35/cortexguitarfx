#include "stdint.h"
#include "drivers/usb.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "usb/usb_dfu.h"
#include "stm32h750/stm32h750xx.h"
#include "systick.h"
#include "globalConfig.h"
#include "memoryRegions.h"

static const uint8_t usbDeviceDescriptorFull[] = {
    0x12, // bLength
    0x01, // device descriptor type
    0x10,
    0x01, //bcdUSB
    0x02, // device class
    0x02, // device subclass
    0x00, // device protocol
    0x40, // max endpoint0 size
    __LOBYTE(USB_VENDOR_ID), // vendor id, lsb
    __HIBYTE(USB_VENDOR_ID), // vendor id, msb
    __LOBYTE(USB_PRODUCT_ID), // product id, lsb
    __HIBYTE(USB_PRODUCT_ID), // product id, msb
    0x04,
    0x00, //bcdDevice
    0x01, // manufacturer string id
    0x02, // product string id
    0x03, // serial string id
    0x01, // max number of configurations
    };
static const uint16_t usbDeviceDescriptorFullSize = sizeof(usbDeviceDescriptorFull);
    
static const uint8_t usbConfigurationDescriptorFull[] = {
    // ------------------------------------
    // configuration header
    // ------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_CONFIGURATION, // descriptor type configuration
    85, // configation descriptor size, lsb
    0x00, // configurator descriptor size, msb
    0x03, // bNumInterfaces
    0x01, // bConfigurationValue
    0x07, // configuration string id
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
    USB_CDC_CONTROL_EP_PACKETSIZE, // packet size, lsb
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
    USB_CDC_DATA_OUT_PACKETSIZE, // packet size, lsb
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
    USB_CDC_DATA_IN_PACKETSIZE, // packet size, lsb
    0x00, // packet size, msb
    0x00, // bInterval
    //------------------------------------
    //------------------------------------
    // Interface Descriptor: USB DFU
    // -----------------------------------
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, 
    0x02, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x00, //bNumEndpoints
    0xFE, //bInterfaceClass
    0x01, //bInterfaceSubClass
    0x01, //bInterfaceProtocol
    0x06, //iInterface
    //------------------------------------
    // Interfacce functional descriptor
    //------------------------------------
    //------------------------------------
    0x09, //bLength
    0x21, //bDescriptorType
    (uint8_t)((1 << 3) | (0 << 2) | (1 << 1 ) | ( 1 << 0)), //bmAttributes:will detach, can download and upload 
    0xF0, //wDetachTimeOut, lsb
    0x00, //wDetachTimeOut, msb
    0x00, //wTransferSize, lsb
    0x02, //wTransferSize, msb
    0x1a, // bcdDFUVersion
    0x01  // bcdDFUVersion
    };
    
static const uint16_t usbConfigurationDescriptorFullSize = sizeof(usbConfigurationDescriptorFull);

static UsbStringDescriptorType stringDescriptors[] = {
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (11*2)+2, .bString = "StoneRose35"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (8*2)+2, .bString = "PiPicoFX"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (10*2)+2, .bString = "3457456234"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (17*2)+2, .bString = "Control Interface"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (14*2)+2, .bString = "Data Interface"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (25*2)+2, .bString = "DFU Programming Interface"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = (12*2)+2, .bString = "Default Conf"}
};
#define USB_CDC_TE_RATE (115200)
const uint8_t getcoding[]=
{
USB_CDC_TE_RATE & 0xFF,
(USB_CDC_TE_RATE >> 8) & 0xFF,
(USB_CDC_TE_RATE >> 16) & 0xFF,
(USB_CDC_TE_RATE >> 24) & 0xFF,
0x0, // bCharFormat: 1 Stop bit
0x0, // bParityType: None
0x8 // bDataBits: 8
};

void USBCDCInit()
{
    setUsbConfigurationDescriptor(usbConfigurationDescriptorFull,usbConfigurationDescriptorFullSize);
    setUsbDeviceDescriptor(usbDeviceDescriptorFull,usbDeviceDescriptorFullSize);
    setUsbStringDescriptors(stringDescriptors);
    setConfigurationHandler(&usbCdcSetConfiguration);
    setClassSpecificSetupHandler(&usbCdcHandleClassSetupRequest);
}

volatile uint8_t bmUsbStatus=0; // bit 0: usb cdc configured, bit 1: transfer in progress
volatile uint8_t  receivedDataBfr[128];
volatile uint16_t receivedDataLevel=0;
uint8_t usbCdcSetConfiguration(uint16_t confNr)
{
    
    
    USB_OTG_INEndpointTypeDef * inEndpoint;
    USB_OTG_OUTEndpointTypeDef * outEndpoint;

    uint16_t offset;
    uint16_t cnt;
    // OUT Endpoint 1: Bulk, Packet Size USB_CDC_DATA_OUT_PACKETSIZE
    setupEndpoint(1,EP_DIRECTION_OUT,USB_CDC_DATA_OUT_PACKETSIZE);
    outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 1*0x20));
    outEndpoint->DOEPCTL =  (1 << (USB_OTG_DOEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DOEPCTL_EPTYP_Pos))
                            | (USB_CDC_DATA_OUT_PACKETSIZE << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_OEPM_Pos+1));

    // IN Endpoint 1: Bulk, Packet Size USB_CDC_DATA_OUT_PACKETSIZE, fifo 1
    // define fifo ram area
    offset = USB2_OTG_FS->GRXFSIZ + ((USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ >> 16) & 0xFFFF);
    USB2_OTG_FS->DIEPTXF[1-1] = offset | ((USB_CDC_DATA_IN_PACKETSIZE << 1) << USB_OTG_DIEPTXF_INEPTXFD_Pos);
    // flush fifo
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U && cnt < USB_CDC_FLUSH_TIMEOUT)
    {
        waitSysticks(1);
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
        waitSysticks(1);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        return 1;
    }
    setupEndpoint(1,EP_DIRECTION_IN,USB_CDC_DATA_IN_PACKETSIZE);
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 1*0x20));
    inEndpoint->DIEPCTL =   (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (1 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (USB_CDC_DATA_IN_PACKETSIZE << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_IEPM_Pos+1));

    // IN Endpoint 2: Interrupt, Packet Size 8, fifo 2
    // define fifo ram area
    offset = USB2_OTG_FS->GRXFSIZ + ((USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ >> 16) & 0xFFFF) + ((USB2_OTG_FS->DIEPTXF[1-1] >> 16) & 0xFFFF);
    USB2_OTG_FS->DIEPTXF[2-1] = offset | ((USB_CDC_CONTROL_EP_PACKETSIZE << 1) << USB_OTG_DIEPTXF_INEPTXFD_Pos);
    // flush fifo
    cnt=0;
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U && cnt < USB_CDC_FLUSH_TIMEOUT)
    {
        waitSysticks(1);
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
        waitSysticks(1);
        cnt++;
    }
    if (cnt == USB_CDC_FLUSH_TIMEOUT)
    {
        return 1;
    }
    setupEndpoint(2,EP_DIRECTION_IN,USB_CDC_CONTROL_EP_PACKETSIZE);
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 2*0x20));
    inEndpoint->DIEPCTL =  (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_INTERRUPT << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (2 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (USB_CDC_CONTROL_EP_PACKETSIZE  << (USB_OTG_DIEPCTL_MPSIZ_Pos));
    USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << (USB_OTG_DAINTMSK_IEPM_Pos+2));

    // wire up endpoint reception handler
    setEndpointOutHandler(&UsbCdcDataReceived,1);



    // wire up transfer done handler (used for blocking transfer)
    setTransferDoneHandler(&UsbCdcTransferDone,1);
    // enable reception of data at endpoint 1
    prepareUSBReception(1,USB_CDC_DATA_OUT_PACKETSIZE);
    bmUsbStatus |= 1;
    
    return 0;
}

uint8_t usbCdcHandleClassSetupRequest(const UsbSetupPacketType* packet)
{
    switch (packet->bRequest)
    {
    case SETUP_PACKET_REQ_CDC_GET_LINE_CODING:
        prepareUSBTransfer(0,(uint8_t*)&getcoding,7);
        break;
    case SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE:
        prepareUSBTransfer(0,0,0);
        break;
    case SETUP_PACKET_REQ_CDC_SET_LINE_CODING:
        prepareUSBTransfer(0,0,0);
        break;
    case SETUP_REQUEST_DFU_DETACH:
        usbDfuHandleClassSetupRequest(packet);
        break;
    default:
        
        break;
    }
    return 0;
}


void sendOverUsb(uint8_t * data,uint16_t dlen,uint8_t blocking)
{
    
    if ((bmUsbStatus & (1 << USB_CDC_Configured_Pos)) != 0 && (bmUsbStatus & (1 << USB_CDC_TransferInProgress_Pos)) == 0)
    {
        prepareUSBTransfer(1,data,dlen);
        bmUsbStatus |= (1 << USB_CDC_TransferInProgress_Pos);
        if (blocking != 0)
        {
        while((bmUsbStatus & (1 << USB_CDC_TransferInProgress_Pos)) != 0);
        }
    }
}


void UsbCdcTransferDone(void)
{
    bmUsbStatus &= ~(1 << USB_CDC_TransferInProgress_Pos);
}

void UsbCdcDataReceived(void* dataPtr,uint16_t len)
{
    uint16_t c=0;
    if (receivedDataLevel == 0)
    {
        while (c + receivedDataLevel < 128 && c < len)
        {
            *(receivedDataBfr + c + receivedDataLevel) = *((uint8_t*)dataPtr + c); 
            c++;
        }
        receivedDataLevel = len;
    }
    prepareUSBReception(1,USB_CDC_DATA_OUT_PACKETSIZE);
}

void UsbCdcEp0OutHandler(void* dataPtr,uint16_t len)
{
    prepareUSBReception(0,64);
}

uint16_t getUsbCdcReceivedDataLevel()
{
    return receivedDataLevel;
}

uint16_t readUsbCdcData(uint8_t * data)
{
    uint16_t lastDataLevel = receivedDataLevel;
    for(uint16_t c=0;c<receivedDataLevel;c++)
    {
        *(data + c) = *(receivedDataBfr + c);
    }
    receivedDataLevel = 0;
    return lastDataLevel;
}

void handleVendorSetupRequest(const UsbSetupPacketType* packet)
{

}