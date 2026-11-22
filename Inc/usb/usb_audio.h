#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

#include <stdint.h>

#define USB_AUDIO_SAMPLE_RATE 48000
#define USB_AUDIO_VENDOR_OUT_EP 1
#define USB_AUDIO_VENDOR_IN_EP 1
#define USB_AUDIO_PLAYBACK_OUT_EP 3
#define USB_AUDIO_CAPTURE_IN_EP 4
#define USB_AUDIO_FEEDBACK_IN_EP 5
void USBAudioInit(void);
void USBAudioStartOfFrame(void);
void UsbAudioIncompleteInTransfer(void);
void UsbAudioEndOfPeriodicFrame(void);
void UsbAudioIsoOutIncomplete(void);
uint8_t USBAudioMixSample(int32_t processedSample, int32_t *left, int32_t *right);
void usbAudioReset(void);
#endif
