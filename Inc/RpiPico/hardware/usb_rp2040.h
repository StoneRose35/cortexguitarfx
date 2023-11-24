#ifndef USB_RP2040_H_
#define USB_RP2040_H_
#include "stdint.h"
#include "hardware/structs/usb.h"
#include "usb/usb_common.h"

#define usb_hw_set hw_set_alias(usb_hw)
#define usb_hw_clear hw_clear_alias(usb_hw)

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

#define EP_CTRL_ENABLE_POS 31
#define EP_CTRL_BUFFERTYPE_POS 30
#define EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS 29
#define EP_CTRL_INTR_AFTER_TWO_BUFFERS_POS 28
#define EP_CTRL_EP_TYPE 26 // 0: control, 1: ISO, 2: Bulk, 3: Interrupt
#define EP_CTRL_INTR_STALL 17
#define EP_CTRL_INTER_NAK 16



void usb_start_in_transfer(UsbEndpointConfigurationType * ep,const uint8_t * data,uint8_t len);
void usb_start_out_transfer(UsbEndpointConfigurationType * ep,uint8_t len);

#define CDC_DATA_MAX_PACKET_SIZE_IN 0x40
#define CDC_DATA_MAX_PACKET_SIZE_OUT 0x40

#endif