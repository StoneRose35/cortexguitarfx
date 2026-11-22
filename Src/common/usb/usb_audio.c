#include <stdint.h>
#include "drivers/usb.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_audio.h"
#include "usb/usb_common.h"
#include "drivers/gpio.h"
#include "usb/usb_vendor_specific_dfu_capable.h"
#include "sai.h"

#define USB_AUDIO_DFU_INTERFACE 0
#define USB_AUDIO_VENDOR_INTERFACE 1
#define USB_AUDIO_CONTROL_INTERFACE 2
#define USB_AUDIO_CAPTURE_INTERFACE 3
#define USB_AUDIO_PLAYBACK_INTERFACE 4


#define USB_AUDIO_MAX_PACKET_SIZE 294
#define USB_AUDIO_RING_FRAMES 256
#define USB_AUDIO_RING_MASK (USB_AUDIO_RING_FRAMES - 1)
#define USB_AUDIO_CAPTURE_TARGET 96
#define USB_AUDIO_PLAYBACK_TARGET 96

#define USB_AUDIO_SAMPLES_PER_USB_FRAME 48
#define USB_AUDIO_FEEDBACK_BASE (USB_AUDIO_SAMPLES_PER_USB_FRAME << 14)

#define USB_AUDIO_ENDPOINT_CLASS_GET_CUR 0x81
#define USB_AUDIO_ENDPOINT_CLASS_GET_MIN 0x82
#define USB_AUDIO_ENDPOINT_CLASS_GET_MAX 0x83
#define USB_AUDIO_ENDPOINT_CLASS_GET_RES 0x84
#define USB_AUDIO_ENDPOINT_CLASS_SET_CUR 0x01
#define USB_AUDIO_CONTROL_SAMPLING_FREQUENCY 0x0100

#define USB_AUDIO_FRAME_NUMBER_MASK 0x3FFF


#undef USB_AUDIO_HARDWARE_DEBUGGING // (ab)use the two uart pins (pb14 and pb15) for electrical debugging purpose when defined 

#ifdef USB_AUDIO_HARDWARE_DEBUGGING
#include "drivers/gpio.h"
#define USB_AUDIO_DEBUG_PIN_A (1*16 + 14)
#define USB_AUDIO_DEBUG_PIN_B (1*16 + 15) 
#endif

#if F_SAMPLING != USB_AUDIO_SAMPLE_RATE
#error "USB audio and codec sample rates must match"
#endif

