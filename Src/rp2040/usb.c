#include "stdint.h"
#include "hardware/regs/clocks.h"
#include "hardware/regs/pll.h"
#include "hardware/regs/addressmap.h"
#include "hardware/address_mapped.h"
#include "hardware/rp2040_registers.h"
#include "hardware/structs/usb.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"
#include "systemClock.h"
#include "memFunctions.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "hardware/usb_rp2040.h"
#include "consoleBase.h"
#include "stringFunctions.h"
#include "bufferedInputHandler.h"


extern CommBuffer usbBuffer;

void generateStringDescriptor(UsbEndpointConfigurationType* ep, UsbStringDescriptor descr,const char * str)
{
    uint8_t c=0;
    while (*(str+(c >> 1))!=0)
    {
        *(ep->buffer + c + 2)= *(str+(c >> 1));
        c++;
        *(ep->buffer+ c + 2)=0;
        c++;

    }
    *(ep->buffer)=c+2;
    *(ep->buffer+1)=SETUP_PACKET_DESCR_TYPE_STRING;
    descr->bLength=c+2;
    descr->bDescriptorType=SETUP_PACKET_DESCR_TYPE_STRING;
}

const UsbDeviceDescriptorType pipicofxUsbDeviceDescriptor = {
    .bLength=DEVICE_DESCRIPTOR_LENGTH,
    .bDescriptorType=1,
    .bcdUSB=0x200,
    .bDeviceClass=2, //239, // taken from tinyusb cdc_msc example config
    .bDeviceSubClass=2, // taken from tinyusb cdc_msc example config
    .bMaxPacketSize=64,
    .idVendor=0xcafe, // NOT OFFICIAL
    .idProduct=0x4003, // NOT OFFICIAL
    .bcdDevice=0x100, // taken from tinyusb cdc_msc example config
    .iManufacturer=1, // taken from tinyusb cdc_msc example config
    .iProduct=2, // taken from tinyusb cdc_msc example config
    .iSerialNumber=3, // taken from tinyusb cdc_msc example config
    .bNumConfigurations=1, // taken from tinyusb cdc_msc example config
    .bDeviceProtocol=0
};

const uint8_t pipicofxfc[] = {
    // configuration descriptor
CONFIGURATION_DESCRIPTOR_LENGTH,2,CONFIGURATION_DESCRIPTOR_LENGTH 
                    + INTERFACE_DESCRIPTOR_LENGTH*2 
                    + ENDPOINT_DESCRIPTOR_LENGTH*3
                    + 19,0x0,2,1,4,0x80,50,
    // cdc control interface descriptor
    INTERFACE_DESCRIPTOR_LENGTH,4,0,0,1,2,2,0,5,
    // cdc cd descriptors
    5, 0x24, CDC_FUNC_DESC_HEADER, 0x20, 0x1, // CDC Header, version 1.2
    5, 0x24, CDC_FUNC_DESC_CALL_MANAGEMENT, 0, 1, // call management: no capabilities, data interface 1
    4, 0x24, CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT, 2, // 2: 0Device supports the request combination of Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and the notification Serial_State.
    5, 0x24, CDC_FUNC_DESC_UNION, 0, 1, // control interface is interface 0, subordinate interface is interface 1
    // EP 1 in: control endpoit
    ENDPOINT_DESCRIPTOR_LENGTH,5,0x81,(ENDPOINT_ATTR_TRANSFERTYPE_INTERRUPT << ENDPOINT_ATTR_TRANSFERTYPE_POS) |
                    (ENDPOINT_ATTR_SYNC_TYPE_NO_SYNC << ENDPOINT_ATTR_SYNC_TYPE_POS) |
                    (ENDPOINT_ATTR_USAGE_DATA << ENDPOINT_ATTR_USAGE_POS),0x8,0x0,16,
    // cdc data interface
    INTERFACE_DESCRIPTOR_LENGTH,4,1,0,2,10,0,0,6,
    // data endpoint in
    ENDPOINT_DESCRIPTOR_LENGTH,5,0x82,(ENDPOINT_ATTR_TRANSFERTYPE_BULK << ENDPOINT_ATTR_TRANSFERTYPE_POS) |
                    (ENDPOINT_ATTR_SYNC_TYPE_NO_SYNC << ENDPOINT_ATTR_SYNC_TYPE_POS) |
                    (ENDPOINT_ATTR_USAGE_DATA << ENDPOINT_ATTR_USAGE_POS),CDC_DATA_MAX_PACKET_SIZE_IN,0x0,0,
    // data endpoint out
    ENDPOINT_DESCRIPTOR_LENGTH,5,0x2,(ENDPOINT_ATTR_TRANSFERTYPE_BULK << ENDPOINT_ATTR_TRANSFERTYPE_POS) |
                    (ENDPOINT_ATTR_SYNC_TYPE_NO_SYNC << ENDPOINT_ATTR_SYNC_TYPE_POS) |
                    (ENDPOINT_ATTR_USAGE_DATA << ENDPOINT_ATTR_USAGE_POS),CDC_DATA_MAX_PACKET_SIZE_OUT,0x0,0,
};

