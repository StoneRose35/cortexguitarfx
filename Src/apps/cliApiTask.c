#include <stdint.h>
#include "system.h"
#include "bufferedInputHandler.h"
#include "cliApiTask.h"
#include "consoleHandler.h"
#include "apiHandler.h"

static volatile uint32_t context;

BufferedInputType usbInput;
extern CommBufferType usbCommBuffer;
ConsoleType usbConsole;
ApiType usbApi;

void initCliApi()
{
    initConsole(&usbConsole);
    initApi(&usbApi);

    usbInput.api = &usbApi;
    usbInput.console = &usbConsole;
    usbInput.commBuffer=&usbCommBuffer;
    usbInput.interfaceType=BINPUT_TYPE_CONSOLE;
}

void cliApiTask(uint32_t task)
{
    if ((task & (1 << TASK_USB_CONSOLE_RX))==(1 << TASK_USB_CONSOLE_RX))
    {
        context = (1 << CONTEXT_USB);
        processInputBuffer(&usbInput);

        task &= ~(1 << TASK_USB_CONSOLE_RX);
    }
    /*
    if ((task & (1 << TASK_BT_CONSOLE_RX))==(1 << TASK_BT_CONSOLE_RX))
    {
        context = (1 << CONTEXT_BT);
        processInputBuffer(&btInput);

        task &= ~(1 << TASK_BT_CONSOLE_RX);
    }*/
    if ((task & (1 << TASK_USB_CONSOLE_TX))==(1 << TASK_USB_CONSOLE_TX))
    {
        if (sendCharAsyncUsb()==1) // only disable the task when a new dma transfer has been instantiated
        {
            task &= ~(1 << TASK_BT_CONSOLE_TX);
        }
    }
}