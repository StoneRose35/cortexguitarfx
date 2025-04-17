#include "stdint.h"
#include "usb/usb_common.h"
#include "drivers/usb.h"
#include "memoryRegions.h"

const uint8_t usbDeviceDescriptorDfu[] = {
    0x12, // bLength
    SETUP_PACKET_DESCR_TYPE_DEVICE, // device descriptor type
    0x00,
    0x02, //bcdUSB
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

const uint8_t usbConfigurationDescriptorDfu[] = {
    // configuration header
    // ------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_CONFIGURATION, // descriptor type configuration
    __LOBYTE(sizeof(usbDeviceDescriptorDfu)), // configation descriptor size, lsb
    __HIBYTE(sizeof(usbDeviceDescriptorDfu)), // configurator descriptor size, msb
    0x01, // bNumInterfaces
    0x01, // bConfigurationValue
    0x05, // configuration string id
    0x80, // bmAttributes, 0xC0 self powered, 0x80 bus powered
    0xFA, // bus power
    // ------------------------------
    //   Interface Descriptor
    // ------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, //bDescriptorType
    0x00, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x00, //bNumEndpoints
    0xFE, //bInterfaceClass
    0x01, //bInterfaceSubClass
    0x02, //bInterfaceProtocol
    0x04  //iInterface
};

UsbString0DescriptorType string0DescriptorDfu = {
    .bLength = 4,
    .bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING,
    .wLangId = 0x0409
};

UsbStringDescriptorType stringDescriptorsDfu[] = {
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((12-1)*2)+2, .bString = "StoneRose35"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((9-1)*2)+2, .bString = "PiPicoFX"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((11-1)*2)+2, .bString = "3457456234"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((18-1)*2)+2, .bString = "@Flash 0x08000000"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((31-1)*2)+2, .bString = "QSPI DFU Programming Interface"},
};