static const uint8_t usbAudioConfigurationDescriptor[] = {
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_CONFIGURATION, 
    224, // configuration descriptor size, lsb
    0x00, // configuration descriptor size, msb
    0x05, // bNumInterfaces 
    0x01, // bConfigurationValue
    0x07, // configuration string id
    0x80, // bmAttributes, 0xC0 self powered, 0x80 bus powered 
    0xFA, // bus power

    //-------------------------------------
    // Interface Descriptor: USB DFU
    // -----------------------------------
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, 
    0x00, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x00, //bNumEndpoints
    0xFE, //bInterfaceClass
    0x01, //bInterfaceSubClass
    0x01, //bInterfaceProtocol
    0x06, //iInterface
    //------------------------------------
    //------------------------------------
    // Interface functional descriptor
    //------------------------------------
    0x09, //bLength
    0x21, //bDescriptorType
    (uint8_t)((0 << 3) | (0 << 2) | (1 << 1 ) | ( 1 << 0)), //bmAttributes:will not automatically detach and reattach, not manifestation tolerant, can download and upload 
    0xF0, //wDetachTimeOut, lsb
    0x00, //wDetachTimeOut, msb
    0x40, //wTransferSize, lsb
    0x00, //wTransferSize, msb
    0x00, // bcdDFUVersion
    0x01,  // bcdDFUVersion
    //------------------------------------
    //-------------------------------------
    // Interface Descriptor: Data Interface
    //-------------------------------------
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    0x01, // bInterfaceNumber: interface nr
    0x00, // bAlternateSetting: alternate setting
    0x02, // bNumEndpoints: two (out and in)
    0xFF, // Interface class: vendor-specific
    0x00, // interface subclass
    0x00, // interface protocol
    0x05, // interface description string id
    //-------------------------------------
    //-------------------------------------
    // Endpoint OUT descriptor
    //-------------------------------------
    0x07, // bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT, 
    (ENDPOINT_DIR_OUT << ENDPOINT_DIR_POS) | USB_AUDIO_VENDOR_OUT_EP, // endpoint address: 1 OUT,
    0x02, // bmAttributes: Bulk
    USB_VS_DATA_OUT_PACKETSIZE, // packet size, lsb
    0x00, // packet size, msb
    0x00, // bInterval
    //-------------------------------------
    //-------------------------------------
    // Endpoint IN descriptor
    //-------------------------------------
    0x07, // bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT, 
    (ENDPOINT_DIR_IN << ENDPOINT_DIR_POS) | USB_AUDIO_VENDOR_IN_EP, // endpoint address: 1 IN,
    0x02, // bmAttributes: Bulk
    USB_VS_DATA_IN_PACKETSIZE, // packet size, lsb
    0x00, // packet size, msb
    0x00, // bInterval
    //------------------------------------
    //-------------------------------------
    // Interface Descriptor: USB Audio Control interface
    // -----------------------------------   
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE, 
    USB_AUDIO_CONTROL_INTERFACE, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x00, //bNumEndpoints 
    0x01, //bInterfaceClass, AUDIO
    0x01, //bInterfaceSubClass, AUDIOCONTROL
    0x00, //bInterfaceProtocol, not used
    0x00, // iInterface
    // **************************************************
    // USB Audio Capture Device Interfaces and Terminals
    // **************************************************
    // audio class specific interface descriptor
    0x0A, //bLength
    0x24, //bDescriptorType, CS_INTERFACE
    0x01, //bDescriptorSubtype, HEADER
    0x00, //bcdADC,lsb, 1.0
    0x01, //bcdADC, msb, 1.0
    0x34, // wTotalLength
    0x00, // wTotalLength
    0x02, //bInCollection, 2 (Audio Playback and Capture)
    USB_AUDIO_PLAYBACK_INTERFACE, // baInterfaceNr
    USB_AUDIO_CAPTURE_INTERFACE, // baInterfaceNr
    // ********************************************
    // USB Audio Terminal Descriptors
    // ********************************************
    // -------------------------------------------------
    // Interface Descriptor / Input Terminal Descriptor, ID 1
    // -------------------------------------------------
    0x0C, //bLength
    0x24, //bDescriptorType, CS_INTERFACE
    0x02, //bDescriptorSubtype, INPUT_TERMINAL
    0x01, //bTerminalID, should be unique among terminals 
    0x01, // wTerminalType USB Streaming 
    0x01, // wTerminalType
    0x00, // bAssocTerminal, ID of the output terminal wo which this input terminal is associated
    0x02, // bNrChannels
    0x03, // wChannelConfig 
    0x00, // wChannelConfig
    0x00, // iChannelNames 
    0x00, // iTerminal

    // -------------------------------------------------
    // Interface Descriptor / Input Terminal Descriptor, ID 3
    // -------------------------------------------------
    0x0C, //bLength
    0x24, //bDescriptorType, CS_INTERFACE
    0x02, //bDescriptorSubtype, INPUT_TERMINAL
    0x03, //bTerminalID, should be unique among terminals 
    0x01, // wTerminalType: Microphone
    0x02, // wTerminalType
    0x00, // bAssocTerminal, id of the output terminal to which this input terminal is associated 
    0x02, // bNrChannels 
    0x03, // wChannelConfig
    0x00, // wChannelConfig
    0x00, // iChannelNames  
    0x00, // iTerminal
    // -------------------------------------------------
    // Interface Descriptor / Output Terminal Descriptor, ID 2
    // -------------------------------------------------
    0x09, // bLength
    0x24, // bDescriptorType, CS_INTERFACE
    0x03, // bDescriptorSubtype, OUTPUT_TERMINAL
    0x02, // bTerminalID, should be unique among terminals 
    0x01, // wTerminalType: Speaker  
    0x03, // wTerminalType
    0x00, // bAssocTerminal identifying the Input Terminal to which this Output Terminal is associated.
    0x01, // bSourceID id of the Unit or Terminal to which this Terminal is connected 
    0x00, // iTerminal
    // -------------------------------------------------
    // Interface Descriptor / Output Terminal Descriptor, ID 4
    // -------------------------------------------------
    0x09, //bLength
    0x24, //bDescriptorType, CS_INTERFACE
    0x03, // bDescriptorSubtype, OUTPUT_TERMINAL
    0x04, // bTerminalID 
    0x01, // wTerminalType USB Streaming
    0x01, // wTerminalType
    0x00, // bAssocTerminal, identifyying the inputer terminal to which this output terminal is associated
    0x03, // bSourceID id of the unit or terminal to which this terminal is connected
    0x00, // iTerminal
    
    
    // -------------------------------------------------
    // USB Audio Streaming Output Interface Descriptors, Device -> Host
    // -------------------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    USB_AUDIO_CAPTURE_INTERFACE,
    0x0, // bAlternateSetting: 0 (probably off)
    0x0, // bNumEndpoints
    0x1, // bInterfaceClass: Audio
    0x2, // bInterfaceSubclass: Streaming
    0x0, // bInterfaceProtocol
    0x0, // iInterface
    
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    USB_AUDIO_CAPTURE_INTERFACE,
    0x1, // bAlternateSetting: 1 (probably on)
    0x1, // bNumEndpoints
    0x1, // bInterfaceClass: Audio
    0x2, // bInterfaceSubclass: Streaming
    0x0, // bInterfaceProtocol
    0x0, // iInterface
    
    0x07, // bLength
    0x24, // bDescriptorType: CS_INTERFACE
    0x01, // bDescriptorSubtype: AS_GENERAL
    0x04, // bTerminalLink: 4 USB Streaming output
    0x01, // bDelay, 1 frames
    0x01, // wFormatTag: PCM
    0x00, // wFormatTag
    
    0x0B, // bLength
    0x24, // bDescriptorType: CS_INTERFACE
    0x02, // bDescriptorSubtype: FORMAT_TYPE
    0x01, // bFormatType: 1 (FORMAT_TYPE_I)
    0x02, // bNrChannels
    0x02, // bSubframeSize
    0x10, // bBitResolution
    0x01, // bSamFreqType, one sampling frequency
    0x80, // tSamFreq: 48000
    0xBB,
    0x00,

    // Endpoint Descriptor for Audio Streaming Input
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT,
    0x80 | USB_AUDIO_CAPTURE_IN_EP, 
    0x05, // bmAttributes, Isochronous, Asynchronous, Data
    0x26, // wMaxMacketSize, 294 Bytes
    0x01,
    0x01, // bInterval
    0x00, // bRefresh
    0x00, // bSynchAddress

    0x07, // bLength
    0x25, // bDescriptorType,  CS_ENDPOINT
    0x01, // bDescriptorSubtype, GENERAL
    0x00, // bmAttributes, no controls
    0x00, // bLockDelayUnits, unused
    0x00, // wLockDelay, unused
    0x00,
    
    

    
    // -------------------------------------------------
    // USB Audio Streaming Input Interface Descriptors, Host -> Device 
    // -------------------------------------------------
    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    USB_AUDIO_PLAYBACK_INTERFACE,
    0x0, // bAlternateSetting: 0 (probably off)
    0x0, // bNumEndpoints
    0x1, // bInterfaceClass: Audio
    0x2, // bInterfaceSubclass: Streaming
    0x0, // bInterfaceProtocol
    0x0, // iInterface

    0x09, // bLength
    SETUP_PACKET_DESCR_TYPE_INTERFACE,
    USB_AUDIO_PLAYBACK_INTERFACE,
    0x1, // bAlternateSetting: 1 (probably on)
    0x2, // bNumEndpoints
    0x1, // bInterfaceClass: Audio
    0x2, // bInterfaceSubclass: Streaming
    0x0, // bInterfaceProtocol
    0x0, // iInterface

    0x07, // bLength
    0x24, // bDescriptorType: CS_INTERFACE
    0x01, // bDescriptorSubtype: AS_GENERAL
    0x01, // bTerminalLink: 1 USB Streaming input
    0x01, // bDelay, 1 frames
    0x01, // wFormatTag: PCM
    0x00, // wFormatTag

    0x0B, // bLength
    0x24, // bDescriptorType: CS_INTERFACE
    0x02, // bDescriptorSubtype: FORMAT_TYPE
    0x01, // bFormatType: 1 (FORMAT_TYPE_I)
    0x02, // bNrChannels
    0x02, // bSubframeSize
    0x10, // bBitResolution
    0x01, // bSamFreqType, one sampling frequency
    0x80, // tSamFreq: 48000
    0xBB,
    0x00,

    // Endpoint Descriptor for Audio Streaming Input
    0x09, //bLength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT,
    USB_AUDIO_PLAYBACK_OUT_EP, //bEndpointAddress
    0x09, // bmAttributes, Isochronous, Adaptive, Data
    0x26, // wMaxMacketSize,294 Bytes
    0x01,
    0x01, // bInterval
    0x00, //bRefresh
    0x00, //USB_AUDIO_FEEDBACK_IN_EP, // bSynchAddress

    0x07, // bLength
    0x25, // bDescriptorType,  CS_ENDPOINT
    0x01, // bDescriptorSubtype, GENERAL
    0x00, // bmAttributes, no controls
    0x01, // bLockDelayUnits, unused
    0x01, // wLockDelay, unused
    0x00,

    // Synch endpoint
    0x09, //blength
    SETUP_PACKET_DESCR_TYPE_ENDPOINT,
    0x80 | USB_AUDIO_FEEDBACK_IN_EP,
    0x1, // bmAttributes, Isochronous
    0x03, // wMaxPacketSize
    0x00, // wMaxPacketSize
    0x01, // bInterval, 1
    0x01, // bRefresh, 1 -> 2ms
    0x00 // bSynchAddress
};
_Static_assert(sizeof(usbAudioConfigurationDescriptor) == 224U, "USB audio configuration length mismatch");

