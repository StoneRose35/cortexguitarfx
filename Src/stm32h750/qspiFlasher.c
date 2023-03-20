#include "stdint.h"
#include "uart.h"
#include "i2s.h"
#include "stm32h750/stm32h7xx.h"
#include "system.h"
#include "qspi.h"

#define QSPI_PROGRAMMER_IDLE 0
#define QSPI_PROGRAMMER_IDENTIFIER_RECEIVED 1
#define QSPI_PROGRAMMER_LENGTH_RECEIVED 2
#define QSPI_PROGRAMMER_START_TASK 3
extern uint32_t task;
static uint8_t uartInputBfr[256];
static uint32_t uartInputBfrCntr=0;
const uint8_t qspiProgrammerIdentifier[] = {0x23, 0xed, 0x12, 0xf3, 0x11, 0x6a, 0xc,0xa2};
const uint8_t ack[] = {'A', 'C', 'K'};
// 0: idle, 1: identifier received, 2: length received, 3: data package received
volatile uint32_t qspiFlashingState=0;
volatile uint32_t qspiPageCntr=0;
union datalength
{
    uint32_t binFileLength;
    uint8_t data[4];
} dl ;


void sendAck()
{
    sendBlocking(ack,3);
}


void processUartReception(uint8_t receivedByte)
{
    switch(qspiFlashingState)
        {
            case QSPI_PROGRAMMER_IDLE:
                uartInputBfr[uartInputBfrCntr] = receivedByte;
                if (uartInputBfr[uartInputBfrCntr] != qspiProgrammerIdentifier[uartInputBfrCntr])
                {
                    uartInputBfrCntr = 0;
                }
                else
                {
                    uartInputBfrCntr++;
                }
                if (uartInputBfrCntr == 8)
                {
                    qspiFlashingState=QSPI_PROGRAMMER_IDENTIFIER_RECEIVED;
                    uartInputBfrCntr = 0;
                    sendAck();
                }
                break;
            case QSPI_PROGRAMMER_IDENTIFIER_RECEIVED:
                dl.data[uartInputBfrCntr++] = receivedByte;
                if (uartInputBfrCntr==4)
                {
                    qspiFlashingState = QSPI_PROGRAMMER_START_TASK;
                    __NVIC_DisableIRQ(USART1_IRQn);
                    task |= (1 << TASK_FLASH_QSPI);
                }
                break;
        }
}

void flashingTask()
{
    uint32_t bytesReceived=0;
    // call chip erase
    setQspiStatus(2);
    endMemoryMappedMode();
    QspiEraseChip();
    sendAck();
    uartInputBfrCntr=0;
    while(bytesReceived < dl.binFileLength)
    {
        
        receiveBlocking(uartInputBfr + uartInputBfrCntr++,1);
        if (uartInputBfrCntr == 256 || (qspiPageCntr==(dl.binFileLength >> 8) 
            && uartInputBfrCntr == (dl.binFileLength - (qspiPageCntr << 8))))
        {
            // call program page
            QspiProgramPage((qspiPageCntr << 8), uartInputBfr);
            if (uartInputBfrCntr==256)
            {
                qspiPageCntr++;
                if ((dl.binFileLength - (qspiPageCntr << 8))==0)
                {
                    qspiFlashingState=QSPI_PROGRAMMER_IDLE;
                    setMemoryMappedMode();
                    setQspiStatus(0);
                }
            }
            else
            {
                qspiFlashingState=QSPI_PROGRAMMER_IDLE;
                setMemoryMappedMode();
                setQspiStatus(0);

            }
            uartInputBfrCntr=0;
            sendAck();
        }
        bytesReceived++;
    }
    __NVIC_EnableIRQ(USART1_IRQn);
}


