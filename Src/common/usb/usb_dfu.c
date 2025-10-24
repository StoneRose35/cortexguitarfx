#include "stdint.h"
#include "stdlib.h"
#include "stm32h750/stm32h750xx.h"
#include "usb/usb_common.h"
#include "drivers/usb.h"
#include "qspi.h"
#include "usb/usb_dfu.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "system.h"
#include "uart.h"
#include "flash.h"
#include "stringFunctions.h"
#include "globalConfig.h"

extern volatile uint32_t task;
extern volatile uint8_t programChangeState;
volatile uint8_t usbDfuState=USB_DFU_APP_IDLE;

volatile uint16_t firmwareSize;
volatile uint16_t currentFirmwareBlockNr;


uint8_t dfuFileValid = 0xFF;
uint32_t flashTargetAddress = 0xFFFFFFFF;
uint32_t flashSize = 0;
uint32_t qspiTargetAddress = 0xFFFFFFFF;
uint32_t qspiSize = 0;
uint8_t flashWritten = 0;
uint8_t qspiWritten = 0;
uint8_t flashWordCntr = 0;
uint16_t flashWordsWritten=0;
uint32_t bytesWritten = 0;
uint8_t qspiBuffer[256];
uint16_t qspiBufferBytesFetched = 0;
uint32_t qspiBytesWritten=0;
uint16_t qspiPageCnt=0;
const uint8_t usbDeviceDescriptorDfu[] = {
    0x12, // bLength
    SETUP_PACKET_DESCR_TYPE_DEVICE, // device descriptor type
    0x10, //bcdUSB, lsb
    0x01, //bcdUSB, msb
    0x00, // device class
    0x00, // device subclass
    0x00, // device protocol
    0x40, // max endpoint0 size
    __LOBYTE(USB_VENDOR_ID), // vendor id, lsb
    __HIBYTE(USB_VENDOR_ID), // vendor id, msb
    __LOBYTE(USB_PRODUCT_ID), // product id, lsb
    __HIBYTE(USB_PRODUCT_ID), // product id, msb
    0x04,
    0x00, //bcdDevice
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
    __LOBYTE(27), // configation descriptor size, lsb
    __HIBYTE(27), // configurator descriptor size, msb
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
    (uint8_t)((0 << 3) | (0 << 2) | (1 << 1 ) | ( 1 << 0)), //bmAttributes:will not automatically detach and reattach, not manifestation tolerant, can download and upload 
    0xF0, //wDetachTimeOut, lsb
    0x00, //wDetachTimeOut, msb
    0x40, //wTransferSize, lsb
    0x00, //wTransferSize, msb
    0x00, // bcdDFUVersion
    0x01  // bcdDFUVersion
};

const uint16_t usbConfigurationDescriptorDfuSize = sizeof(usbConfigurationDescriptorDfu);

volatile UsbDfuStatusType usbDfuStatus;

UsbStringDescriptorType stringDescriptorsDfu[] = {
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((12-1)*2)+2, .bString = "StoneRose35"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((9-1)*2)+2, .bString = "PiPicoFX"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((11-1)*2)+2, .bString = "3457456234"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((31-1)*2)+2, .bString = "QSPI DFU Programming Interface"},
    {.bDescriptorType = SETUP_PACKET_DESCR_TYPE_STRING, .bLength = ((18-1)*2)+2, .bString = "@Flash 0x08000000"},
};

__RAMFUNC
uint8_t setUsbConfigurationDfu(uint16_t confNr)
{
    setupEndpoint(0,EP_DIRECTION_OUT,0x40);
    setEndpointOutHandler(&endPoint0DfuHandler,0);
    return 0;
}


__RAMFUNC
void usbDfuResetHandler(void)
{
    if (usbDfuState == USB_DFU_MANIFEST_WAIT_RESET)
    {
        NVIC_SystemReset();
    }
    currentFirmwareBlockNr = 0;
}

