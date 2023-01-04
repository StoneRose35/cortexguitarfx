#include "stdint.h"
#include "hardware/usb_rp2040.h"
#include "usb/usb_cdc.h"
#include "memFunctions.h"
#include "usb/usb_common.h"
#include "usb/usb_config.h"

#define MSC_REQ_RESET 0xFF
#define MSC_REQ_GET_MX_LUN 0xFE

#define MSC_CSW_STATUS_GOOD 0x0
#define MSC_CSW_STATUS_FAILED 0x1
#define MSC_CSW_STATUS_PHASE_ERROR 0x2

#define UFI_CMD_READ10 0x28
#define UFI_CMD_WRITE10 0x2A
#define UFI_CMD_READ_CAPACITY 0x25
#define UFI_CMD_READ_FORMAT_CAPACITIES 0x23
#define UFI_CMD_INQUIRY 0x12
#define UFI_CMD_MODE_SELECT 0x55
#define UFI_CMD_MODE_SENSE 0x5A
#define UFI_CMD_REQUEST_SENSE 0x3
#define SCSI_CMD_MODE_SENSE 0x1A

#include "stdint.h"

#define LOGICAL_BLOCK_SIZE 512
#define PACKET_SIZE_EP2 64
#define TOTAL_SIZE_IN_BLOCKS 16
#define PACKET_PER_BLOCK LOGICAL_BLOCK_SIZE/PACKET_SIZE_EP2


#define SENSE_NO_SENSE 0x0
#define SENSE_RECOVERED_ERROR 0x1
#define SENSE_NOT_READY 0x2
#define SENSE_MEDIUM_ERROR 0x3
#define SENSE_HARDWARE_ERROR 0x4
#define SENSE_ILLEGAL_REQUEST 0x5
#define SENSE_UNIT_ATTENTION 0x6
#define SENSE_DATA_PROTEXT 0x7
#define SENSE_BLANK_CHECK 0x8
#define SENSE_ABORTED_COMMAND 0xb
#define SENSE_VOLUME_OVERFLOW 0xd
#define SENSE_MISCOMPARE 0xe 

static void MSCDataOutHandler();
static void MSCDataInHandler();

typedef struct __attribute__((__packed__))
{
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLun; // upper 4 bits must be 0
	uint8_t bCBWCBLength; // upper 3 bits must be 0
	uint8_t CBWCB[16];
} CBWType;

typedef struct __attribute__((__packed__))
{
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
} CSWType;


typedef struct
{
	uint8_t bOpCode;
	uint8_t bLUN; // only upper 3 bits
	uint8_t bPageCode; // usually 0
	uint8_t res0;
	uint8_t bAllocationLength;
	uint8_t res1[7];
} UFIInquiryCmdType;

typedef struct __attribute__((__packed__))
{
	uint8_t bPeripheralDeviceType; // always 0x1F
	uint8_t bRemovableMedia; // 0x80 if removable, 0x00 otherwise
	uint8_t bIsoEcmaAnsiVersion; // always 0
	uint8_t bResponseDataFormat; //always 0x1
	uint8_t bAdditionalLength; // always 31
	uint8_t res[3];
	char cVendor[8]; // vendor string
	char cProductID[16]; // product id
	char productRevision[4]; // product revision aka firmware version
} UFIInquiryDataType;

typedef UFIInquiryDataType* UFIInquiryData;

#ifdef USB_MSC_DRIVER
UsbEndpointConfigurationType ep1In;
UsbEndpointConfigurationType ep1Out;
uint8_t packetBufferOut[LOGICAL_BLOCK_SIZE];
uint8_t packetBufferIn[LOGICAL_BLOCK_SIZE];
static volatile uint32_t packetBufferOutIdx;
static volatile uint8_t packetsTransferred;
static volatile uint32_t currentTag;
UsbMultipacketTransfer transferMSC;
CSWType  currentCSW;
uint32_t currentSense;

void generateCSW(CSWType * csw,uint8_t status,uint32_t remaining)
{
	csw->dCSWSignature = 0x53425355;
	csw->dCSWTag=currentTag;
	csw->dCSWDataResidue=remaining;
	csw->bCSWStatus=status;
}

