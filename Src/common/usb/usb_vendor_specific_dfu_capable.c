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

static const uint8_t usbDeviceDescriptorFull[] = {
    0x12, // bLength
    0x01, // device descriptor type
    0x10,
    0x01, //bcdUSB
    0xFF, // device class
    0x01, // device subclass
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
    50, // configation descriptor size, lsb
    0x00, // configurator descriptor size, msb
    0x03, // bNumInterfaces
    0x01, // bConfigurationValue
    0x07, // configuration string id
    0xC0, // bmAttributes, 0xC0 self powered, 0x80 bus powered
    0xFA, // bus power
    //-------------------------------------

    //-------------------------------------
    // Interface Descriptor: Data Interface
    //-------------------------------------
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    0x01, // bInterfaceNumber: interface nr
    0x00, // bAlternateSetting: alternate setting
    0x02, // bNumEndpoints: two (out and in)
    0xFF, // Interface class: vendor-specific
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
    USB_VS_DATA_OUT_PACKETSIZE, // packet size, lsb
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
    USB_VS_DATA_IN_PACKETSIZE, // packet size, lsb
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
    (uint8_t)((0 << 3) | (0 << 2) | (1 << 1 ) | ( 1 << 0)), //bmAttributes:will not automatically detach and reattach, not manifestation tolerant, can download and upload 
    0xF0, //wDetachTimeOut, lsb
    0x00, //wDetachTimeOut, msb
    0x40, //wTransferSize, lsb
    0x00, //wTransferSize, msb
    0x00, // bcdDFUVersion
    0x01  // bcdDFUVersion
    };
    
static const uint16_t usbConfigurationDescriptorFullSize = sizeof(usbConfigurationDescriptorFull);

static UsbStringDescriptorType stringDescriptors[] = {
    {.bLength = (11*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "StoneRose35"},
    {.bLength = (8*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "PiPicoFX"},
    {.bLength = ((sizeof(PI_PICO_FX_VNR)-1)*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = PI_PICO_FX_VNR},
    {.bLength = (17*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Control Interface"},
    {.bLength = (14*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Data Interface"},
    {.bLength = (25*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "DFU Programming Interface"},
    {.bLength = (12*2)+2, .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bString = "Default Conf"}
};


void USBVendorSpecificIFInit()
{
    setUsbConfigurationDescriptor(usbConfigurationDescriptorFull,usbConfigurationDescriptorFullSize);
    setUsbDeviceDescriptor(usbDeviceDescriptorFull,usbDeviceDescriptorFullSize);
    setUsbStringDescriptors(stringDescriptors);
    setConfigurationHandler(&usbVendorSpecificIFSetConfiguration);
    setClassSpecificSetupHandler(&usbVendorSpecificIFHandleClassSetupRequest);
    setSetInterfaceHandler(&usbVendorSpecificIFSetInterfaceHandler);
    setSuspendedHandler(&usbVendorSpecificIFSuspendedHandler);
}

static volatile uint8_t bmUsbStatus=0; // bit 0: usb cdc configured, bit 1: transfer in progress
static volatile uint8_t  receivedDataBfr[512];
static volatile uint16_t receivedDataLevel=0;
static volatile uint16_t receivedDataLevelSnapshot=0;
static volatile uint16_t sendDataSize=0;
static uint8_t  sendDataBfr[512];
static volatile uint8_t chunksSent=0;
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

    // wire up endpoint reception handler
    setEndpointOutHandler(&UsbVendorSpecificIFDataReceived,1);



    // wire up transfer done handler (used for blocking transfer)
    setTransferDoneHandler(&UsbVendorSpecificIFTransferDone,1);
    // enable reception of data at endpoint 1
    prepareUSBReception(1,USB_VS_DATA_OUT_PACKETSIZE);
    bmUsbStatus |= 1;
    
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

uint8_t usbVendorSpecificIFSetInterfaceHandler(uint16_t alternateSetting,uint16_t interfaceIndex)
{
    if (alternateSetting==0 && interfaceIndex == 2) // match dfu interface with alternate setting 0
    {
        setClassSpecificSetupHandler(usbDfuHandleClassSetupRequest); // switch over to dfu mode (required for dfu-util)
        prepareUSBTransfer(0,0,0);
    }
    else if (alternateSetting==0 && interfaceIndex == 1) // usb vendor speific interface, acknowledge
    {
        prepareUSBTransfer(0,0,0);
    }
    return 0;
}


/**
 * send data over usb vendor specific interface, 
 * data: pointer to the data to be sent
 * dlen: total length in bytes to send
 * blocking: doesn't return until data has been sent of blocking is 1
 * 
 * note: blocks if the dlen is larger than the internal buffer size (512)
 */
__QSPI_CODE
void usbVendorSpecificSendData(uint8_t * data,uint16_t dlen,uint8_t blocking)
{
    uint16_t bytesSent=0;
    chunksSent = 0;
    sendDataSize =dlen;
    if ((bmUsbStatus & (1 << USB_VS_Configured_Pos)) != 0 )
    {
        while((bmUsbStatus & (1 << USB_VS_TransferInProgress_Pos)) != 0);
    }
    while (bytesSent < dlen)
    {
        for (uint16_t c=0;c<dlen && c < 512;c++)
        {
            *(sendDataBfr + c) = *(data + c);
            bytesSent++;
        }
        if ((bmUsbStatus & (1 << USB_VS_Configured_Pos)) != 0 && (bmUsbStatus & (1 << USB_VS_TransferInProgress_Pos)) == 0)
        {
            if (sendDataSize > 64)
            {
                prepareUSBTransfer(1,sendDataBfr,64);
                sendDataSize -= 64;
            }
            else
            {
                prepareUSBTransfer(1,sendDataBfr,dlen);
                sendDataSize = 0;
            }
            bmUsbStatus |= (1 << USB_VS_TransferInProgress_Pos);
            if (blocking != 0 || bytesSent < dlen)
            {
                while((bmUsbStatus & (1 << USB_VS_TransferInProgress_Pos)) != 0);
            }
        }
    }
}


__QSPI_CODE
void UsbVendorSpecificIFTransferDone(void)
{
    chunksSent += 1;
    if (sendDataSize > 64)
    {
        prepareUSBTransfer(1,sendDataBfr+(chunksSent << 6),64);   
        sendDataSize -=64;
    }
    else if (sendDataSize > 0)
    {
        prepareUSBTransfer(1,sendDataBfr+(chunksSent << 6),sendDataSize);
        sendDataSize = 0;
    }
    else
    {
        bmUsbStatus &= ~(1 << USB_VS_TransferInProgress_Pos);
    }
}

__QSPI_CODE
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

__QSPI_CODE
uint16_t getUsbVendorSpecificReceivedDataLevel()
{
    return (receivedDataLevel - receivedDataLevelSnapshot) & USB_VS_RR_BUFFER_SIZE;
}

__QSPI_CODE
uint16_t readUsbVendorSpecificData(uint8_t * data,uint16_t startIndex)
{
    uint16_t c=0;
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
    (void)packet;
}

void usbVendorSpecificIFSuspendedHandler()
{
    bmUsbStatus = 0;
}