static volatile uint8_t playbackActive;
static volatile uint8_t captureActive;
static volatile uint16_t playbackWrite;
static volatile uint16_t playbackRead;
static volatile uint16_t captureWrite;
static volatile uint16_t captureRead;
static volatile uint8_t captureInFlight;
static volatile uint8_t feedbackInFlight;
static volatile uint8_t configured;
static volatile uint8_t dbgIndicator;
static volatile uint8_t frameStatus; //bit 0: packet receive complete, bit 1: end of packet interrupt received
#define USB_AUDIO_FRAME_STATUS_EOP_INTERRUPT_POS 1
#define USB_AUDIO_FRAME_STATUS_PACKED_RECEIVED_POS 0
static int32_t playbackRing[USB_AUDIO_RING_FRAMES][2];
static int32_t captureRing[USB_AUDIO_RING_FRAMES][2];
static uint8_t capturePacket[USB_AUDIO_MAX_PACKET_SIZE];
static uint8_t feedbackPacket[4];
static uint8_t sampleRateResponse[3];
static int32_t feedbackIntegral;
static uint32_t audioSampleRate = USB_AUDIO_SAMPLE_RATE;
extern void (*ep0OUTHandler)(void *, uint16_t);

static uint8_t usbAudioCompositeClassSetup(const UsbSetupPacketType *packet);
static void resetAudioRings(void);

