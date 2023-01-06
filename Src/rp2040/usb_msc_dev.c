#include "stdint.h"
#include "hardware/usb_rp2040.h"
#include "usb/usb_cdc.h"
#include "memFunctions.h"
#include "usb/usb_common.h"
#include "usb/usb_config.h"
#include "usb/usb_msc.h"

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

static void MSCDataOutHandler();
static void MSCDataInHandler();

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

static void startDataReception()
{
	 usb_start_out_transfer(&ep1Out,PACKET_SIZE_EP2); // get ready to receive data
}

void initUsbDeviceDriver(UsbEndpointConfigurationType ** epsIn,UsbEndpointConfigurationType ** epsOut,void(**onConfigured)(void))
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

	*onConfigured=startDataReception;
}

void generateSenseData(uint8_t*data)
{
	memset(data,0,18);
	*(data+0x0)=0xF0; // 0x70 and valid
	*(data+7)=10;
	*(data+2)=currentSense;
	if (currentSense==SENSE_NO_SENSE)
	{
		*(data+12)=0; //ASC
		*(data+13)=0x0; //ASCQ
	}
	else if (currentSense==SENSE_ILLEGAL_REQUEST)
	{
		*(data+12)=0x20; //ASC
		*(data+13)=0x0; //ASCQ
	}
	else if (currentSense==SENSE_NOT_READY)
	{
		*(data+12)=0x3a;
		*(data+13)=0x0;
	}
}

static void MSCDataOutHandler()
{
	uint16_t tl = *(ep1Out.ep_buf_ctrl) & 0x3FF;
	uint8_t checkres=0;
	CBWType * cbw;
	uint32_t transferSize;
	UFIInquiryData inquiry;
	if(transferMSC.idx < transferMSC.len && transferMSC.transferInProgress==1)
	{
		memcpy((uint8_t*)(transferMSC.address+transferMSC.idx),ep1Out.buffer,*ep1Out.ep_buf_ctrl & 0x3FF);
		usb_msc_handle_received_packet(&transferMSC); //TODO do something with the received packet, should block until the usb controller is ready to receive another packet
		receive_next_packet(&ep1Out,&transferMSC);
	}
	else if (transferMSC.idx == transferMSC.len && transferMSC.len != 0 && transferMSC.transferInProgress==0)
	{
		transferMSC.len=0;
		transferMSC.idx=0;
		memcpy((uint8_t*)(transferMSC.address+transferMSC.idx),ep1Out.buffer,*ep1Out.ep_buf_ctrl & 0x3FF);
		usb_msc_handle_received_packet(&transferMSC); //TODO 
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
					usb_handle_read10_request(&transferMSC, (*(cbw->CBWCB + 2) << 24) + (*(cbw->CBWCB + 3) << 16) + (*(cbw->CBWCB + 4) << 8) + (*(cbw->CBWCB + 5)), ((*(cbw->CBWCB + 7) << 8) + *(cbw->CBWCB+8))); //TODO
					transferMSC.bMaxPacketSize=PACKET_SIZE_EP2;
					transferMSC.idx=0;
					transferMSC.len= transferSize;
					send_next_packet(&ep1In,&transferMSC);
					currentSense=SENSE_NO_SENSE;
					break;
				case UFI_CMD_WRITE10:
					transferSize = ((*(cbw->CBWCB + 7) << 8) + *(cbw->CBWCB+8))*LOGICAL_BLOCK_SIZE;
					transferMSC.address=0; //TODO: get address from somewhere, e.g. compute using lba
					usb_handle_write10_request(&transferMSC,(*(cbw->CBWCB + 2) << 24) + (*(cbw->CBWCB + 3) << 16) + (*(cbw->CBWCB + 4) << 8) + (*(cbw->CBWCB + 5)), ((*(cbw->CBWCB + 7) << 8) + *(cbw->CBWCB+8))); //TODO
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
				case SCSI_CMD_TEST_UNIT_READY:
					// maybe set sense to SENSE_NOT_READY in case the drive is not ready
					currentSense=SENSE_NO_SENSE;
					break;
				case SCSI_CMD_START_STOP_UNIT:
					// maybe start or stop the unit when removed via the host
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
		usb_msc_handle_sent_packet(&transferMSC); //TODO possibly prepare data to be sent, may block until data is ready
        send_next_packet(&ep1In,&transferMSC);
    }
}

uint8_t handleSetupRequestOut(UsbSetupPacket pck,UsbEndpointConfigurationType * ep)
{
	uint8_t handled=0;
	if(pck->bRequest==0xFF) // device reset
	{
		//TODO hardware-specific reset, maybe reinitialize the sd card....
		usb_msc_handle_reset();
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