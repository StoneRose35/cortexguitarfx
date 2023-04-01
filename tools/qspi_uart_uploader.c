#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "time.h"

union datalength
{
    __uint32_t binFileLength;
    __uint8_t data[4];
};
#define ACK_TIMEOUT 1000
int writeUartArray(int fd,const __uint8_t * data, __uint32_t len)
{

    for(__uint32_t c=0;c<len;c++)
    {
        if (write(fd,data+c,1)!=1)
        {
            return 1;
        }
        tcdrain(fd);
    }
    return 0;
}

int waitForAck(int fd)
{
    char databuffer[3];
    const char ack[]={'A', 'C', 'K'};
    __uint8_t cnt=0;
    clock_t t;
    t = clock();
    while(cnt < 3 && (clock() - t) < ACK_TIMEOUT*CLOCKS_PER_SEC)
    {
        if(read(fd,databuffer+cnt,1)>0)
        {
            cnt++;
        }
    }
    for (__uint8_t c=0;c<3;c++)
    {
        if(*(databuffer+c)!=*(ack+c))
        {
            return 1;        
        }
    }
    return 0;
}

int main(int argc,char ** argv)
{
    FILE * binfile;
    int uploader;
    struct termios uartconfig;
    __uint8_t pageBuffer[256];
    if (argc < 2)
    {
        printf("please specify input bin-file and device-file for uploading\r\n");
        printf("example: qspi_uart_uploader qspidata.bin /dev/ttyACM0\r\n");
        return 1;
    }
    printf(" Starting flashing of %s via %s\r\n",argv[1], argv[2]);
    binfile = fopen(argv[1],"rb");
    if (binfile==NULL)
    {
        printf("opening binary file %s failed\r\n",argv[1]);
        return 1;
    }
    uploader = open(argv[2],O_RDWR | O_NOCTTY | O_NDELAY);
    if (uploader==-1)
    {
        printf("opening the device %s failed\r\n",argv[2]);
        fclose(binfile);
        return 1;
    }
    if(!isatty(uploader))
    {
        printf("%s is not a serial device\r\n",argv[2]);
        close(uploader);
        fclose(binfile);
        return 1;
    }
    if(tcgetattr(uploader, &uartconfig) < 0) {
        printf("unable to open serial device configuration\r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }

    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    uartconfig.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);

    // no case mapping, no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no fill characters, ,
    // no local output processing
    //
    uartconfig.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | OFILL | OPOST);

    // echo off, echo newline off, canonical mode off,
    // extended input processing off, signal chars off
    uartconfig.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    // clear current char size mask, no parity checking,
    // no output processing, force 8 bit input
    uartconfig.c_cflag &= ~(CSIZE | PARENB);
    uartconfig.c_cflag |= CS8;

    // one byte at the time, no inter-byte timer
    uartconfig.c_cc[VMIN]  = 1;
    uartconfig.c_cc[VTIME] = 0;

    if(cfsetispeed(&uartconfig, B57600) < 0 || cfsetospeed(&uartconfig, B57600) < 0) {
        printf("could not set baud rate of 57600baud \r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }

    tcsetattr(uploader,TCSANOW,&uartconfig);

    // main workflow
    const __uint8_t identifier[] = {0x23, 0xed, 0x12, 0xf3, 0x11, 0x6a, 0xc,0xa2};
    if (writeUartArray(uploader,identifier,sizeof(identifier))!=0)
    {
        printf("writing identifier failed\r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }
    printf("waiting for identifier ACK\r\n");
    if (waitForAck(uploader)!=0)
    {
        printf("identifier not acknowledged\r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }

    // send file length
    fseek(binfile,0L,SEEK_END);
    __uint32_t flength = (__uint32_t)ftell(binfile);
    fseek(binfile, 0L, SEEK_SET);
    union datalength dl;
    dl.binFileLength = flength;
    if (writeUartArray(uploader,dl.data,4)!=0)
    {
        printf("writing bin file length failed\r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }
    printf("waiting for bin file length ACK\r\n");
    if (waitForAck(uploader)!=0)
    {
        printf("bin file length not acknowledged\r\n");
        close(uploader);
        fclose(binfile);
        return 1;
    }
    // send pages, wait for ack after each page
    __uint32_t nPages = (flength >> 8)+1;
    printf("writing %d pages\r\n",nPages);
    size_t nChars;
    for(__uint16_t c=0;c<nPages;c++)
    {
        if(c==nPages-1)
        {
            nChars = flength - (c << 8);
        }
        else
        {
            nChars=256;
        }
        if (nChars == 0)
        {
            printf("\r\nflashing successful!\r\n");
            return 0;
        }
        if (fread(pageBuffer,nChars,1,binfile)!=1)
        {
            printf("failed reading %lu bytes from bin file\r\n",nChars);
            return 1;
        }
        //printf("\rwriting page %d\r\n",c);
        if (writeUartArray(uploader,pageBuffer,nChars)!=0)
        {
            printf("writing page %hu failed\r\n",c);
            close(uploader);
            fclose(binfile);
            return 1;
        }
        if (waitForAck(uploader)!=0)
        {
            printf("ACK for page %hu failed\r\n",c);
            close(uploader);
            fclose(binfile);
            return 1;
        }
        printf("\rPage (%d/%d) written      ",c+1,nPages);
    }

    printf("\r\nflashing successful!\r\n");

    return 0;
}