void usbAudioReset(void)
{
    playbackActive = 0;
    captureActive = 0;
    captureInFlight = 0;
    feedbackInFlight = 0;
    configured = 0;
    audioSampleRate = USB_AUDIO_SAMPLE_RATE;
    resetAudioRings();
}

__RAMFUNC
static uint16_t ringLevel(volatile uint16_t writeIndex, volatile uint16_t readIndex)
{
    return (uint16_t)((writeIndex - readIndex) & USB_AUDIO_RING_MASK);
}

static void resetAudioRings(void)
{
    playbackWrite = 0;
    playbackRead = 0;
    captureWrite = 0;
    captureRead = 0;
    feedbackIntegral = 0;
}

static void primePlaybackRing(void)
{
    for (uint16_t frame = 0; frame < USB_AUDIO_PLAYBACK_TARGET; frame++)
    {
        playbackRing[frame][0] = 0;
        playbackRing[frame][1] = 0;
    }
    playbackRead = 0;
    playbackWrite = USB_AUDIO_PLAYBACK_TARGET;
}

static void primeCaptureRing(void)
{
    for (uint16_t frame = 0; frame < USB_AUDIO_CAPTURE_TARGET; frame++)
    {
        captureRing[frame][0] = 0;
        captureRing[frame][1] = 0;
    }
    captureRead = 0;
    captureWrite = USB_AUDIO_CAPTURE_TARGET;
}

__RAMFUNC
static void setIsochronousFrame(uint8_t endpoint, uint8_t direction)
{
    if (direction == EP_DIRECTION_IN)
    {
        USB_OTG_INEndpointTypeDef *inEndpoint =
            (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * endpoint);
        if ((((uint16_t)((USB2_OTG_FS_DEVICE->DSTS >> USB_OTG_DSTS_FNSOF_Pos) & USB_AUDIO_FRAME_NUMBER_MASK)) &1)==0)
        {
            inEndpoint->DIEPCTL |= USB_OTG_DIEPCTL_SODDFRM;
        }
        else
        {
            inEndpoint->DIEPCTL |= USB_OTG_DIEPCTL_SD0PID_SEVNFRM;
        }
    }
    else
    {
        USB_OTG_OUTEndpointTypeDef *outEndpoint =
            (USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20U * endpoint);
        if ((((uint16_t)((USB2_OTG_FS_DEVICE->DSTS >> USB_OTG_DSTS_FNSOF_Pos) & USB_AUDIO_FRAME_NUMBER_MASK)) &1)==0)
        {
            outEndpoint->DOEPCTL |= USB_OTG_DOEPCTL_SODDFRM;
        }
        else
        {
            outEndpoint->DOEPCTL |= USB_OTG_DOEPCTL_SD0PID_SEVNFRM;
        }
    }

    
}


