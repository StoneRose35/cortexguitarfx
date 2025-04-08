#include "stdint.h"
#include "drivers/usb.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "stm32h750/stm32h750xx.h"

void setUsbConfiguration(uint8_t confNr)
{
    USB_OTG_INEndpointTypeDef * inEndpoint;
    USB_OTG_OUTEndpointTypeDef * outEndpoint;
    // IN Endpoint 2: Interrupt, Packet Size 8, fifo 1
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 2*0x20));
    inEndpoint->DIEPCTL = (1 << (USB_OTG_DIEPCTL_CNAK_Pos)) 
                            | (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_INTERRUPT << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (2 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (8 << (USB_OTG_DIEPCTL_MPSIZ_Pos));

    // OUT Endpoint 1: Bulk, Packet Size 64
    outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 1*0x20));
    outEndpoint->DOEPCTL = (1 << (USB_OTG_DOEPCTL_CNAK_Pos)) 
                            | (1 << (USB_OTG_DOEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DOEPCTL_EPTYP_Pos))
                            | (64 << (USB_OTG_DIEPCTL_MPSIZ_Pos));

    // IN Endpoint 1: Bulk, Packet Size 64, fifo 2
    inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 1*0x20));
    inEndpoint->DIEPCTL = (1 << (USB_OTG_DIEPCTL_CNAK_Pos)) 
                            | (1 << (USB_OTG_DIEPCTL_USBAEP_Pos)) 
                            | (EPTYP_BULK << (USB_OTG_DIEPCTL_EPTYP_Pos))
                            | (2 << (USB_OTG_DIEPCTL_TXFNUM_Pos))
                            | (64 << (USB_OTG_DIEPCTL_MPSIZ_Pos));
}

void handleClassSetupRequest(const UsbSetupPacketType* packet)
{
    switch (packet->bRequest)
    {
    case SETUP_PACKET_REQ_CDC_GET_LINE_CODING:
        break;
    case SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE:
        break;
    case SETUP_PACKET_REQ_CDC_SET_LINE_CODING:
        break;
    default:
        break;
    }
}

void handleVendorSetupRequest(const UsbSetupPacketType* packet)
{

}