__RAMFUNC
uint8_t usbDfuHandleClassSetupRequest(const UsbSetupPacketType* packet)
{
    switch (packet->bRequest)
    {
    case SETUP_REQUEST_DFU_DETACH:
        prepareUSBTransfer(0,0,0); 
        setUsbDeviceDescriptor(usbDeviceDescriptorDfu,usbDeviceDescriptorDfuSize);
        setUsbConfigurationDescriptor(usbConfigurationDescriptorDfu,usbConfigurationDescriptorDfuSize);
        setUsbStringDescriptors(stringDescriptorsDfu);
        setConfigurationHandler(&setUsbConfigurationDfu);
        setResetHandler(&usbDfuResetHandler);
        task |= (1 << TASK_PREPARE_FOR_DFU);
        usbDfuState = USB_DFU_APP_DETACH;
        setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,0xFF,100); 
        currentFirmwareBlockNr = 0;
        break;
    case SETUP_REQUEST_DFU_UPLOAD:
        firmwareSize = packet->wLength;
        usbDfuState = USB_DFU_UPLOAD_IDLE;
        setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,0xFF,255);
        prepareUSBTransfer(0,(uint8_t*)(0x08000000),firmwareSize); 
        break;
    case SETUP_REQUEST_DFU_DNLOAD: // from host to device
        firmwareSize = packet->wLength;
        if (packet->wValue != currentFirmwareBlockNr)
        {
            usbDfuState = USB_DFU_ERROR;
            setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_ERR_FILE,0xFF,255);
        }
        else
        {
            if (packet->wLength > 0)
            {
                usbDfuState = USB_DFU_DNLOAD_SYNC;
                setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,USB_DFU_DNBUSY,10);
                currentFirmwareBlockNr = packet->wValue+1;
                prepareEP0Rception();
            }
            else if (packet->wLength==0 && usbDfuState == USB_DFU_DNLOAD_IDLE)
            {
                prepareUSBTransfer(0,0,0);
                usbDfuState = USB_DFU_MANIFEST_SYNC;
                task |= (1 << TASK_MANIFEST_DFU);
                setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,USB_DFU_MANIFEST,10);
            }
        }
        
        break;
    case SETUP_REQUEST_DFU_GETSTATUS:
        setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,0xff,10);
        prepareUSBTransfer(0,(uint8_t*)(&usbDfuStatus),sizeof(usbDfuStatus));
        break; 
    case SETUP_REQUEST_DFU_GETSTATE:
        prepareUSBTransfer(0,(uint8_t*)(&usbDfuState),1);
        break;
    case SETUP_REQUEST_DFU_CLRSTATUS:
        usbDfuState = USB_DFU_IDLE;
        currentFirmwareBlockNr = 0;
        prepareUSBTransfer(0,0,0); 
        break;
    case SETUP_REQUEST_DFU_ABORT:
        prepareUSBTransfer(0,0,0); 
        break;
    default:
        break;
    }
    return 0;
}

__RAMFUNC
void prepareSystemForDFU()
{
        #ifdef DFU_SIM
        initUart(115200);
        #endif
            // disable interrupts
        // audio engine
        #ifdef EXTERNAL_CODEC
        #ifdef PCM3060_CODEC_EXTERNAL
        NVIC_ClearPendingIRQ(DMA1_Stream0_IRQn);
        NVIC_DisableIRQ(DMA1_Stream0_IRQn);
        #endif
        #else
        NVIC_ClearPendingIRQ(DMA1_Stream0_IRQn);
        NVIC_DisableIRQ(DMA1_Stream0_IRQn);
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

        setClassSpecificSetupHandler(&usbDfuHandleClassSetupRequest);

        usbDfuState = USB_DFU_IDLE;
        while(1)
        {
            if ((task & (1 << TASK_MANIFEST_DFU))!=0)
            {
                    // do whatever necessary before a dfu-driver initiated system reset
                    usbDfuEndManifestation(); // notify the usb dfu driver that the device is ready to be reborn as a different specie
                    task &= ~(1 << TASK_MANIFEST_DFU);
            }
        }
}

/**
 * status: the usb status to set (either ok or one of the various errors)
 * nextstate: the state the state machine has upon the next request of getStatus, 0xFF is no change expected
 * timeout: how long the host should wait before issueing the next getStatus request
 */
