#include "stdlib.h"
#include "stdint.h"
#include "tusb.h"
#include "bufferedInputHandler.h"
#include "consoleHandler.h"

extern uint32_t task;

void cdc_write(CommBuffer bfr, uint8_t blocking)
{
  uint32_t len;
  uint32_t offset;
  uint8_t * str;
  getOutputBuffer(bfr,&len,&offset);
  if (len > 0)
  {
    str = (uint8_t*)malloc(len);
    for(uint32_t c=0;c<len;c++)
    {
      *(str + c) = *(bfr->outputBuffer + ((offset + c) & ((1 << OUTPUT_BUFFER_SIZE)-1)));
    }
    tud_cdc_write(str, len);
    tud_cdc_write_flush();
    consumeOutputBufferBytes(bfr,len);
  }
  /*
  while (*(str+len)!= 0)
  {
    len++;
  }
  tud_cdc_write(str, len);
  tud_cdc_write_flush();
  */
}

void cdc_task(CommBuffer bfr)
{
    if ( tud_cdc_available() )
    {
      // read data
      char buf[64];
      uint16_t count = tud_cdc_read(buf, sizeof(buf));
      //for (uint16_t c=0;c<count;c++)
      //{
        appendToInputBuffer(bfr,(uint8_t*)buf,count);
        //usbCommBuffer.inputBuffer[usbCommBuffer.inputBufferCnt++]=buf[c];
		    //usbCommBuffer.inputBufferCnt &= (INPUT_BUFFER_SIZE-1);
      //}
      //processInputBuffer(&usbInput);

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
      //tud_cdc_write(buf, count);
      //tud_cdc_write_flush();
    }
    //getOutputBuffer(&usbCommBuffer,&len,&offset);
    //if(len > 0)
    //{
    //  (&usbCommBuffer)->bufferConsumer(&usbCommBuffer,0);
    //}
    
}



/*
generate filesystem

mkfs.vfat -C -D0x80 -F12 -M0xF8 -f1 -h0 -nPIPICOFX -R1 -s1 -S512 ./fstest.bin 8

*/

const uint8_t msc_disk[16*512];

uint8_t ejected=0;
// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "SR35";
  const char pid[] = "PiPicoFX";
  const char rev[] = "0.1";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

  // RAM disk is ready until ejected
  if (ejected) {
    // Additional Sense 3A-00 is NOT_FOUND
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }

  return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
  (void) lun;

  *block_count = 16;
  *block_size  = 512;
}


// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  (void) lun;

  // out of ramdisk
  if ( lba >= 16 ) return -1;

  uint8_t const* addr = msc_disk + offset; //msc_disk[lba] + offset;
  memcpy(buffer, addr, bufsize);

  return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
  (void) lun;

#ifdef CFG_EXAMPLE_MSC_READONLY
  return false;
#else
  return true;
#endif
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  (void) lun;

  // out of ramdisk
  if ( lba >= 16 ) return -1;

//#ifndef CFG_EXAMPLE_MSC_READONLY
//  uint8_t* addr = msc_disk[lba] + offset;
//  memcpy(addr, buffer, bufsize);
//#else
(void) lba; (void) offset; (void) buffer;
//#endif

  return (int32_t) bufsize;
}