const uint8_t pipicoString0Descriptor[] = {
    4,3,9,4
};

const uint8_t status[]={0x0, 0x0};

char const* pipicofxStringDescriptors[] = {
    "Stonerose35", // 1: Manufacturer 
    "PiPicoFX", // 2: Product
    "123456789012", // 3: Serial Number
    "Default Config", // 4: Configuration
    "CDC Control", // Interface
    "CDC Data" // Interface
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


UsbEndpointConfigurationType ep0In;
UsbEndpointConfigurationType ep0Out;
UsbEndpointConfigurationType ep1In;
UsbEndpointConfigurationType ep2In;
UsbEndpointConfigurationType ep2Out;

static volatile uint16_t deviceAddress;
static volatile uint8_t setAddress=0;
static uint8_t epBfr[256];
static volatile uint8_t configured=0;
static UsbMultipacketTransfer transferhandler= {
    .address=0,
    .bMaxPacketSize=0,
    .idx=0,
    .len=0,
    .transferInProgress=0
};

static UsbMultipacketTransfer transferHandlerCDC = {
    .address=0,
    .bMaxPacketSize=0,
    .idx=0,
    .len=0,
    .transferInProgress=0
};

// usb controller interrupt routine
void isr_usbctrl_irq5()
{
    uint32_t stat;
    stat = usb_hw->ints;
    UsbSetupPacket setupPacket;
    UsbStringDescriptorType stringdescr;
    volatile uint8_t descrType;
    volatile uint8_t descrIndex;
    uint16_t transferLength;
    #ifdef USB_DBG
    char charbfr[8];
    #endif


    if ((stat & (1 << USB_INTS_BUFF_STATUS_LSB)) != 0) // when a buffer is transferred
    {
        uint32_t bfrstatus = usb_hw->buf_status;
        for(uint8_t c=0;c<16;c+=2)
        {
            if ((bfrstatus & (1 << c))!=0) // IN transfer done
            {
                if (usb_hw->sie_status & USB_SIE_STATUS_ACK_REC_BITS)
                {
                    #ifdef USB_DBG
                    printf("ACK rec.\r\n");
                    #endif
                    usb_hw_clear->sie_status = USB_SIE_STATUS_ACK_REC_BITS;
                }
                // handle in endpoint transferred
                // endpoint nr is c>>1
                usb_hw_clear->buf_status = (1 << c);
                if ((c>>1)==0) // endpoint 0
                {
                    if(setAddress)
                    {
                        usb_hw->dev_addr_ctrl=deviceAddress;
                        setAddress=0;
                    }
                    else if (transferhandler.transferInProgress==1) // handle multi-packet transfers
                    {
                        send_next_packet(&ep0In,&transferhandler);
                    }
                    else
                    {
                        #ifdef USB_DBG
                        printf("out0 req\r\n");
                        #endif
                        usb_start_out_transfer(&ep0Out,0);
                    }
                }
                else if ((c>>1)==2)
                {
                    if(transferHandlerCDC.transferInProgress==1)
                    {
                        send_next_packet(&ep2In,&transferHandlerCDC);
                    }
                }
                else
                {
                    #ifdef USB_DBG
                    printf("ep");
                    UInt8ToChar(c>>1,charbfr);
                    printf(charbfr);
                    printf(" in done\r\n");
                    #endif
                }
            }
            if ((bfrstatus & (1 << (c+1)))!=0)
            {
                // handle OUT endpoint transferred
                // endpoint nr is c>>1
                usb_hw_clear->buf_status = (1 << (c+1));
                if ((c>>1)==0) // EP 0 
                {
                    #ifdef USB_DBG
                    printf("out0 rec\r\n");
                    #endif
                }
                else if ((c>>1)==2) // CDC Data
                {
                    transferLength = *ep2Out.ep_buf_ctrl & 0x3FF;
                    appendToInputBufferReverse(usbBuffer, ep2Out.buffer ,transferLength);
                    usb_start_out_transfer(&ep2Out,64);
                
                }
                else
                {
                    #ifdef USB_DBG
                    printf("ep");
                    UInt8ToChar(c>>1,charbfr);
                    printf(charbfr);
                    printf(" out done\r\n");
                    #endif
                } 
                
            }
        }
    }
    // setup case
    if ((stat & (1 << USB_INTS_SETUP_REQ_LSB)) != 0)
    {
        // gather setup packet
        ep0In.pid=1;
        setupPacket = (UsbSetupPacket)usb_dpram->setup_packet;
        usb_hw_clear->sie_status = (1 << USB_SIE_STATUS_SETUP_REC_LSB);
        
        if ((setupPacket->mbRequestType & (SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST))==(SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST))
        {
            switch (setupPacket->bRequest)
            {
                case SETUP_PACKET_REQ_GET_CONFIGURATION:
                    break;
                case SETUP_PACKET_REQ_GET_DESCRIPTOR:
                    descrType=setupPacket->wValue >> 8; 
                    descrIndex=setupPacket->wValue & 0xFF;
                    switch (descrType)
                    {
                        case (SETUP_PACKET_DESCR_TYPE_DEVICE):
                            #ifdef USB_DBG
                            printf("get device descriptor\r\n");
                            #endif
                            usb_start_in_transfer(&ep0In,(uint8_t*)&pipicofxUsbDeviceDescriptor,DEVICE_DESCRIPTOR_LENGTH);
                            break;
                        case (SETUP_PACKET_DESCR_TYPE_CONFIGURATION):
                            if (setupPacket->wLength == CONFIGURATION_DESCRIPTOR_LENGTH)
                            {   
                                #ifdef USB_DBG
                                printf("get configuration descriptor\r\n");
                                #endif
                                usb_start_in_transfer(&ep0In,(uint8_t*)&pipicofxfc,CONFIGURATION_DESCRIPTOR_LENGTH);
                            }
                            else // send full configuration descriptor, divide into pieces of bMaxPacketSize length
                            {
                                transferhandler.address=(uint32_t)pipicofxfc;
                                transferhandler.bMaxPacketSize=pipicofxUsbDeviceDescriptor.bMaxPacketSize;
                                transferhandler.idx=0;
                                transferhandler.len=*((uint16_t*)(pipicofxfc + 2));
                                transferhandler.transferInProgress=1;
                                #ifdef USB_DBG
                                printf("get full configuration descriptor\r\n");
                                #endif
                                send_next_packet(&ep0In,&transferhandler);
                            }
                            break;
                        case (SETUP_PACKET_DESCR_TYPE_STRING):
                            if (descrIndex==0)
                            {
                                #ifdef USB_DBG
                                printf("get lang string descriptor\r\n");
                                #endif
                                memcpy(epBfr,pipicoString0Descriptor,4);
                                usb_start_in_transfer(&ep0In,epBfr,4);
                            }
                            else 
                            {
                                #ifdef USB_DBG
                                printf("get string descriptor ");
                                UInt16ToChar(descrIndex,charbfr);
                                printf(charbfr);
                                printf("\r\n");
                                #endif
                                generateStringDescriptor(&ep0In,&stringdescr,pipicofxStringDescriptors[descrIndex-1]);
                                usb_start_in_transfer(&ep0In,0,stringdescr.bLength);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case SETUP_PACKET_REQ_GET_STATUS:
                    usb_start_in_transfer(&ep0In,status,2);
                    break;
                case SETUP_PACKET_REQ_SYNCH_FRAME:
                    break;
                case SETUP_PACKET_REQ_CDC_GET_LINE_CODING:
                    usb_start_in_transfer(&ep0In,(const uint8_t*)&getcoding,7);
                    break;
                default:
                    break;
            }
        }
        else // Host to Device
        {
            switch(setupPacket->bRequest)
            {
                case SETUP_PACKET_REQ_CLEAR_FEATURE:
                    break;
                case SETUP_PACKET_REQ_SET_ADDRESS:
                    deviceAddress = setupPacket->wValue;
                    setAddress=1;
                    #ifdef USB_DBG
                    printf("set address received, address: ");
                    
                    UInt16ToChar(deviceAddress,charbfr);
                    printf(charbfr);
                    printf("\r\n");
                    #endif
                    usb_start_in_transfer(&ep0In,0,0); // send empty package to acknowledge
                    break;
                case SETUP_PACKET_REQ_SET_CONFIGURATION:
                    #ifdef USB_DBG
                    printf("set configuration\r\n");
                     #endif
                     configured=1;
                    usb_start_in_transfer(&ep0In,0,0); // send empty package to acknowledge

                    // CDC-specifix
                    // get ready to receive data in ep2
                    usb_start_out_transfer(&ep2Out,64);
                    break;
                case SETUP_PACKET_REQ_SET_DESCRIPTOR:
                    //break;
                case SETUP_PACKET_REQ_SET_FEATURE:
                    //break;
                case SETUP_PACKET_REQ_SET_INTERFACE:
                    //break;

                case SETUP_PACKET_REQ_CDC_SET_LINE_CODING:
                    // configure physical uart port behind if present
                    // in our case ignore 
                    usb_start_in_transfer(&ep0In,0,0); 
                    break;
                case SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE:
                    // arm both rs232 control line DTR and RTS in case they are present
                    // ignore as well
                    usb_start_in_transfer(&ep0In,0,0); 
                    break;
                default:
                    //blindly acknowledge at first...
                    #ifdef USB_DBG
                    printf("got unknown request \r\n");
                    UInt8ToChar(setupPacket->bRequest,charbfr);
                    printf(charbfr);
                    printf("\n");
                    #endif
                    usb_start_in_transfer(&ep0In,0,0); // send empty package to acknowledge
                    break;
            }
        }
    }
    if ((stat & (1 << USB_INTS_BUS_RESET_LSB)) != 0) // bus reset
    {
        #ifdef USB_DBG
        printf("usb reset\r\n");
        #endif
        usb_hw_clear->sie_status = (1 << USB_SIE_STATUS_BUS_RESET_LSB);
        usb_hw->dev_addr_ctrl=0;
        setAddress=0;
    }

    if ((stat & (1 << USB_INTS_ERROR_DATA_SEQ_LSB)) != 0)
    {
        usb_hw_clear->sie_status = (1 << USB_SIE_STATUS_DATA_SEQ_ERROR_LSB);
    }
    // 
}


void usb_start_in_transfer(UsbEndpointConfigurationType * ep,const uint8_t * data,uint8_t len)
{
    uint32_t ctrlval;
    if (data)
    {
        memcpy(ep->buffer,data,len);
    }
    ctrlval = len | (1 << EP_BUFFER_CTRL_BUFFER_0_FULL) | (1 << EP_BUFFER_CTRL_RESET_BFR_SEL);
    ctrlval |= ep->pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    ep->pid ^=1;
    *ep->ep_buf_ctrl = ctrlval;
    __asm volatile (
            "b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1:\n"
            : : : "memory");
    ctrlval |= (1 << EP_BUFFER_CTRL_BUFFER_0_AVAIL);
    *ep->ep_buf_ctrl = ctrlval;    
}

void usb_start_out_transfer(UsbEndpointConfigurationType * ep,uint8_t len)
{
    uint32_t ctrlval;
    ctrlval = len | (0 << EP_BUFFER_CTRL_BUFFER_0_FULL);
    ctrlval |= ep->pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    ep->pid ^=1;
    *ep->ep_buf_ctrl = ctrlval;
    __asm volatile (
            "b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1: b 1f\n"
            "1:\n"
            : : : "memory");
    ctrlval |= (1 << EP_BUFFER_CTRL_BUFFER_0_AVAIL);
    *ep->ep_buf_ctrl = ctrlval;
}

void send_next_packet(UsbEndpointConfigurationType* ep,UsbMultipacketTransfer* th)
{

    if ((th->idx + th->bMaxPacketSize) > th->len)
    {
        usb_start_in_transfer(ep,(const uint8_t*)(th->address+th->idx),th->len-th->idx);
        th->idx=th->len;
        th->transferInProgress=0;
    } 
    else if ((th->idx + th->bMaxPacketSize) == th->len)
    {
        usb_start_in_transfer(ep,(const uint8_t*)(th->address+th->idx),th->bMaxPacketSize);
        th->idx=th->len;
        th->transferInProgress=1; // send a zero-length packet in the end
    }
    else
    {
        usb_start_in_transfer(ep,(const uint8_t*)(th->address+th->idx),th->bMaxPacketSize);
        th->idx += th->bMaxPacketSize;
        th->transferInProgress=1;
    }
}


void initUsbCdcDevice()
{
    ep0In.buffer=usb_dpram->ep0_buf_a;
    ep0Out.buffer=usb_dpram->ep0_buf_a;
    ep0In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[0].in);
    ep0Out.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[0].out);
    ep0In.pid=0;
    ep0Out.pid=0;

    ep1In.buffer=usb_dpram->epx_data;
    ep1In.pid=0;
    ep1In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[1].in);
    usb_dpram->ep_ctrl[1-1].in = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep1In.buffer - (uint32_t)usb_dpram) | (3 << EP_CTRL_EP_TYPE) | (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);

    ep2In.buffer=usb_dpram->epx_data+64;
    ep2In.pid=0;
    ep2In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[2].in);
    usb_dpram->ep_ctrl[2-1].in = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep2In.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);

    ep2Out.buffer=usb_dpram->epx_data+64*2;
    ep2Out.pid=0;
    ep2Out.ep_buf_ctrl=&usb_dpram->ep_buf_ctrl[2].out;
    usb_dpram->ep_ctrl[2-1].out = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep2Out.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);

}