uint8_t checkCBW(uint8_t * epBfr,uint8_t transferLength,volatile uint32_t* tag)
{
	CBWType * cbw;
	// check transfer length
	if(transferLength!=31)
	{
		return 1;
	}
	cbw = (CBWType*)epBfr;
	// check signature
	if (cbw->dCBWSignature != 0x43425355)
	{
		return 1;
	}
	if((cbw->bmCBWFlags & 0x7F)!=0) // reserved flag set, not meaningful
	{
		return 2;
	}
	if(cbw->bCBWLun != 0)
	{
		return 2;
	}
	if (cbw->bCBWCBLength < 1 || cbw->bCBWCBLength > 16)
	{
		return 2;
	}
	*tag=cbw->dCBWTag;
	return 0;
}

void initUsbDeviceDriver(UsbEndpointConfigurationType ** epsIn,UsbEndpointConfigurationType ** epsOut,void(*onConfigured)(void))
{
	ep1In.buffer=usb_dpram->epx_data+64;
    ep1In.pid=0;
    ep1In.ep_buf_ctrl=&(usb_dpram->ep_buf_ctrl[1].in);
    ep1In.epHandler=MSCDataInHandler;
    usb_dpram->ep_ctrl[1-1].in = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep1In.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);
    epsIn[1]=&ep1In;

    ep1Out.buffer=usb_dpram->epx_data+64*2;
    ep1Out.pid=0;
    ep1Out.ep_buf_ctrl=&usb_dpram->ep_buf_ctrl[1].out;
    ep1Out.epHandler=MSCDataOutHandler;
    epsOut[1]=&ep1Out;
    usb_dpram->ep_ctrl[1-1].out = (1 << EP_CTRL_ENABLE_POS) | ((uint32_t)ep1Out.buffer - (uint32_t)usb_dpram) | (2 << EP_CTRL_EP_TYPE) |  (1 << EP_CTRL_INTR_AFTER_EVERY_BUFFER_POS);
}

void generateSenseData(uint8_t*data)
{
	memset(data,0,18);
	*(data+0x0)=0xF0; // 0x70 and valid
	*(data+7)=10;
	if (currentSense==SENSE_NO_SENSE)
	{
		*(data+2)=currentSense;
		*(data+12)=0; //ASC
		*(data+13)=0; //ASCQ
	}
	else if (currentSense==SENSE_ILLEGAL_REQUEST)
	{
		*(data+2)=currentSense;
		*(data+12)=0x20; //ASC
		*(data+13)=0; //ASCQ
	}
}

