#include <stdint.h>
#include "system.h"
#include "bufferedInputHandler.h"
#include "cliApiTask.h"
#include "consoleHandler.h"
#include "apiHandler.h"
#include "consoleBase.h"

//static volatile uint32_t context;

//BufferedInputType usbInput;
//extern CommBufferType usbCommBuffer;
//ConsoleType usbConsole;
//ApiType usbApi;

void initCliApi(BufferedInputType * bfr,ConsoleType * console,ApiType* api,CommBuffer commBfr, bufferConsumerType consumer)
{
    if (console !=0)
    {
        initConsole(console);
        bfr->console = console;
    }
    if (api != 0)
    {
        initApi(api);
        bfr->api = api;
    }
    if (commBfr != 0)
    {
        bfr->commBuffer = commBfr;
        if (consumer != 0)
        {
            bfr->commBuffer->bufferConsumer = consumer;
        }
    }

    bfr->interfaceType=BINPUT_TYPE_CONSOLE;
}

void cliApiTask(BufferedInput input)
{
    uint32_t len,offset;
    //if ((task & (1 << TASK_USB_CONSOLE_RX))==(1 << TASK_USB_CONSOLE_RX))
    //{
    //    context = (1 << CONTEXT_USB);
    processInputBuffer(input);

    //    task &= ~(1 << TASK_USB_CONSOLE_RX);
    //}

    getOutputBuffer(input->commBuffer,&len,&offset);
    if(len > 0)
    {
      (input->commBuffer)->bufferConsumer(input->commBuffer,0);
    }
    /*
    if ((task & (1 << TASK_BT_CONSOLE_RX))==(1 << TASK_BT_CONSOLE_RX))
    {
        context = (1 << CONTEXT_BT);
        processInputBuffer(&btInput);

        task &= ~(1 << TASK_BT_CONSOLE_RX);
    }*/
    //sendCharAsyncUsb();
    //if ((task & (1 << TASK_USB_CONSOLE_TX))==(1 << TASK_USB_CONSOLE_TX))
    //{
    //    if (sendCharAsyncUsb()==1) // only disable the task when a new dma transfer has been instantiated
    //    {
    //        task &= ~(1 << TASK_USB_CONSOLE_TX);
    //    }
    //}
}