void startUsbCdcDevice()
{
    if (configured)
    {
        usb_start_out_transfer(&ep2Out,64); // get ready to receive data
    }
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
    send_next_packet(&ep2In,&transferHandlerCDC);
    consumeOutputBufferBytes(bfr,len);

    if(blocking)
    {
        while (transferHandlerCDC.transferInProgress || (*ep2In.ep_buf_ctrl & (1 << EP_BUFFER_CTRL_BUFFER_0_FULL)));
    }
}

/**
 * @brief initializes the usb hardware
 *        restarts the usb pll of not locked
 *        exposes a single communication device 
 */
void initUSB()
{

    // provide clock signal to the usb controller
    if ((PLL_USB->cs & (1 << PLL_CS_LOCK_LSB))==0)
    {
        initUsbPll();
    }
    *CLK_USB_CTRL &= ~(0x7 << CLOCKS_CLK_USB_CTRL_AUXSRC_LSB);
    *CLK_USB_CTRL |= (1 << CLOCKS_CLK_USB_CTRL_ENABLE_LSB);


    // reset the usb controller
    *RESETS |= (1 << RESETS_RESET_USBCTRL_LSB);
    *RESETS &= ~(1 << RESETS_RESET_USBCTRL_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_USBCTRL_LSB)) == 0);


    // clear dpram
    memset(usb_dpram,0,4096);


    initUsbCdcDevice();

    //
    // Hardware configuration, when done the usb controller listens th interrupts and works in device mode
    //
    // set address to zero
    usb_hw->dev_addr_ctrl = 0;

    // set to device mode, disable
    usb_hw->main_ctrl &= ~((1 << USB_MAIN_CTRL_HOST_NDEVICE_LSB)|(1 << USB_MAIN_CTRL_CONTROLLER_EN_LSB));

    // set endpoint0 to single buffered, set bit in BUFFER_STATUS on every buffer completed
    // enable pullup
    usb_hw->sie_ctrl |= (1 << USB_SIE_CTRL_EP0_INT_1BUF_LSB)| (1 << USB_SIE_CTRL_PULLUP_EN_LSB);

    // virtually connect the device to a host
    usb_hw->pwr =  (1 << USB_USB_PWR_VBUS_DETECT_LSB) | (1 << USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_LSB);

    // enable a few interrupt bits in the mask register in order to handle setup
    // bus reset: probably set the address 0 zero and clear buffers
    // setup req: handle setup packets
    // buffer status: to be generally notified when a transfer buffer has been processed
    // data sequence error: for debugging purposes
    usb_hw->inte |= (1 << USB_INTE_BUS_RESET_LSB) 
                  | (1 << USB_INTE_SETUP_REQ_LSB) 
                  | (1 << USB_INTE_BUFF_STATUS_LSB)
                  | (1 << USB_INTE_ERROR_DATA_SEQ_LSB);

    // connect to physical endpoint and enable softcon
    usb_hw->muxing |= (1 << USB_USB_MUXING_SOFTCON_LSB) | (1 << USB_USB_MUXING_TO_PHY_LSB);
    
    // enable the usb interrupt on the processor level
    *NVIC_ISER = (1 << 5);
    
    // finally enable the usb controller
    usb_hw->main_ctrl |= (1 << USB_MAIN_CTRL_CONTROLLER_EN_LSB);

}