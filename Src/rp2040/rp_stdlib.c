#include <sys/stat.h>
#include "uart.h"
#include "bufferedInputStructs.h"

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;
extern void(*_reset)();

void _exit()
{
    _reset();
}

int _close()
{
    return -1;
}

int _lseek(int file, int offset, int whence) {
  return 0;
}

void _kill(int pid, int sig) {
  return;
}

int _getpid(void) {
  return -1;
}

int _isatty(int file) {
  return 1;
}

int _fstat(int file, struct stat *st) {
  return 0;
}

/**
 * @brief funky implementation of write, file 4 is the usb uart and file 8 is the bluetooth uart 
 *        the character are put into the ouput buffer and written to the uarts (blocking) if the buffer is full
 * @param fd 
 * @param buf 
 * @param count 
 * @return int 
 */
int _write (int fd, char *buf, int count) {
  int written = 0;
  for(;count>0;--count)
  {
		if ((fd & 4) == 4)
		{
			*(usbCommBuffer.outputBuffer+usbCommBuffer.outputBufferReadCnt) = *(buf + written);
			usbCommBuffer.outputBufferReadCnt++;
			usbCommBuffer.outputBufferReadCnt &= (OUTPUT_BUFFER_SIZE-1);

			if (usbCommBuffer.outputBufferReadCnt==(OUTPUT_BUFFER_SIZE-1))
			{
				uint8_t sc_res = sendCharAsyncUsb();
				while (sc_res == 0)
				{
					sc_res = sendCharAsyncUsb();
				}
			}

		}
		if ((fd & 8) == 8)
		{
			*(btCommBuffer.outputBuffer+btCommBuffer.outputBufferReadCnt) = *(buf + written);
			btCommBuffer.outputBufferReadCnt++;
			btCommBuffer.outputBufferReadCnt &= (OUTPUT_BUFFER_SIZE-1);

			if (btCommBuffer.outputBufferReadCnt==(OUTPUT_BUFFER_SIZE-1))
			{
				uint8_t sc_res = sendCharAsyncBt();
				while (sc_res == 0)
				{
					sc_res = sendCharAsyncBt();
				}
			}
		}
		written++;
  }
  return written;
}

int _read (int fd, char *buf, int count) {
  int read = 0;

  for (; count > 0; --count) {
    if ((fd & 4) == 4)
		{
      if (usbCommBuffer.inputBufferCnt < INPUT_BUFFER_SIZE)
      {
        *(buf+read) = *(usbCommBuffer.inputBuffer + usbCommBuffer.inputBufferCnt++);
      }
    }
    if ((fd & 8) == 8)
    {
      if (btCommBuffer.inputBufferCnt < INPUT_BUFFER_SIZE)
      {
        *(buf+read) = *(btCommBuffer.inputBuffer + btCommBuffer.inputBufferCnt++);
      }
    }
    read++;
  }
  return read;
}

