#ifndef USB_MSC_H_
#define USB_MSC_H_
#include "stdint.h"
#include "usb_common.h"

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
#define SCSI_CMD_TEST_UNIT_READY 0x0
#define SCSI_CMD_START_STOP_UNIT 0x1B


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
	uint8_t bIsoEcmaAnsiVersion; // always 2
	uint8_t bResponseDataFormat; //always 2
	uint8_t bAdditionalLength; // always 31
	uint8_t res[3];
	char cVendor[8]; // vendor string
	char cProductID[16]; // product id
	char productRevision[4]; // product revision aka firmware version
} UFIInquiryDataType;

typedef UFIInquiryDataType* UFIInquiryData;

void usb_handle_read10_request(UsbMultipacketTransfer *  transfer, uint32_t blockStart, uint16_t nBlocks);

void usb_handle_write10_request(UsbMultipacketTransfer *  transfer, uint32_t blockStart, uint16_t nBlocks);

void usb_msc_handle_reset();

void usb_msc_handle_received_packet(UsbMultipacketTransfer* transfer);

void usb_msc_handle_sent_packet(UsbMultipacketTransfer* transfer);

#endif