#include "stdint.h"
#include "hardware/usb_rp2040.h"
#include "usb/usb_cdc.h"

#include "usb/usb_common.h"
#include "usb/usb_config.h"
#include "bufferedInputHandler.h"

#ifdef USB_CDC_DRIVER
UsbEndpointConfigurationType ep2In;
UsbEndpointConfigurationType ep2Out;
UsbEndpointConfigurationType ep1In;

extern CommBufferType usbCommBuffer;
extern uint8_t epBfr[256];

static void CDCDataInHandler(void);
static void CDCDataOutHandler(void);
static void startCDCDataReception();

static UsbMultipacketTransfer transferHandlerCDC = {
    .address=0,
    .bMaxPacketSize=0,
    .idx=0,
    .len=0,
    .transferInProgress=0
};

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


void initUsbDeviceDriver(UsbEndpointConfigurationType ** epsIn,UsbEndpointConfigurationType ** epsOut,void(**onConfigured)(void))
{

    ep1In.buffer=usb_dpram->epx_data;
    ep1In.pid=0;
    ep1In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[1].in);
    ep1In.epHandler=0;
    usb_dpram->ep_ctrl[1-1].in = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep1In.buffer - (uint32_t)usb_dpram) | (3 << EP_CTRL_EP_TYPE) | (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);
    epsIn[1]=&ep1In;

    ep2In.buffer=usb_dpram->epx_data+64;
    ep2In.pid=0;
    ep2In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[2].in);
    ep2In.epHandler=CDCDataInHandler;
    usb_dpram->ep_ctrl[2-1].in = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep2In.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);
    epsIn[2]=&ep2In;

    ep2Out.buffer=usb_dpram->epx_data+64*2;
    ep2Out.pid=0;
    ep2Out.ep_buf_ctrl=&usb_dpram->ep_buf_ctrl[2].out;
    ep2Out.epHandler=CDCDataOutHandler;
    epsOut[2]=&ep2Out;
    usb_dpram->ep_ctrl[2-1].out = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep2Out.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);

    *onConfigured=startCDCDataReception;

}


static void startCDCDataReception()
{
    usb_start_out_transfer(&ep2Out,64); // get ready to receive data
}

void sendOverUsb(void * commBfr,uint8_t blocking)
{
	uint32_t len;
	uint32_t offset;
	CommBuffer bfr = (CommBuffer)commBfr;
	getOutputBuffer(bfr,&len,&offset);

    while (transferHandlerCDC.transferInProgress || (*ep2In.ep_buf_ctrl & (1 << EP_BUFFER_CTRL_BUFFER_0_FULL)));
    for (uint16_t c4=0;c4<len;c4++)
    {
        *(epBfr+c4) = *(bfr->outputBuffer + ((offset + c4) & ((1 << OUTPUT_BUFFER_SIZE)-1))); 
    }

    transferHandlerCDC.bMaxPacketSize = CDC_DATA_MAX_PACKET_SIZE_IN;
    transferHandlerCDC.address=(uint32_t)epBfr;
    transferHandlerCDC.idx=0;
    transferHandlerCDC.len=len;
    send_next_packet(&ep2In,&transferHandlerCDC,0);
    consumeOutputBufferBytes(bfr,len);

    if(blocking)
    {
        while (transferHandlerCDC.transferInProgress || (*ep2In.ep_buf_ctrl & (1 << EP_BUFFER_CTRL_BUFFER_0_FULL)));
    }
}

uint8_t handleSetupRequestOut(UsbSetupPacket pck,UsbEndpointConfigurationType * ep)
{
    uint8_t handled=0;
    switch (pck->bRequest)
    {
        case SETUP_PACKET_REQ_CDC_SET_LINE_CODING:
            // configure physical uart port behind if present
            // in our case ignore 
            usb_start_in_transfer(ep,0,0);
            handled=1;
            break;
        case SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE:
            // arm both rs232 control line DTR and RTS in case they are present
            // ignore as well
            usb_start_in_transfer(ep,0,0); 
            handled=1;
            break;
        default:
            break;
    }
    return handled;
}

uint8_t handleSetupRequestIn(UsbSetupPacket pck,UsbEndpointConfigurationType * ep)
{
    uint8_t handled=0;
    if (pck->bRequest == SETUP_PACKET_REQ_CDC_GET_LINE_CODING)
    {
        usb_start_in_transfer(ep,(const uint8_t*)&getcoding,7);
        handled=1;
    }
    return handled;
}

static void CDCDataInHandler(void)
{
    if(transferHandlerCDC.transferInProgress==1)
    {
        send_next_packet(&ep2In,&transferHandlerCDC,0);
    }
}

static void CDCDataOutHandler()
{
    uint16_t transferLength;
    transferLength = *ep2Out.ep_buf_ctrl & 0x3FF;
    appendToInputBufferReverse(&usbCommBuffer, ep2Out.buffer ,transferLength);
    usb_start_out_transfer(&ep2Out,64);
}
#endif