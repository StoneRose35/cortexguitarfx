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

#define usb_hw_set hw_set_alias(usb_hw)
#define usb_hw_clear hw_clear_alias(usb_hw)


/** structures
 * 
*/
#define SETUP_PACKET_REQTYPE_DIR_HOST_TO_DEVICE 0
#define SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST (1<<7)
#define SETUP_PACKET_REQTYPE_STD 0
#define SETUP_PACKET_REQTYPE_CLASS 1
#define SETUP_PACKET_REQTYPE_VENDOR 2
#define SETUP_PAKET_RECIPIENT_DEVICE 0
#define SETUP_PAKET_RECIPIENT_INTERFACE 1
#define SETUP_PAKET_RECIPIENT_ENDPOINT 2
#define SETUP_PAKET_RECIPIENT_OTHER 3

#define SETUP_PACKET_REQ_CLEAR_FEATURE 1
#define SETUP_PACKET_REQ_GET_CONFIGURATION 8
#define SETUP_PACKET_REQ_GET_DESCRIPTOR 6
#define SETUP_PACKET_REQ_GET_STATUS 0
#define SETUP_PACKET_REQ_SET_ADDRESS 5
#define SETUP_PACKET_REQ_SET_CONFIGURATION 9
#define SETUP_PACKET_REQ_SET_DESCRIPTOR 7
#define SETUP_PACKET_REQ_SET_FEATURE 3
#define SETUP_PACKET_REQ_SET_INTERFACE 11
#define SETUP_PACKET_REQ_SYNCH_FRAME 12

#define SETUP_PACKET_DESCR_TYPE_DEVICE 1
#define SETUP_PACKET_DESCR_TYPE_CONFIGURATION 2
#define SETUP_PACKET_DESCR_TYPE_STRING 3
#define SETUP_PACKET_DESCR_TYPE_INTERFACE 4
#define SETUP_PACKET_DESCR_TYPE_ENDPOINT 5
#define SETUP_PACKET_DESCR_TYPE_DEVICE_QUALIFIER 6
#define SETUP_PACKET_DESCR_TYPE_OTHER_SPEED_CONFIG 7
#define SETUP_PACKET_DESCR_TYPE_INTERFACE_POWER 8
#define SETUP_PACKET_DESCR_TYPE_OTG 9

#define EP_BUFFER_CTRL_BUFFER_1_FULL 31
#define EP_BUFFER_CTRL_BUFFER_1_LAST 30
#define EP_BUFFER_CTRL_BUFFER_1_DATA_PID 29
#define EP_BUFFER_CTRL_DBFR_OFFSET 27
#define EP_BUFFER_CTRL_BUFFER_1_AVAIL 26
#define EP_BUFFER_CTRL_BUFFER_1_LEN 16

#define EP_BUFFER_CTRL_BUFFER_0_FULL 15
#define EP_BUFFER_CTRL_BUFFER_0_LAST 14
#define EP_BUFFER_CTRL_BUFFER_0_DATA_PID 13
#define EP_BUFFER_CTRL_RESET_BFR_SEL 12
#define EP_BUFFER_CTRL_STALL 11
#define EP_BUFFER_CTRL_BUFFER_0_AVAIL 10
#define EP_BUFFER_CTRL_BUFFER_0_LEN 0

typedef volatile struct
{
    uint8_t mbRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} UsbSetupPacketType;

typedef UsbSetupPacketType* UsbSetupPacket; 

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} UsbDeviceDescriptorType;

typedef UsbDeviceDescriptorType* UsbDeviceDescriptor;

typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} UsbConfigurationDescriptorType;

typedef UsbConfigurationDescriptorType* UsbConfigurationDescriptor;

typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} UsbInterfaceDescriptorType;

typedef UsbInterfaceDescriptorType* UsbInterfaceDescriptor;

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} UsbEndpointDescriptorType;

typedef UsbEndpointDescriptorType* UsbEndpointDescriptor;

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    char * bString;
} UsbStringDescriptorType;

typedef UsbStringDescriptorType* UsbStringDecriptor;

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t * wLangId;
} UsbString0DescriptorType;

typedef UsbString0DescriptorType* UsbString0Descriptor;

const UsbDeviceDescriptorType pipicofxUsbDevice = {
    .bLength=sizeof(UsbDeviceDescriptorType),
    .bDescriptorType=1,
    .bcdUSB=0x200,
    .bDeviceClass=239, // taken from tinyusb cdc_msc example config
    .bDeviceSubClass=2, // taken from tinyusb cdc_msc example config
    .bMaxPacketSize=32,
    .idVendor=0xcafe, // NOT OFFICIAL
    .idProduct=0x4003, // NOT OFFICIAL
    .bcdDevice=0x100, // taken from tinyusb cdc_msc example config
    .iManufacturer=1, // taken from tinyusb cdc_msc example config
    .iProduct=2, // taken from tinyusb cdc_msc example config
    .iSerialNumber=3, // taken from tinyusb cdc_msc example config
    .bNumConfigurations=1 // taken from tinyusb cdc_msc example config
};

