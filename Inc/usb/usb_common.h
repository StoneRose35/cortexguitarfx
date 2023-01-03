#ifndef USB_COMMON_H_
#define USB_COMMON_H_
#include "stdint.h"



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


#define ENDPOINT_DIR_POS 7
#define ENDPOINT_DIR_OUT 0 
#define ENDPOINT_DIR_IN 1
#define ENDPOINT_ATTR_TRANSFERTYPE_POS 0
#define ENDPOINT_ATTR_TRANSFERTYPE_CONTROL 0
#define ENDPOINT_ATTR_TRANSFERTYPE_ISOCHRONOUS 1
#define ENDPOINT_ATTR_TRANSFERTYPE_BULK 2
#define ENDPOINT_ATTR_TRANSFERTYPE_INTERRUPT 3
#define ENDPOINT_ATTR_SYNC_TYPE_POS 2
#define ENDPOINT_ATTR_SYNC_TYPE_NO_SYNC 0
#define ENDPOINT_ATTR_SYNC_TYPE_ASYNC 1
#define ENDPOINT_ATTR_SYNC_TYPE_ADAPTIVE 2
#define ENDPOINT_ATTR_SYNC_TYPE_SYNC 3
#define ENDPOINT_ATTR_USAGE_POS 4
#define ENDPOINT_ATTR_USAGE_DATA 0
#define ENDPOINT_ATTR_USAGE_FEEDBACK 1
#define ENDPOINT_ATTR_USAGE_IMPL_FEEDBACK 2

#define CDC_FUNC_DESC_HEADER 0
#define CDC_FUNC_DESC_CALL_MANAGEMENT 1
#define CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT 2
#define CDC_FUNC_DESC_UNION 6

#define DEVICE_DESCRIPTOR_LENGTH 18
#define CONFIGURATION_DESCRIPTOR_LENGTH 9
#define INTERFACE_DESCRIPTOR_LENGTH 9
#define ENDPOINT_DESCRIPTOR_LENGTH 7

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

typedef struct 
{
    uint8_t* buffer;
    volatile uint32_t* ep_buf_ctrl;
    uint8_t pid;
    void(*epHandler)(void);
    
} UsbEndpointConfigurationType;

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

typedef UsbStringDescriptorType* UsbStringDescriptor;

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wLangId;
} UsbString0DescriptorType;

typedef UsbString0DescriptorType* UsbString0Descriptor;

typedef struct 
{
    uint8_t transferInProgress;
    uint8_t bMaxPacketSize;
    uint32_t address;
    uint32_t len; 
    uint32_t idx;
} UsbMultipacketTransfer;

/**
 * @brief send the next packet of a transfer spanning multiple packets over the given endpoint. 
 * 
 * @param ep the endpoint to send the data over
 * @param th the object containing the data for the multipacket transfer
 */
void send_next_packet(UsbEndpointConfigurationType* ep,UsbMultipacketTransfer* th);

/**
 * @brief initializes the usb device. Specifically configures endpoint0 to be ready to handle the default setup requests
 * 
 */
void initUSB();

/**
 * @brief to be implemented in the specific device driverm, should setup the endpoints according to the 
 * specific device type , e.g. and HID device contains typically a single in endpoints besides ep0In and ep0Out
 * 
 */
void initUsbDeviceDriver(UsbEndpointConfigurationType **,UsbEndpointConfigurationType **,void(*)(void));

/**
 * @brief handles setup packets from device to host which are driver-specific
 * 
 * @param pck 
 * @param ep 
 * @return uint8_t 1 if handled 0 otherwise
 */
uint8_t handleSetupRequestIn(UsbSetupPacket pck,UsbEndpointConfigurationType * ep);

/**
 * @brief handles setup packets from the host to the device
 * 
 * @param pck 
 * @param ep 
 * @return uint8_t 
 */
uint8_t handleSetupRequestOut(UsbSetupPacket pck,UsbEndpointConfigurationType * ep);

#endif