static void MSCDataOutHandler()
{
	uint16_t tl = *(ep1Out.ep_buf_ctrl) & 0x3FF;
	uint8_t checkres=0;
	CBWType * cbw;
	uint32_t transferSize;
	UFIInquiryData inquiry;
	if(transferMSC.idx < transferMSC.len)
	{
		receive_next_packet(&ep1Out,&transferMSC);
	}
	else if (transferMSC.idx == transferMSC.len && transferMSC.len != 0 && transferMSC.transferInProgress==0)
	{
		transferMSC.len=0;
		transferMSC.idx=0;
		generateCSW(&currentCSW,MSC_CSW_STATUS_GOOD,0);
		usb_start_in_transfer(&ep1In,(uint8_t*)&currentCSW,sizeof(CSWType));
	}
	else
	{
		checkres = checkCBW(ep1Out.buffer,tl,&currentTag);
		if (checkres==1)
		{
			// send stall because cbw invalid
		}
		else if (checkres==2)
		{
			// ?? ignore
		}
		else // meaningful cbw found
		{
			cbw =(CBWType*)ep1Out.buffer;
			switch(*(cbw->CBWCB)) // treat commands separately
			{
				case UFI_CMD_READ10:
					transferSize = ((*(cbw->CBWCB + 7) << 8) + *(cbw->CBWCB+8))*LOGICAL_BLOCK_SIZE;
					transferMSC.address=0; //TODO: get address from somewhere, e.g. compute using lba
					transferMSC.bMaxPacketSize=PACKET_SIZE_EP2;
					transferMSC.idx=0;
					transferMSC.len= transferSize;
					send_next_packet(&ep1In,&transferMSC);
					break;
				case UFI_CMD_WRITE10:
					transferSize = ((*(cbw->CBWCB + 7) << 8) + *(cbw->CBWCB+8))*LOGICAL_BLOCK_SIZE;
					transferMSC.address=0; //TODO: get address from somewhere, e.g. compute using lba
					transferMSC.bMaxPacketSize=PACKET_SIZE_EP2;
					transferMSC.idx=0;
					transferMSC.len= transferSize;
					receive_next_packet(&ep1Out,&transferMSC);
					transferMSC.address=0; 
					packetBufferOutIdx=0;
					packetsTransferred=0;
					currentSense=SENSE_NO_SENSE;
				case UFI_CMD_READ_FORMAT_CAPACITIES:
					for(uint8_t c=0;c<12;c++)
					{
						packetBufferIn[c]=0;
					}
					packetBufferIn[3]=8; // one one format descriptor
					packetBufferIn[11]=TOTAL_SIZE_IN_BLOCKS;
					packetBufferIn[12]=2; // current format
					packetBufferIn[14]=(LOGICAL_BLOCK_SIZE & 0xFF00) >> 8;
					packetBufferIn[15]=(LOGICAL_BLOCK_SIZE & 0xFF);
					usb_start_in_transfer(&ep1In,packetBufferIn,16);
					break;
					currentSense=SENSE_NO_SENSE;
				case UFI_CMD_READ_CAPACITY:
					for(uint8_t c=0;c<8;c++)
					{
						packetBufferIn[c]=0;
					}
					packetBufferIn[3]=TOTAL_SIZE_IN_BLOCKS-1;
					packetBufferIn[6]=(LOGICAL_BLOCK_SIZE & 0xFF00) >> 8;
					packetBufferIn[7]=(LOGICAL_BLOCK_SIZE & 0xFF);
					usb_start_in_transfer(&ep1In,packetBufferIn,8);
					currentSense=SENSE_NO_SENSE;
					break;
				case UFI_CMD_INQUIRY:
					memset(packetBufferIn,sizeof(UFIInquiryDataType),0);
					inquiry=(UFIInquiryData)packetBufferIn;
					inquiry->bPeripheralDeviceType=0x1F;
					inquiry->bRemovableMedia=0;
					inquiry->bIsoEcmaAnsiVersion=0;
					inquiry->bResponseDataFormat=1;
					inquiry->bAdditionalLength=0x1F;
					memcpy(inquiry->cVendor,"SR35",sizeof("SR35"));
					memcpy(inquiry->cProductID,"PiPicoFX",sizeof("PiPicoFX"));
					memcpy(inquiry->productRevision,"0.1",4);
					usb_start_in_transfer(&ep1In,packetBufferIn,sizeof(UFIInquiryDataType));
					currentSense=SENSE_NO_SENSE;
					break;
				case UFI_CMD_REQUEST_SENSE:
					generateSenseData(packetBufferIn);
					usb_start_in_transfer(&ep1In,packetBufferIn,18);
					currentSense=SENSE_NO_SENSE;
					break;
				case SCSI_CMD_MODE_SENSE:
					memset(packetBufferIn,0,4);
					*packetBufferIn=3; // data length
					*(packetBufferIn+1)=0; // medium type
					*(packetBufferIn+2)=0; // 1 for write protected medium
					usb_start_in_transfer(&ep1In,packetBufferIn,4);
					currentSense=SENSE_NO_SENSE;
					break;
				default:
					currentSense=SENSE_ILLEGAL_REQUEST;

					break; //unhandled command
			}
		}
	}
}

static void MSCDataInHandler()
{
	if(transferMSC.transferInProgress==1)
    {
        send_next_packet(&ep1In,&transferMSC);
    }
}

uint8_t handleSetupRequestOut(UsbSetupPacket pck,UsbEndpointConfigurationType * ep)
{
	uint8_t handled=0;
	if(pck->bRequest==0xFF) // device reset
	{
		//TODO hardware-specific reset, maybe reinitialize the sd card....
		handled=1;
		usb_start_in_transfer(ep,0,0);
	}
	return handled;
}

uint8_t handleSetupRequestIn(UsbSetupPacket pck,UsbEndpointConfigurationType * ep)
{
	uint8_t handled=0;
	if (pck->bRequest == 0xFE) // get max LUN, return 0
	{
		*packetBufferIn=0;
		usb_start_in_transfer(ep,(const uint8_t*)packetBufferIn,0);
		handled=1;
	}
	return handled;
}

#endif