// usb controller interrupt routine
void isr_usbctrl_irq5()
{
    uint32_t stat;
    stat = usb_hw->ints;
    UsbSetupPacket setupPacket;

    // setup case
    if ((stat & (1 << USB_INTS_SETUP_REQ_LSB)) != 0)
    {
        // gather setup packet
        setupPacket = (UsbSetupPacket)usb_dpram->setup_packet;
        usb_hw_clear->sie_status = (1 << USB_SIE_STATUS_SETUP_REC_LSB);
        
        if ((setupPacket->mbRequestType & (SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST))==(SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST))
        {
            switch (setupPacket->bRequest)
            {
                case SETUP_PACKET_REQ_GET_CONFIGURATION:
                    break;
                case SETUP_PACKET_REQ_GET_DESCRIPTOR:
                    switch (setupPacket->wValue)
                    {
                        case (SETUP_PACKET_DESCR_TYPE_DEVICE << 8):
                            // put device descriptor into buffer for ep0
                            memcpy(usb_dpram->ep0_buf_a,&pipicofxUsbDevice,sizeof(UsbDeviceDescriptorType));

                            // set pid to 1, available and buffer control full
                            usb_dpram->ep_buf_ctrl[0].in = (1 << EP_BUFFER_CTRL_BUFFER_0_DATA_PID) | (1 << EP_BUFFER_CTRL_BUFFER_0_AVAIL) | (sizeof(UsbDeviceDescriptorType) << EP_BUFFER_CTRL_BUFFER_0_LEN);
                            break;
                        case (SETUP_PACKET_DESCR_TYPE_CONFIGURATION << 8):
                            break;
                        case (SETUP_PACKET_DESCR_TYPE_STRING<<8):
                            break;
                        default:
                            break;
                    }


                    break;
                case SETUP_PACKET_REQ_GET_STATUS:
                    break;
                case SETUP_PACKET_REQ_SYNCH_FRAME:
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch(setupPacket->bRequest)
            {
                case SETUP_PACKET_REQ_CLEAR_FEATURE:
                    break;
                case SETUP_PACKET_REQ_SET_ADDRESS:
                    break;
                case SETUP_PACKET_REQ_SET_CONFIGURATION:
                    break;
                case SETUP_PACKET_REQ_SET_DESCRIPTOR:
                    break;
                case SETUP_PACKET_REQ_SET_FEATURE:
                    break;
                case SETUP_PACKET_REQ_SET_INTERFACE:
                    break;
                default:
                    break;
            }
        }
    }
    if ((stat & (1 << USB_INTS_BUFF_STATUS_LSB)) != 0) // when a buffer is transferred
    {
        uint32_t bfrstatus = usb_hw->buf_status;
        //usb_hw->buf_status = 0xFFFFFFFF;
        for(uint8_t c=0;c<16;c+=2)
        {
            if ((bfrstatus & (1 << c))!=0)
            {
                // handle in endpoint transferred
                // endpoint nr is c>>1
                usb_hw_clear->buf_status = (1 << c);
            }
            if ((bfrstatus & (1 << (c+1)))!=0)
            {
                // handle out endpoint transferred
                // endpoint nr is c>>1
                usb_hw_clear->buf_status = (1 << (c+1));
            }
        }
    }
    if ((stat & (1 << USB_INTS_BUFF_STATUS_LSB)) != 0) // bus reset
    {
        usb_hw_clear->sie_status = (1 << USB_SIE_STATUS_BUS_RESET_LSB);
        usb_hw->dev_addr_ctrl=0;
    }

    // 

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


    // enable a few interrupt bits in the mask register in order to handle setup
    // bus reset: probably set the address 0 zero and clear buffers
    // setup req: handle setup packets
    // buffer status: to be generally notified when a transfer buffer has been processed
    usb_hw->inte |= (1 << USB_INTE_BUS_RESET_LSB) | (1 << USB_INTE_SETUP_REQ_LSB) | (1 << USB_INTE_BUFF_STATUS_LSB);

    // connect to physical endpoint and enable softcon
    usb_hw->muxing |= (1 << USB_USB_MUXING_SOFTCON_LSB) | (1 << USB_USB_MUXING_TO_PHY_LSB);
    
    // enable the usb interrupt on the processor level
    *NVIC_ISER = (1 << 5);
    
    // finally enable the usb controller
    usb_hw->main_ctrl |= (1 << USB_MAIN_CTRL_CONTROLLER_EN_LSB);


}