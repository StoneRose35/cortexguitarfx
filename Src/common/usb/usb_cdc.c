#include "stdint.h"
#include "drivers/usb.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "stm32h750/stm32h750xx.h"
#include "systick.h"


const struct __attribute__((packed))
{
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;

} getcoding=
{
    .dwDTERate=115200,
    .bCharFormat=0,
    .bParityType=0,
    .bDataBits=8
};

volatile uint8_t bmUsbStatus=0; // bit 0: usb cdc configured, bit 1: transfer in progress
volatile uint8_t  receivedDataBfr[128];
volatile uint16_t receivedDataLevel=0;
uint8_t setUsbConfiguration(uint8_t confNr)
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

    setEndpointOutHandler(&UsbCdcEp0OutHandler,0);

    // wire up transfer done handler (used for blocking transfer)
    setTransferDoneHandler(&UsbCdcTransferDone,1);
    // enable reception of data at endpoint 1
    prepareUSBReception(1,USB_CDC_DATA_OUT_PACKETSIZE);
    bmUsbStatus |= 1;
    
    return 0;
}

void handleClassSetupRequest(const UsbSetupPacketType* packet)
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
    default:
        break;
    }
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