__RAMFUNC
static void armPlaybackOut(void)
{
    uint32_t regval = 0;
    USB_OTG_OUTEndpointTypeDef *outEndpoint =
        (USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20U * USB_AUDIO_PLAYBACK_OUT_EP);
    regval = outEndpoint->DOEPCTL;
    if (!playbackActive || (regval & (1U << USB_OTG_DOEPCTL_EPENA_Pos)) != 0U)
    {
        return;
    }
    setIsochronousFrame(USB_AUDIO_PLAYBACK_OUT_EP, EP_DIRECTION_OUT);
    regval = outEndpoint->DOEPTSIZ;
    regval = (1U << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | USB_AUDIO_MAX_PACKET_SIZE;
    outEndpoint->DOEPTSIZ = regval;
    regval = outEndpoint->DOEPCTL;
    regval |= (1U << USB_OTG_DOEPCTL_CNAK_Pos) | (1U << USB_OTG_DOEPCTL_EPENA_Pos);
    outEndpoint->DOEPCTL = regval;
}

__RAMFUNC
static void sendCapturePacket(void)
{
    if (captureInFlight != 0U)
    {
        return;
    }

    uint16_t available = ringLevel(captureWrite, captureRead);
    uint16_t frames = USB_AUDIO_SAMPLES_PER_USB_FRAME;
    if (available > USB_AUDIO_CAPTURE_TARGET + 2U)
    {
        frames++;
    }
    else if (available + 2U < USB_AUDIO_CAPTURE_TARGET && frames > 1U)
    {
        frames--;
    }

    uint16_t byteCount = (uint16_t)(frames * 4U);
    for (uint16_t frame = 0; frame < frames; frame++)
    {
        for (uint8_t channel = 0; channel < 2; channel++)
        {
            uint32_t sample = 0;
            if (frame < available)
            {
                sample = (uint32_t)captureRing[captureRead][channel];
            }
            uint16_t offset = (uint16_t)(frame * 4U + channel * 2U);
            capturePacket[offset] = (uint8_t)sample;
            capturePacket[offset + 1U] = (uint8_t)(sample >> 8);
        }
        if (frame < available)
        {
            captureRead = (uint16_t)((captureRead + 1U) & USB_AUDIO_RING_MASK);
        }
    }

    setIsochronousFrame(USB_AUDIO_CAPTURE_IN_EP, EP_DIRECTION_IN);
    captureInFlight = 1;
    prepareUSBTransfer(USB_AUDIO_CAPTURE_IN_EP, capturePacket, byteCount,3);
}

__RAMFUNC
static void sendFeedbackPacket(void)
{
    if (feedbackInFlight != 0U)
    {
        return;
    }

    int32_t levelError = (int32_t)ringLevel(playbackWrite, playbackRead) - USB_AUDIO_PLAYBACK_TARGET;
    feedbackIntegral -= levelError;
    if (feedbackIntegral > 512)
    {
        feedbackIntegral = 512;
    }
    else if (feedbackIntegral < -512)
    {
        feedbackIntegral = -512;
    }

    int32_t adjustment = -(levelError * 4) + feedbackIntegral;
    if (adjustment > 4096)
    {
        adjustment = 4096;
    }
    else if (adjustment < -4096)
    {
        adjustment = -4096;
    }
    uint32_t feedback = (uint32_t)((int32_t)USB_AUDIO_FEEDBACK_BASE + adjustment);
    feedbackPacket[0] = (uint8_t)feedback;
    feedbackPacket[1] = (uint8_t)(feedback >> 8);
    feedbackPacket[2] = (uint8_t)(feedback >> 16);
    feedbackPacket[3] = 0;

    setIsochronousFrame(USB_AUDIO_FEEDBACK_IN_EP, EP_DIRECTION_IN);
    feedbackInFlight = 1;
    prepareUSBTransfer(USB_AUDIO_FEEDBACK_IN_EP, feedbackPacket, 3,2);
}

__RAMFUNC
static uint8_t usbAudioCaptureTransferDone(uint8_t epNr)
{
    (void)epNr;
    captureInFlight = 0;
    return 0;
}

__RAMFUNC
static uint8_t usbAudioFeedbackTransferDone(uint8_t epNr)
{
    (void)epNr;
    feedbackInFlight = 0;
    return 0;
}

__RAMFUNC
static void usbAudioPlaybackDataReceived(void *data, uint16_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint16_t frames = (uint16_t)(length / 4U);
    for (uint16_t frame = 0; frame < frames; frame++)
    {
        uint16_t nextWrite = (uint16_t)((playbackWrite + 1U) & USB_AUDIO_RING_MASK);
        if (nextWrite == playbackRead)
        {
            dbgIndicator = 2;
            break;
        }
        for (uint8_t channel = 0; channel < 2; channel++)
        {
            uint16_t offset = (uint16_t)(frame * 4U + channel * 2U);
            uint32_t sample = (uint32_t)bytes[offset]
                            | ((uint32_t)bytes[offset + 1U] << 8);
            if ((sample & 0x00008000U) != 0U)
            {
                sample |= 0xFFFF0000U;
            }
            sample <<= 8;
            playbackRing[playbackWrite][channel] = (int32_t)sample;
        }
        playbackWrite = nextWrite;
    }

    armPlaybackOut();

    #ifdef USB_AUDIO_HARDWARE_DEBUGGING
    setPin(USB_AUDIO_DEBUG_PIN_B,1);
    #endif
}

static void usbAudioControlOut(void *data, uint16_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    if (length == 3U)
    {
        uint32_t requestedRate = (uint32_t)bytes[0]
                               | ((uint32_t)bytes[1] << 8)
                               | ((uint32_t)bytes[2] << 16);
        if (requestedRate == USB_AUDIO_SAMPLE_RATE)
        {
            audioSampleRate = requestedRate;
            prepareUSBTransfer(0, 0, 0,0);
        }
        else
        {
            stallInEndpoint(0);
        }
    }
    else
    {
        stallInEndpoint(0);
    }
    setEndpointOutHandler(ep0OUTHandler, 0);
    prepareEP0Rception();
}

static void usbAudioConfigureEndpoint(uint8_t endpoint, uint8_t direction, uint16_t maxPacketSize)
{
    setupEndpoint(endpoint, direction, maxPacketSize);
    if (direction == EP_DIRECTION_OUT)
    {
        USB_OTG_OUTEndpointTypeDef *outEndpoint =
            (USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20U * endpoint);
        outEndpoint->DOEPCTL = (1U << USB_OTG_DOEPCTL_USBAEP_Pos)
                             | (EPTYP_ISOCHRONOUS << USB_OTG_DOEPCTL_EPTYP_Pos)
                             | ((uint32_t)maxPacketSize << USB_OTG_DOEPCTL_MPSIZ_Pos);
        USB2_OTG_FS_DEVICE->DAINTMSK |= (1U << (USB_OTG_DAINTMSK_OEPM_Pos + endpoint));
        setEndpointOutHandler(&usbAudioPlaybackDataReceived, endpoint);
    }
    else
    {
        USB_OTG_INEndpointTypeDef *inEndpoint =
            (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * endpoint);
        inEndpoint->DIEPCTL = (1U << USB_OTG_DIEPCTL_USBAEP_Pos)
                            | (EPTYP_ISOCHRONOUS << USB_OTG_DIEPCTL_EPTYP_Pos)
                            | ((uint32_t)endpoint << USB_OTG_DIEPCTL_TXFNUM_Pos)
                            | ((uint32_t)maxPacketSize << USB_OTG_DIEPCTL_MPSIZ_Pos);
        USB2_OTG_FS_DEVICE->DAINTMSK |= (1U << (USB_OTG_DAINTMSK_IEPM_Pos + endpoint));
    }
}

static void configureAudioInFifos(void)
{
    uint32_t offset = USB2_OTG_FS->GRXFSIZ
                    + ((USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ >> 16) & 0xFFFFU)
                    + ((USB2_OTG_FS->DIEPTXF[0] >> 16) & 0xFFFFU)
                    + ((USB2_OTG_FS->DIEPTXF[1] >> 16) & 0xFFFFU);
    USB2_OTG_FS->DIEPTXF[USB_AUDIO_CAPTURE_IN_EP - 1U] = offset | (80U << 16);
    offset += 80U;
    USB2_OTG_FS->DIEPTXF[USB_AUDIO_FEEDBACK_IN_EP - 1U] = offset | (16U << 16);
}

static uint8_t usbAudioSetConfiguration(uint16_t configuration)
{
    if (configuration == 0U)
    {
        playbackActive = 0;
        captureActive = 0;
        configured = 0;
        captureInFlight = 0;
        feedbackInFlight = 0;
        resetAudioRings();
        ((USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE
            + 0x20U * USB_AUDIO_PLAYBACK_OUT_EP))->DOEPCTL = 0;
        ((USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE
            + 0x20U * USB_AUDIO_CAPTURE_IN_EP))->DIEPCTL = 0;
        ((USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE
            + 0x20U * USB_AUDIO_FEEDBACK_IN_EP))->DIEPCTL = 0;
        USB2_OTG_FS_DEVICE->DAINTMSK &=
            ~((1U << (USB_OTG_DAINTMSK_OEPM_Pos + USB_AUDIO_PLAYBACK_OUT_EP))
              | (1U << (USB_OTG_DAINTMSK_IEPM_Pos + USB_AUDIO_CAPTURE_IN_EP))
              | (1U << (USB_OTG_DAINTMSK_IEPM_Pos + USB_AUDIO_FEEDBACK_IN_EP)));
        return 0;
    }
    if (configuration != 1U)
    {
        return 1;
    }
    disableAudioEngine();
    usbAudioConfigureEndpoint(USB_AUDIO_PLAYBACK_OUT_EP, EP_DIRECTION_OUT, USB_AUDIO_MAX_PACKET_SIZE);
    usbAudioConfigureEndpoint(USB_AUDIO_CAPTURE_IN_EP, EP_DIRECTION_IN, USB_AUDIO_MAX_PACKET_SIZE);
    usbAudioConfigureEndpoint(USB_AUDIO_FEEDBACK_IN_EP, EP_DIRECTION_IN, 3);
    configureAudioInFifos();

    setTransferDoneHandler(&usbAudioCaptureTransferDone, USB_AUDIO_CAPTURE_IN_EP);
    setTransferDoneHandler(&usbAudioFeedbackTransferDone, USB_AUDIO_FEEDBACK_IN_EP);

    playbackActive = 0;
    captureActive = 0;
    captureInFlight = 0;
    feedbackInFlight = 0;
    resetAudioRings();
    configured = 1;
    USB2_OTG_FS->GINTMSK |= (1U << USB_OTG_GINTMSK_SOFM_Pos);
    enableAudioEngine();
    return 0;
}

static uint8_t usbAudioCompositeClassSetup(const UsbSetupPacketType *packet)
{
    uint8_t endpoint = (uint8_t)(packet->wIndex & 0xFFU);

    if ((endpoint == USB_AUDIO_PLAYBACK_OUT_EP
         || endpoint == (0x80U | USB_AUDIO_CAPTURE_IN_EP))
        && packet->wValue == USB_AUDIO_CONTROL_SAMPLING_FREQUENCY)
    {
        if (packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_SET_CUR
            && (packet->bmRequestType & SETUP_PACKET_REQTYPE_DIR_DEVICE_TO_HOST) == 0U
            && packet->wLength == 3U)
        {
            setEndpointOutHandler(&usbAudioControlOut, 0);
            prepareUSBReception(0, 3);
            return 0;
        }

        if (packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_GET_CUR
            || packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_GET_MIN
            || packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_GET_MAX
            || packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_GET_RES)
        {
            uint32_t rate = (packet->bRequest == USB_AUDIO_ENDPOINT_CLASS_GET_RES) ? 0U : audioSampleRate;
            sampleRateResponse[0] = (uint8_t)rate;
            sampleRateResponse[1] = (uint8_t)(rate >> 8);
            sampleRateResponse[2] = (uint8_t)(rate >> 16);
            prepareUSBTransfer(0, sampleRateResponse, (packet->wLength < 3U) ? packet->wLength : 3U,0);
            return 0;
        }
    }

    return 1;
}

static uint8_t usbAudioCompositeSetInterfaceCapture(uint16_t alternateSetting)
{
    if ( alternateSetting <= 1U)
    {
        captureActive = (uint8_t)(alternateSetting == 1U);
        captureInFlight = 0;
        if (captureActive)
        {
            primeCaptureRing();
        }
        else
        {
            captureRead = captureWrite;
            USB_OTG_INEndpointTypeDef *inEndpoint =
                (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * USB_AUDIO_CAPTURE_IN_EP);
            inEndpoint->DIEPCTL &= ~(1U << USB_OTG_DIEPCTL_EPENA_Pos);
        }
        prepareUSBTransfer(0, 0, 0,0);
        return 0;
    }
    return 1;
}

static uint8_t usbAudioCompositeSetInterfacePlayback(uint16_t alternateSetting)
{
    if (alternateSetting <= 1U)
    {
        playbackActive = (uint8_t)(alternateSetting == 1U);
        if (playbackActive)
        {
            primePlaybackRing();
            armPlaybackOut();
        }
        else
        {
            playbackRead = playbackWrite;
            USB_OTG_OUTEndpointTypeDef *outEndpoint =
                (USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20U * USB_AUDIO_PLAYBACK_OUT_EP);
            outEndpoint->DOEPCTL &= ~(1U << USB_OTG_DOEPCTL_EPENA_Pos);
            USB_OTG_INEndpointTypeDef *feedbackEndpoint =
                (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * USB_AUDIO_FEEDBACK_IN_EP);
            feedbackEndpoint->DIEPCTL &= ~(1U << USB_OTG_DIEPCTL_EPENA_Pos);
            feedbackInFlight = 0;
        }
        prepareUSBTransfer(0, 0, 0,0);
        return 0;
    }
    return 1;
}

void USBAudioInit(void)
{
    setUsbConfigurationDescriptor(usbAudioConfigurationDescriptor, sizeof(usbAudioConfigurationDescriptor));
    addConfigurationHandler(&usbAudioSetConfiguration);
    setClassSpecificSetupHandler(&usbAudioCompositeClassSetup,USB_AUDIO_CONTROL_INTERFACE);
    setSetInterfaceHandler(&usbAudioCompositeSetInterfacePlayback,USB_AUDIO_PLAYBACK_INTERFACE);
    setSetInterfaceHandler(&usbAudioCompositeSetInterfaceCapture,USB_AUDIO_CAPTURE_INTERFACE);
    setResetHandler(&usbAudioReset);
    #ifdef USB_AUDIO_HARDWARE_DEBUGGING
    setAsOutput(USB_AUDIO_DEBUG_PIN_A);
    setAsOutput(USB_AUDIO_DEBUG_PIN_B);    
    #endif

    playbackActive = 0;
    captureActive = 0;
    configured = 0;
}

__ITCM_CODE_FLASH
void USBAudioStartOfFrame(void)
{
    #ifdef USB_AUDIO_HARDWARE_DEBUGGING
    setPin(USB_AUDIO_DEBUG_PIN_A,1);
    #endif
    if (configured == 0U)
    {
        return;
    }
    /*
    if (playbackActive)
    {
        USB_OTG_INEndpointTypeDef *feedbackEndpoint =
            (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * USB_AUDIO_FEEDBACK_IN_EP);
        if ((feedbackEndpoint->DIEPCTL & (1U << USB_OTG_DIEPCTL_EPENA_Pos)) == 0U)
        {
            feedbackInFlight = 0;
        }
        armPlaybackOut();
        //sendFeedbackPacket();
    }
    if (captureActive)
    {
        USB_OTG_INEndpointTypeDef *captureEndpoint =
            (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * USB_AUDIO_CAPTURE_IN_EP);
        if ((captureEndpoint->DIEPCTL & (1U << USB_OTG_DIEPCTL_EPENA_Pos)) == 0U)
        {
            captureInFlight = 0;
        }
        sendCapturePacket();
    }
    */
}

__RAMFUNC
void UsbAudioEndOfPeriodicFrame()
{
    #ifdef USB_AUDIO_HARDWARE_DEBUGGING
    setPin(USB_AUDIO_DEBUG_PIN_A,0);
    setPin(USB_AUDIO_DEBUG_PIN_B,0);
    #endif
}


__RAMFUNC
void UsbAudioIncompleteInTransfer(void)
{
    USB_OTG_INEndpointTypeDef *captureEndpoint =
            (USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20U * USB_AUDIO_CAPTURE_IN_EP);
    captureEndpoint->DIEPCTL |= (1 << USB_OTG_DIEPCTL_EPDIS_Pos) | (1 << USB_OTG_DIEPCTL_SNAK_Pos);
}

__RAMFUNC
void UsbAudioIsoOutIncomplete(void)
{
    USB_OTG_OUTEndpointTypeDef * playbackEndpoint = 
        (USB_OTG_OUTEndpointTypeDef * )(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20U * USB_AUDIO_PLAYBACK_OUT_EP);
    playbackEndpoint->DOEPCTL |= (1 << USB_OTG_DOEPCTL_EPDIS_Pos);
    resetOutEndpointBuffer(USB_AUDIO_PLAYBACK_OUT_EP);
}

__ITCM_CODE
uint8_t USBAudioMixSample(int32_t processedSample, int32_t *left, int32_t *right)
{
    if (captureActive)
    {
        uint16_t nextWrite = (uint16_t)((captureWrite + 1U) & USB_AUDIO_RING_MASK);
        if (nextWrite != captureRead)
        {
            captureRing[captureWrite][0] = processedSample;
            captureRing[captureWrite][1] = processedSample;
            captureWrite = nextWrite;
        }
    }

    int32_t playbackLeft = 0;
    int32_t playbackRight = 0;
    if (playbackActive && playbackRead != playbackWrite)
    {
        playbackLeft = playbackRing[playbackRead][0];
        playbackRight = playbackRing[playbackRead][1];
        playbackRead = (uint16_t)((playbackRead + 1U) & USB_AUDIO_RING_MASK);
    }

    int64_t mixedLeft = (int64_t)processedSample + playbackLeft;
    int64_t mixedRight = (int64_t)processedSample + playbackRight;
    uint8_t clipped = 0;
    if (mixedLeft > 8388607)
    {
        mixedLeft = 8388607;
        clipped = 1;
    }
    else if (mixedLeft < -8388608)
    {
        mixedLeft = -8388608;
        clipped = 1;
    }
    if (mixedRight > 8388607)
    {
        mixedRight = 8388607;
        clipped = 1;
    }
    else if (mixedRight < -8388608)
    {
        mixedRight = -8388608;
        clipped = 1;
    }
    *left = (int32_t)mixedLeft;
    *right = (int32_t)mixedRight;
    return clipped;
}
