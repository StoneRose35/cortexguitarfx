#ifndef USB_CDC_H_
#define USB_CDC_H_


#define SETUP_PACKET_REQ_CDC_GET_LINE_CODING 0x20
#define SETUP_PACKET_REQ_CDC_SET_LINE_CODING 0x21
#define SETUP_PACKET_REQ_CDC_SET_CONTROL_LINE_STATE 0x22

void sendOverUsb(void * commBfr,uint8_t blocking);
#endif