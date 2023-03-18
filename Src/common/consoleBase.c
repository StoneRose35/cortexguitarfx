#include <stdint.h>
#include "system.h"
#include "bufferedInputHandler.h"

extern uint8_t context; // used by printf to decide where a certain information should be output

CommBufferType usbCommBuffer __attribute__((aligned (256)));

CommBufferType btCommBuffer;

// TODO: properly handle the blocked case
// it seems data is sent multiple time when the output buffer would run over
void printf(const char* data)
{
	uint32_t cnt = 0;
	uint8_t cur_data;
	uint8_t sc_res;
	cur_data = *(data + cnt);
	while (cur_data != 0)
	{
		if ((context & (1 << CONTEXT_USB)) == (1 << CONTEXT_USB))
		{
			*(usbCommBuffer.outputBuffer+usbCommBuffer.outputBufferReadCnt) = *(data + cnt);
			usbCommBuffer.outputBufferReadCnt++;
			usbCommBuffer.outputBufferReadCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);

			if (usbCommBuffer.outputBufferReadCnt==((usbCommBuffer.outputBufferWriteCnt-1)& ((1 << OUTPUT_BUFFER_SIZE)-1))) // ring buffer full
			{
			    sc_res = sendCharAsyncUsb();
				while (sc_res == 0)
				{
					sc_res = sendCharAsyncUsb();
				}
			}
		}
		if ((context & (1 << CONTEXT_BT)) == (1 << CONTEXT_BT))
		{
			*(btCommBuffer.outputBuffer+btCommBuffer.outputBufferReadCnt) = *(data + cnt);
			btCommBuffer.outputBufferReadCnt++;
			btCommBuffer.outputBufferReadCnt &= ((1 << OUTPUT_BUFFER_SIZE)-1);

			if (btCommBuffer.outputBufferReadCnt==((btCommBuffer.outputBufferWriteCnt-1)& ((1 << OUTPUT_BUFFER_SIZE)-1))) // ring buffer full
			{
				sc_res = sendCharAsyncBt();
				while (sc_res == 0)
				{
					sc_res = sendCharAsyncBt();
				}
			}
		}
		cnt++;
		cur_data = *(data + cnt);
	}
}