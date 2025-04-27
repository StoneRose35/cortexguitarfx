#include "stdint.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_common.h"
#include "drivers/usb.h"
#include "usb/usb_dfu.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "system.h"

extern volatile uint32_t task;
extern volatile uint8_t programChangeState;
const uint8_t usbDeviceDescriptorDfu[] = {
    0x12, // bLength
    SETUP_PACKET_DESCR_TYPE_DEVICE, // device descriptor type
    0x00, //bcdUSB, lsb
    0x01, //bcdUSB, msb
    0x00, // device class
    0x00, // device subclass
    0x00, // device protocol
    0x40, // max endpoint0 size
    0xfe, // vendor id, lsb
    0xca, // vendor id, msb
    0x42, // product id, lsb
    0x41, // product id, msb
    0x00,
    0x02, //bcdDevice
    0x01, // manufacturer string id
    0x02, // product string id
    0x03, // serial string id
    0x01, // max number of configurations
    } ;

const uint16_t usbDeviceDescriptorDfuSize = sizeof(usbDeviceDescriptorDfu);

const uint8_t usbConfigurationDescriptorDfu[] = {
    // configuration header
    // ------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_CONFIGURATION, // descriptor type configuration
    __LOBYTE(sizeof(usbDeviceDescriptorDfu)), // configation descriptor size, lsb
    __HIBYTE(sizeof(usbDeviceDescriptorDfu)), // configurator descriptor size, msb
    0x01, // bNumInterfaces
    0x01, // bConfigurationValue
    0x04, // configuration string id
    0x80, // bmAttributes, 0xC0 self powered, 0x80 bus powered
    0xFA, // bus power
    // ------------------------------
    //   Interface Descriptor, 
    // ------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, //bDescriptorType
    0x00, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x00, //bNumEndpoints
    0xFE, //bInterfaceClass
    0x01, //bInterfaceSubClass
    0x02, //bInterfaceProtocol
    0x05,  //iInterface
    //-------------------------------
    // Functional Descriptor
    //-------------------------------
    //------------------------------------
    // Interface functional descriptor
    //------------------------------------
    //------------------------------------
    0x09, //bLength
    0x21, //bDescriptorType
    (0 << 3) | (0 << 2) | (1 << 1 ) | ( 1 << 0), //bmAttributes: can download and upload only
    0xF0, //wDetachTimeOut, lsb
    0x00, //wDetachTimeOut, msb
    0x00, //wTransferSize, lsb
    0x02, //wTransferSize, msb
    0x10, // bcdDFUVersion
    0x01  // bcdDFUVersion
};

const uint16_t usbConfigurationDescriptorDfuSize = sizeof(usbConfigurationDescriptorDfu);


UsbStringDescriptorType stringDescriptorsDfu[] = {
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((12-1)*2)+2, .bString = "StoneRose35"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((9-1)*2)+2, .bString = "PiPicoFX"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((11-1)*2)+2, .bString = "3457456234"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((31-1)*2)+2, .bString = "QSPI DFU Programming Interface"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((18-1)*2)+2, .bString = "@Flash 0x08000000"},
};

uint8_t setUsbConfigurationDfu(uint8_t confNr)
{
    setupEndpoint(0,EP_DIRECTION_OUT,0x40);
    setEndpointOutHandler(0,0);
    return 0;
}


#ifdef USB_DFU
__RAMFUNC
void handleClassSetupRequest(const UsbSetupPacketType* packet)
{
    switch (packet->bRequest)
    {
    case SETUP_REQUEST_DFU_DETACH:
        setUsbDeviceDescriptor(usbConfigurationDescriptorDfu,usbDeviceDescriptorDfuSize);
        setUsbConfigurationDescriptor(usbConfigurationDescriptorDfu,usbConfigurationDescriptorDfuSize);
        setUsbStringDescriptors(stringDescriptorsDfu);
        setConfigurationHandler(&setUsbConfigurationDfu);
        task |= (1 << TASK_PREPARE_FOR_DFU);

        break;
    case SETUP_REQUEST_DFU_UPLOAD:
        break;
    case SETUP_REQUEST_DFU_DNLOAD: // from host to device
        uint16_t firmwareSize = packet->wLength;
        prepareUSBReception(0,firmwareSize);
        break;
    default:
        break;
    }
}
#endif

__RAMFUNC
void prepareSystemFroDFU()
{
            // disable interrupts
        // audio engine
        #ifdef EXTERNAL_CODEC
        #ifdef PCM3060_CODEC
        NVIC_ClearPendingIRQ(DMA1_Stream1_IRQn);
        NVIC_DisableIRQ(DMA1_Stream1_IRQn);
        #else
        NVIC_ClearPendingIRQ(DMA1_Stream0_IRQn);
        NVIC_DisableIRQ(DMA1_Stream0_IRQn);
        #endif
        #endif
        #ifdef INTERNAL_CODEC
        #ifdef PCM3060_CODEC
        NVIC_ClearPendingIRQ(DMA1_Stream1_IRQn);
        NVIC_DisableIRQ(DMA1_Stream1_IRQn);
        #else
        NVIC_ClearPendingIRQ(DMA1_Stream0_IRQn);
        NVIC_DisableIRQ(DMA1_Stream0_IRQn);
        #endif
        #endif
        //display update
        NVIC_ClearPendingIRQ(DMA1_Stream3_IRQn);
        NVIC_DisableIRQ(DMA1_Stream3_IRQn);

        // i2c
        NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
        NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
        NVIC_DisableIRQ(I2C1_EV_IRQn);
        NVIC_DisableIRQ(I2C1_ER_IRQn);
        task &= ~((1 << TASK_UPDATE_AUDIO_UI) | (1 << TASK_DISPLAY_NEXT_LINE) | (1 << TASK_I2C_DATA_RECEIVED));
        programChangeState = 0;

        while(1)
        {

        }
}

__RAMFUNC
void endPoint0DfuHandler(void*data,uint16_t dataSize)
{
 //Force NACK
 // process the data
 //

 // handle special case if data size is 0

}