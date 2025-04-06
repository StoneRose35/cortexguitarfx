#ifndef _USB_H_
#define _USB_H_

#define USB2_OTG_FS_DEVICE ((USB_OTG_DeviceTypeDef*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
// initializes the hardware part for usb communication
void initUSB();



#endif