__RAMFUNC
void setUsbDfuStatus(volatile UsbDfuStatusType*statusStruct,uint8_t status,uint8_t nextState,uint32_t timeout)
{
    statusStruct->bState = usbDfuState;
    if (nextState != 0xFF)
    {
        usbDfuState = nextState;
    }
    statusStruct->bwPollTimeout[0] = (uint8_t)(timeout & 0xFF);
    statusStruct->bwPollTimeout[1] = (uint8_t)((timeout >> 8) & 0xFF);
    statusStruct->bwPollTimeout[2] = (uint8_t)((timeout >> 16 ) & 0xFF);
    statusStruct->iString=0;
    statusStruct->bStatus = status;
}

__RAMFUNC
void endPoint0DfuHandler(void*data,uint16_t dataSize)
{
    #ifdef DFU_SIM
    char chrbfr[32]; 

    #else
    uint8_t flashWordBfr[] = {  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                };
    #endif
    if (firmwareSize > 0)
    {
        firmwareSize -= dataSize;
    }
    if (firmwareSize==0 && dataSize > 0)
    {
        prepareUSBTransfer(0,0,0);
        if (dfuFileValid == 0xFF)
        {
            if (*((uint32_t*)data) == 0x4168b18f)
            {
                dfuFileValid = 1;
                #ifdef DFU_SIM
                sendStringBlocking("USB DFU got Magic Nr\r\n");
                #else
                eraseSector();
                #endif
            }
            else
            {
                dfuFileValid = 0;
                usbDfuState = USB_DFU_ERROR;
                setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_ERR_FIRMWARE,0xFF,10);
            }
        }
        if (dfuFileValid == 1 && flashTargetAddress == 0xFFFFFFFF && flashWritten == 0)
        {
            flashTargetAddress = *(((uint32_t*)data) + 1);
            flashSize = *(((uint32_t*)data) + 2);
            #ifdef DFU_SIM
            sendStringBlocking("Flash Target Address: ");
            UInt32ToHex(flashTargetAddress,chrbfr);
            sendStringBlocking(chrbfr);
            sendStringBlocking(", Flash Size: ");
            UInt32ToChar(flashSize,chrbfr);
            sendStringBlocking(chrbfr);
            sendStringBlocking("\r\n");
            #endif
            uint16_t c=12;
            while (c < dataSize && bytesWritten < flashSize)
            {
                #ifndef DFU_SIM
                flashWordBfr[flashWordCntr++]=*(((uint8_t*)data) + c);
                #else
                flashWordCntr++;
                #endif
                c++;
                bytesWritten++;
                if ((flashWordCntr == 32 || bytesWritten == flashSize) && flashWritten == 0)
                {
                    #ifdef DFU_SIM
                    sendStringBlocking("Writing Flash, Word ");
                    UInt16ToChar(flashWordsWritten,chrbfr);
                    sendStringBlocking(chrbfr);
                    sendStringBlocking("\r\n");
                    #else
                    writeFlashWord(flashWordBfr,flashTargetAddress-0x08000000 + (flashWordsWritten << 5));
                    #endif
                    flashWordsWritten++;
                    flashWordCntr=0;
                    if (bytesWritten == flashSize)
                    {
                        flashWritten = 1;
                    }
                }
            }
            if ((flashWordCntr == 32 || bytesWritten == flashSize) && flashWritten == 0)
            {
                #ifdef DFU_SIM
                sendStringBlocking("Writing Flash, Word ");
                UInt16ToChar(flashWordsWritten,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                writeFlashWord(flashWordBfr,flashTargetAddress-0x08000000 + (flashWordsWritten << 5));
                #endif
                flashWordsWritten++;
                flashWordCntr=0;
                if (bytesWritten == flashSize)
                {
                    flashWritten = 1;
                }
            }
            if (bytesWritten == flashSize)
            {
                qspiTargetAddress = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                qspiSize = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                flashWritten = 1;
                #ifdef DFU_SIM
                sendStringBlocking("Qspi Target Address: ");
                UInt32ToHex(qspiTargetAddress,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking(", Qspi Size: ");
                UInt32ToChar(qspiSize,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                setQspiStatus(2);
                endMemoryMappedMode();
                #endif


                #ifndef DFU_SIM
                uint32_t blockaddress=0;
                uint32_t nblocks = qspiSize / 0x10000;
                nblocks++;
                for (uint32_t c=0;c<nblocks;c++)
                {
                    QspiEraseBlock64(blockaddress);
                    blockaddress += 0x10000;
                }
                #endif
                while ( c < dataSize)
                {
                    // write qspi data
                    qspiBuffer[qspiBufferBytesFetched++] = *((uint8_t*)data + c);
                    qspiBytesWritten++;
                    c++;
                    if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
                    {
                        #ifdef DFU_SIM
                        sendStringBlocking("Writing Qspi Page ");
                        UInt16ToChar(qspiPageCnt,chrbfr);
                        sendStringBlocking(chrbfr);
                        sendStringBlocking("\r\n");
                        #else
                        QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                        #endif
                        qspiPageCnt++;
                        qspiBufferBytesFetched = 0;
                        if (qspiBytesWritten == qspiSize)
                        {
                            qspiWritten = 1;
                        }
                    }
                }
                if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
                {
                    #ifdef DFU_SIM
                    sendStringBlocking("Writing Qspi Page ");
                    UInt16ToChar(qspiPageCnt,chrbfr);
                    sendStringBlocking(chrbfr);
                    sendStringBlocking("\r\n");
                    #else
                    QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                    #endif
                    qspiPageCnt++;
                    qspiBufferBytesFetched = 0;
                    if (qspiBytesWritten == qspiSize)
                    {
                        qspiWritten = 1;
                    }
                }
            }
        }
        else if (dfuFileValid == 1 && flashWritten == 0)
        {
            uint16_t c=0;
            while (c<dataSize && bytesWritten < flashSize)
            {
                #ifndef DFU_SIM
                flashWordBfr[flashWordCntr++]=*(((uint8_t*)data) + c);
                #else
                flashWordCntr++;
                #endif
                c++;
                bytesWritten++;
                if ((flashWordCntr == 32 || bytesWritten == flashSize) && flashWritten == 0)
                {
                    #ifdef DFU_SIM
                    sendStringBlocking("Writing Flash, Word ");
                    UInt16ToChar(flashWordsWritten,chrbfr);
                    sendStringBlocking(chrbfr);
                    sendStringBlocking("\r\n");
                    #else
                    writeFlashWord(flashWordBfr,flashTargetAddress-0x08000000 + (flashWordsWritten << 5));
                    #endif
                    flashWordsWritten++;
                    flashWordCntr=0;
                    if (bytesWritten == flashSize)
                    {
                        flashWritten = 1;
                    }
                }
            }
            if ((flashWordCntr == 32 || bytesWritten == flashSize) && flashWritten == 0)
            {
                #ifdef DFU_SIM
                sendStringBlocking("Writing Flash, Word ");
                UInt16ToChar(flashWordsWritten,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                writeFlashWord(flashWordBfr,flashTargetAddress-0x08000000 + (flashWordsWritten << 5));
                #endif
                flashWordsWritten++;
                flashWordCntr=0;
                if (bytesWritten == flashSize)
                {
                    flashWritten = 1;
                }
            }
            if (bytesWritten == flashSize)
            {
                qspiTargetAddress = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                qspiSize = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                flashWritten = 1;
                #ifdef DFU_SIM
                sendStringBlocking("Qspi Target Address: ");
                UInt32ToHex(qspiTargetAddress,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking(", Qspi Size: ");
                UInt32ToChar(qspiSize,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                setQspiStatus(2);
                endMemoryMappedMode();
                #endif

                #ifndef DFU_SIM
                uint32_t blockaddress=0;
                uint32_t nblocks = qspiSize / 0x10000;
                nblocks++;
                for (uint32_t c=0;c<nblocks;c++)
                {
                    QspiEraseBlock64(blockaddress);
                    blockaddress += 0x10000;
                }
                #endif
                while ( c < dataSize)
                {
                    // write qspi data
                    qspiBuffer[qspiBufferBytesFetched++] = *((uint8_t*)data + c);
                    qspiBytesWritten++;
                    c++;
                    if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
                    {
                        #ifdef DFU_SIM
                        sendStringBlocking("Writing Qspi Page ");
                        UInt16ToChar(qspiPageCnt,chrbfr);
                        sendStringBlocking(chrbfr);
                        sendStringBlocking("\r\n");
                        #else
                        QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                        #endif
                        qspiPageCnt++;
                        qspiBufferBytesFetched = 0;
                        if (qspiBytesWritten == qspiSize)
                        {
                            qspiWritten = 1;
                        }
                    }
                }
                if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
                {
                    #ifdef DFU_SIM
                    sendStringBlocking("Writing Qspi Page ");
                    UInt16ToChar(qspiPageCnt,chrbfr);
                    sendStringBlocking(chrbfr);
                    sendStringBlocking("\r\n");
                    #else
                    QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                    #endif
                    qspiPageCnt++;
                    qspiBufferBytesFetched = 0;
                    if (qspiBytesWritten == qspiSize)
                    {
                        qspiWritten = 1;
                    }
                }
            }
        }
        else if (dfuFileValid == 1 && flashWritten == 1)
        {
            uint16_t c=0;
            if (qspiTargetAddress == 0xFFFFFFFF)
            {
                qspiTargetAddress = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                qspiSize = (((const struct T_UINT32_READ *)(const void *)(data + c))->v);
                c+=4;
                #ifdef DFU_SIM
                sendStringBlocking("Qspi Target Address: ");
                UInt32ToHex(qspiTargetAddress,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking(", Qspi Size: ");
                UInt32ToChar(qspiSize,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                setQspiStatus(2);
                endMemoryMappedMode();
                #endif

                #ifndef DFU_SIM
                uint32_t blockaddress=0;
                uint32_t nblocks = qspiSize / 0x10000;
                nblocks++;
                for (uint32_t c=0;c<nblocks;c++)
                {
                    QspiEraseBlock64(blockaddress);
                    blockaddress += 0x10000;
                }
                #endif
            }

            while ( c < dataSize)
            {
                // write qspi data
                qspiBuffer[qspiBufferBytesFetched++] = *((uint8_t*)data + c);
                qspiBytesWritten++;
                c++;
                if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
                {
                    #ifdef DFU_SIM
                    sendStringBlocking("Writing Qspi Page ");
                    UInt16ToChar(qspiPageCnt,chrbfr);
                    sendStringBlocking(chrbfr);
                    sendStringBlocking("\r\n");
                    #else
                    QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                    #endif
                    qspiPageCnt++;
                    qspiBufferBytesFetched = 0;
                    if (qspiBytesWritten == qspiSize)
                    {
                        qspiWritten = 1;
                    }
                }
            }
            if ((qspiBufferBytesFetched == 256 || qspiBytesWritten == qspiSize) && qspiWritten == 0)
            {
                #ifdef DFU_SIM
                sendStringBlocking("Writing Qspi Page ");
                UInt16ToChar(qspiPageCnt,chrbfr);
                sendStringBlocking(chrbfr);
                sendStringBlocking("\r\n");
                #else
                QspiProgramPage(qspiPageCnt << 8, qspiBuffer);
                #endif
                qspiPageCnt++;
                qspiBufferBytesFetched = 0;
                if (qspiBytesWritten == qspiSize)
                {
                    qspiWritten = 1;
                }
            }
        }

        if (usbDfuState==USB_DFU_DNBUSY)
        {
            usbDfuState = USB_DFU_DNLOAD_IDLE; // set ready for next download 
        }
    }
    else if (firmwareSize > 0)
    {
        prepareEP0Rception();
    }

    setUsbDfuStatus(&usbDfuStatus,USB_DFU_STATUS_OK,0xFF,10);
}

__RAMFUNC
void usbDfuEndManifestation()
{
    if (qspiBytesWritten == qspiSize && bytesWritten == flashSize)
    {
        usbDfuState = USB_DFU_MANIFEST_WAIT_RESET;
    }
    else
    {
        usbDfuState = USB_DFU_ERROR;
    }
}