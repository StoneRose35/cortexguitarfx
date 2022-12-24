#ifndef _USB_H_
#define _USB_H_
void initUSB();
void cdc_write(CommBuffer bfr, uint8_t blocking);

void cdc_task(CommBuffer bfr);

#endif