#ifndef _USB_H_
#define _USB_H_
#include "hardware/structs/usb.h"
typedef struct 
{
    uint8_t* bufferIn;
    uint8_t* bufferOut;
    usb_device_dpram_t* dpram;
    uint8_t pid;
    uint8_t nr;
} UsbEndpointConfigurationType;

void initUSB();
//void cdc_write(CommBuffer bfr, uint8_t blocking);
//void cdc_task(CommBuffer bfr);
void usb_start_in_transfer(UsbEndpointConfigurationType * ep,const uint8_t * data,uint8_t len);
void usb_start_out_transfer(UsbEndpointConfigurationType * ep,uint8_t len);
void initUsbCdcDevice();


#endif