#include "stdlib.h"
#include "drivers/usb.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "usb/usb_common.h"
#include "usb/usb_config.h"
#include "uart.h"
#include "usb/usb_cdc.h"
#include "memoryRegions.h"
#include "globalConfig.h"


// handlers
void(*ep0OUTHandler)(void*,uint16_t)=0;
void(*ep1OUTHandler)(void*,uint16_t)=0;
void(*ep2OUTHandler)(void*,uint16_t)=0;
void(*ep3OUTHandler)(void*,uint16_t)=0;
void(*ep4OUTHandler)(void*,uint16_t)=0;
void(*ep5OUTHandler)(void*,uint16_t)=0;
void(*ep6OUTHandler)(void*,uint16_t)=0;
void(*ep7OUTHandler)(void*,uint16_t)=0;
void(*ep8OUTHandler)(void*,uint16_t)=0;

//driver-specific handlers
void(*usbDriverResetHandler)(void)=0; // if anything special needs to be done on usb reset

endPointHandler outHandlers[9]={0,0,0,0,0,0,0,0,0};
void(*transferDoneHandlers[9])(void)={0,0,0,0,0,0,0,0,0};
uint8_t * epOutBuffers[9]={0,0,0,0,0,0,0,0,0};
uint8_t * epInBuffers[9]={0,0,0,0,0,0,0,0,0};
volatile uint16_t epOutDataCntrs[9]={0,0,0,0,0,0,0,0,0};
volatile uint16_t epInDataCntrs[9]={0,0,0,0,0,0,0,0,0};
volatile uint16_t epInBytesTransferred[9]={0,0,0,0,0,0,0,0,0};
volatile uint16_t epOutMaxPacketSizes[9]={64,0,0,0,0,0,0,0,0};
volatile uint16_t epInMaxPacketSizes[9]={64,0,0,0,0,0,0,0,0};
uint8_t ep0OutDataBfr[512];
uint8_t ep0InDataBfr[128]; 
uint8_t const* ep0InDataBfrPtr=ep0InDataBfr;

volatile uint8_t configuredAddress=0;
volatile uint8_t triggerAddressChange=0;
volatile UsbSetupPacketType setupPacket;

void OTG_FS_EP1_OUT_IRQHandler(void)
{

}

void OTG_FS_EP1_IN_IRQHandler(void)
{

}

//__ITCM_CODE_FLASH
void OTG_FS_IRQHandler(void)
{
    uint32_t coreInterrupts = USB2_OTG_FS->GINTSTS;
    uint32_t deviceInterrupts;
    uint32_t epNr;
    USB_OTG_INEndpointTypeDef * inEndpoint;
    USB_OTG_OUTEndpointTypeDef * outEndpoint;
    uint16_t upperLimit;
    //uint16_t wordCount;

    //usb reset, we know that we're in device mode
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_USBRST_Pos))
    {
        #ifdef USB_DBG
        sendStringBlocking("USB reset\r\n");
        #endif
        // unmask interrupts: INEP0 control 0 IN endpoint
        // OUTEP0 control 0 OUT endpoint
        // STUPM setup phase done
        // XFRCM transfer completed interrupt in IN endpoint
        // XFRC transfer complete interrupt in OUT endpoint
        // TOM timeout condition in IN endpoint
        USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << USB_OTG_DAINTMSK_IEPM_Pos) | (1 << USB_OTG_DAINTMSK_OEPM_Pos);
        USB2_OTG_FS_DEVICE->DOEPMSK |= (1 << USB_OTG_DOEPMSK_STUPM_Pos) | (1 << USB_OTG_DOEPMSK_XFRCM_Pos);
        USB2_OTG_FS_DEVICE->DIEPMSK |= (1 << USB_OTG_DIEPMSK_XFRCM_Pos) | (1 << USB_OTG_DIEPMSK_TOM_Pos);

        // flush all TX FIFO's
        // wait until AHB is idle
        while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U); // maybe timeout here
        USB2_OTG_FS->GRSTCTL = ((1 << USB_OTG_GRSTCTL_TXFFLSH_Pos) | (0x10 << 6));
        // wait until flushed
        while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH); // maybe timeout as well

        // flush the rx FIFO
        // wait until no traffic over AHB
        while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U); // maybe timeout here
        USB2_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH; // flush the rx FIFO
        while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH);// maybe timeout here
        for (uint8_t c = 0; c < 9; c++)
        {
            inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20*c));
            inEndpoint->DIEPCTL = 0;
            inEndpoint->DIEPTSIZ = 0;
            inEndpoint->DIEPINT  = 0xFB7F;
        }
        for (uint8_t c=0; c < 9 ; c++)
        {
            outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*c));
            outEndpoint->DOEPCTL = 0;
            outEndpoint->DOEPTSIZ = 0;
            outEndpoint->DOEPINT  = 0xFB7F;
        }

        // setup in endpoint 0
        inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE));
        inEndpoint->DIEPCTL = (1 << USB_OTG_DIEPCTL_USBAEP_Pos) | (0x40 << USB_OTG_DIEPCTL_MPSIZ_Pos) | USB_OTG_DIEPCTL_SD0PID_SEVNFRM;
        

        // set setupcount to 3 to receive up to 3 setup packages at once
        outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE));
        outEndpoint->DOEPTSIZ = (3 << USB_OTG_DOEPTSIZ_STUPCNT_Pos) | (3*8) | (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);

        USB2_OTG_FS->GINTMSK |= (1 << USB_OTG_GINTMSK_RXFLVLM_Pos); // switch on rx interrupt
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_USBRST_Pos);
        
        for(uint8_t c=0;c<32;c++)
        {
            ep0OutDataBfr[c]=0;
        }
        for (uint8_t c=1;c<9;c++)
        {
            free(epOutBuffers[c]);
            epOutBuffers[c]=0;
            epOutDataCntrs[c]=0;
        }
        epOutDataCntrs[0]=0;
        if (usbDriverResetHandler != 0)
        {
            usbDriverResetHandler();
        }
    }

    // RX FIFO not empty: something has been received
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_RXFLVL_Pos))
    {
        USB2_OTG_FS->GINTMSK &= ~(1 << USB_OTG_GINTMSK_RXFLVLM_Pos); // mask RX FIFO Interrupt

        #ifdef USB_DBG
            sendStringBlocking("USB RX\r\n");
        #endif
        uint32_t statusRegisterPopped = USB2_OTG_FS->GRXSTSP;
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_RXFLVL_Pos); // clear RX FIFO interrupt
        uint16_t bCnt = (statusRegisterPopped >> 4) & 0x7FF;
        epNr = (statusRegisterPopped) & 0xF;

        if (bCnt > 0)
        {
            uint16_t receivedWordCnt = (bCnt + 3) / 4;
            for (uint8_t c=0;c< receivedWordCnt;c++)
            {
                (void)((((struct T_UINT32_WRITE *)(void *)(epOutBuffers[epNr]+epOutDataCntrs[epNr]))->v)= (*((volatile uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE))));
                epOutDataCntrs[epNr] += 4;
            }
        }
       USB2_OTG_FS->GINTMSK |= (1 << USB_OTG_GINTMSK_RXFLVLM_Pos); // unmask RX FIFO Interrupt
    }

    // enumeration done
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_ENUMDNE_Pos))
    {
        // check that enumerated speed is correct
        #ifdef USB_DBG
        uint8_t enumeratedSpeed = (uint8_t)((USB2_OTG_FS_DEVICE->DSTS >> 1) & 0x3);
        if (enumeratedSpeed == 3)
        {
            sendStringBlocking("USB enumDone: speed correct\r\n");
        }
        else
        {
            enumeratedSpeed += 0x30;
            sendStringBlocking("USB enumDone: wrong speed ");
            sendBlocking(&enumeratedSpeed,1);
            sendStringBlocking("\r\n");
        }
        #endif
        USB2_OTG_FS_DEVICE->DCTL |= (1 << USB_OTG_DCTL_CGINAK_Pos);
        USB2_OTG_FS->GINTMSK |= (1 << USB_OTG_GINTMSK_RXFLVLM_Pos); // switch on rx interrupt
    }
    

    // usb suspended, a.k.a. cable unplugged
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_USBSUSP_Pos))
    {
        #ifdef USB_DBG
            sendStringBlocking("USB susp\r\n");
        #endif
        // TODO shutdown data streams, notify user-level code that usb has been disconnected
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_USBSUSP_Pos);
    }

    // early suspend
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_ESUSP_Pos))
    {
        #ifdef USB_DBG
        sendStringBlocking("USB early susp\r\n");
        #endif
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_ESUSP_Pos);
    }

    // start of frame
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_SOF_Pos))
    {
        #ifdef USB_DBG
            //sendStringBlocking("USB SOF\r\n");
        #endif
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_SOF_Pos);
    }

    // session request
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_SRQINT_Pos))
    {
        // handle session request, don't know what needs to be done yet...
        #ifdef USB_DBG
            sendStringBlocking("USB session req\r\n");
        #endif
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_SRQINT_Pos);
    }

    // mode mismatch interrupt, indicates software error or wrong cable attached/forcing the device to be host
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_MMIS_Pos))
    {
        #ifdef USB_DBG
            sendStringBlocking("USB mode mismatch IRQ\r\n");
        #endif
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_MMIS_Pos);
    }



    // ----------------------------------
    // ----------------------------------
    // Endpoint interrupts
    // first read OTG_GINTSTS to determined whether it is an OUT or IN endpoint interrupt
    // then read daint to find out which endpoint has been adressed
    // do what needs to be done...
    // then clear the interrupt flag using OTG_DOEPINTx or OTG_DIEPINTx
    if ((coreInterrupts & (1 << USB_OTG_GINTSTS_OEPINT_Pos)) == (1 << USB_OTG_GINTSTS_OEPINT_Pos))
    {
        #ifdef USB_DBG
        sendStringBlocking("USB OUT EP IRQ\r\n");
        #endif
        deviceInterrupts = (USB2_OTG_FS_DEVICE->DAINT >> 16) & 0xFFFF;
        epNr = 0;
        while (deviceInterrupts != 0) 
        {
            if ((deviceInterrupts & 1) != 0)
            {
                
                outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*epNr));
                uint16_t dataFetched = epOutDataCntrs[epNr];  
                epOutDataCntrs[epNr]=0;
                if (outEndpoint->DOEPINT & (1 << USB_OTG_DOEPINT_STUP_Pos))
                {
                    // flush tx endpoint 0 IN to get rid of partial data streams
                    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U); 
                    USB2_OTG_FS->GRSTCTL = ((1 << USB_OTG_GRSTCTL_TXFFLSH_Pos) | (0x0 << 6));
                    // wait until flushed
                    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH); 

                    // handle setup phase
                    for (uint8_t c=0; c<(dataFetched >> 3); c++)
                    {
                        getSetupPacket(&setupPacket,epOutBuffers[epNr] + (0 << 3));
                        ProcessUsbSetupPackage(&setupPacket);
                    }  
                    outEndpoint->DOEPINT = (1 << USB_OTG_DOEPINT_STUP_Pos);            
                }
                if ((outEndpoint->DOEPINT & (1 << USB_OTG_DOEPINT_XFRC_Pos)))
                {
                    if (outHandlers[epNr] != 0 && epOutBuffers[epNr] != 0)
                    {
                        outHandlers[epNr](epOutBuffers[epNr],dataFetched);
                    }
                    outEndpoint->DOEPINT = (1 << USB_OTG_DOEPINT_XFRC_Pos);          
                }
            }
            deviceInterrupts >>=1;
            epNr++;
        }
    }

    if ((coreInterrupts & (1 << USB_OTG_GINTSTS_IEPINT_Pos)) == (1 << USB_OTG_GINTSTS_IEPINT_Pos))
    {
        deviceInterrupts = USB2_OTG_FS_DEVICE->DAINT & 0xFFFF;
        epNr = 0;
        while(deviceInterrupts != 0)
        {
            if ((deviceInterrupts & 1) == 1)
            {
                inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20*epNr));
                if (inEndpoint->DIEPINT & (1 << USB_OTG_DIEPINT_XFRC_Pos)) // transmit done
                {
                    #ifdef USB_DBG
                    sendStringBlocking("USB Xfer Dne\r\n");
                    #endif
                    //USB2_OTG_FS_DEVICE->DIEPEMPMSK &= ~(1 << epNr);
                    
                    if (epNr==0)
                    {
                        if (epInDataCntrs[epNr] == epInBytesTransferred[epNr] 
                            && (epInBytesTransferred[epNr] % epInMaxPacketSizes[epNr])==0 
                            && epInBytesTransferred[epNr] > 0)
                        { // send a zerol length package when total length is a multiple of the maximum package size
                            prepareUSBTransfer(0,0,0);
                        }
                        else
                        {
                            prepareEP0Rception();
                        }
                    }
                    if (transferDoneHandlers[epNr] != 0)
                    {
                        transferDoneHandlers[epNr]();
                    }
                    inEndpoint->DIEPINT = (1 << USB_OTG_DIEPINT_XFRC_Pos);
                }
                if ((inEndpoint->DIEPINT & (1 << USB_OTG_DIEPINT_TXFE_Pos)) && (USB2_OTG_FS_DEVICE->DIEPEMPMSK & (1 << epNr))==1) // transmit fifo empty
                {
                    #ifdef USB_DBG
                    sendStringBlocking("USB TX Fifo empty\r\n");
                    #endif
                    upperLimit=0;
                    if ((epInDataCntrs[epNr] - epInBytesTransferred[epNr]) > (epInMaxPacketSizes[epNr]))
                    {
                        upperLimit = epInMaxPacketSizes[epNr];
                    }
                    else
                    {
                        upperLimit = epInDataCntrs[epNr] - epInBytesTransferred[epNr];
                    }
                    uint16_t wordCount = (upperLimit + 3)/4;
                    uint16_t c=0;
                    while (c< wordCount && ((inEndpoint->DTXFSTS& 0xFFFF)!=0))
                    {
                        *((volatile uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + epNr * USB_OTG_FIFO_SIZE)) = (((const struct T_UINT32_READ *)(const void *)(epInBuffers[epNr] + epInBytesTransferred[epNr]))->v);
                        epInBytesTransferred[epNr]+=4;
                        c++;
                    }   
                    if (epInBytesTransferred[epNr] >= epInDataCntrs[epNr])
                    {
                        USB2_OTG_FS_DEVICE->DIEPEMPMSK &= ~(1 << epNr);
                    }
                    inEndpoint->DIEPINT = (1 << USB_OTG_DIEPINT_TXFE_Pos);
                }
            }

            deviceInterrupts >>= 1;
            epNr++;
        }
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_IEPINT_Pos);   
    }

}

void initUSB()
{
// ---------------------
// - GPIO's and clocks -
// ---------------------
//
// enable USB voltage detector 
PWR->CR3 |=(1 << PWR_CR3_USB33DEN_Pos); 

// enable GPIO A
RCC->AHB4ENR |= (1 << RCC_AHB4ENR_GPIOAEN_Pos);
// set pll's and clock's
// enable HSI48
RCC->CR |= (1 << RCC_CR_HSI48ON_Pos);
while (!(RCC->CR && (1 << RCC_CR_HSI48RDY_Pos)));
// multiplex HSI48 to the usb block
RCC->D2CCIP2R |= (3 << RCC_D2CCIP2R_USBSEL_Pos);

// enable usb2 clock
RCC->AHB1ENR |= (1 << RCC_AHB1ENR_USB2OTGHSEN_Pos);

// --------------------------
// - Wire up pin's
// --------------------------
// connect to daisy usb D1 and D- pins to the USB PHY
//OTG_FS_DM: PA11
//OTG_FS_DP: PA12
//OTG_FS_ID: PA10
// PORTA has 0 offset from base
GPIO_TypeDef * portA = ((GPIO_TypeDef*)(GPIOA_BASE + 0*0x400));
uint32_t regval;
regval = portA->AFR[1];
regval &= ~((0xF << (4*4))) | (0xF << (3*4));
regval |= (10 << (3*4)) | (10 << (4*4));
portA->AFR[1] = regval;
regval = portA->MODER;
regval &= ~((0x3 << (11*2)) | (0x3 << (12*2)));
regval |= (0x2 << (11*2)) | (0x2 << (12*2));
portA->MODER = regval; // mode alternate function
regval = portA->PUPDR;
regval &= ~((0x3 << (11*2)) | (0x3 << (12*2))); // no pullup/pulldown
portA->PUPDR = regval;
regval = portA->OSPEEDR;
regval &= ~((0x3 << (11*2)) | (0x3 << (12*2))); // slow speed
portA->OSPEEDR = regval;

// pa9 (vbus) as input
regval = portA->AFR[1];
regval &= ~(0xF << (1*4));
portA->AFR[1] = regval;
regval = portA->MODER;
regval &= ~(0x3 << (9*2));
portA->MODER = regval;
regval = portA->PUPDR;
regval &= ~(0x3 << (9*2));
portA->PUPDR = regval;
regval = portA->OSPEEDR;
regval &= ~(0x3 << (9*2));
portA->OSPEEDR = regval;

// --------------------------
// - USB Core initialization
// --------------------------

USB2_OTG_FS->GUSBCFG |= (1 << USB_OTG_GUSBCFG_PHYSEL_Pos); // set internal USB PHY

// core clock reset
while (!(USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL));
USB2_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);

USB2_OTG_FS->GCCFG |= (1 << USB_OTG_GCCFG_PWRDWN_Pos); // enable usb transceiver
USB2_OTG_FS->GUSBCFG |= (1 << USB_OTG_GUSBCFG_FDMOD_Pos); // for device mode to be able to access device registers

// disable VBUS sensing
USB2_OTG_FS_DEVICE->DCTL |= (1 << USB_OTG_DCTL_SDIS_Pos);
USB2_OTG_FS->GCCFG &= ~(1 << USB_OTG_GCCFG_VBDEN_Pos);
USB2_OTG_FS->GOTGCTL |= (1 << USB_OTG_GOTGCTL_BVALOEN_Pos) | (1 << USB_OTG_GOTGCTL_BVALOVAL_Pos);

// restart PHY clock, set Register PCGCCTL to zero which is too far away to be part of the structure
*(volatile uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)=0;

USB2_OTG_FS_DEVICE->DCFG |= (3 << USB_OTG_DCFG_DSPD_Pos); // speed is internal phy, full speed


// mask all device interrupts
USB2_OTG_FS_DEVICE->DIEPMSK = 0;
USB2_OTG_FS_DEVICE->DOEPMSK = 0;
USB2_OTG_FS_DEVICE->DAINTMSK = 0;

// config all 9 endpoints
for (uint8_t c = 0; c < 9; c++)
{
  ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20*c))->DIEPCTL = 0;
  ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20*c))->DIEPTSIZ = 0;
  ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + 0x20*c))->DIEPINT  = 0x2B7F; // according to dataset //0xFB7F;
}
for (uint8_t c=0; c < 9 ; c++)
{
    ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*c))->DOEPCTL = 0;
    ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*c))->DOEPTSIZ = 0;
    ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*c))->DOEPINT  = 0x2B7F; // according to dataset //0xFB7F;
}

USB2_OTG_FS->GINTMSK = 0; // mask all usb interrupts
USB2_OTG_FS->GINTSTS = 0xBFFFFFFF; // clear pending interrupts
// umask all device mode interrupts
USB2_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |
USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |
USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM |
USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM;

USB2_OTG_FS_DEVICE->DCFG &= ~(0x7F << USB_OTG_DCFG_DAD_Pos); // set device address 0

// set fifo sizes
USB2_OTG_FS->GRXFSIZ = 0x80; // receiver fifo
USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ = (0x40 << 16) | 0x80; // transmit fifo 0 
//USB2_OTG_FS->DIEPTXF[0] = ((uint32_t)0x80 << 16) | (0x80 + 0x40); // transmit fifo 1



outHandlers[0]=ep0OUTHandler;
outHandlers[1]=ep1OUTHandler;
outHandlers[2]=ep2OUTHandler;
outHandlers[3]=ep3OUTHandler;
outHandlers[4]=ep4OUTHandler;
outHandlers[5]=ep5OUTHandler;
outHandlers[6]=ep6OUTHandler;
outHandlers[7]=ep7OUTHandler;
outHandlers[8]=ep8OUTHandler;

for(uint16_t c=0;c<512;c++)
{
    ep0OutDataBfr[c]=0;
}
for (uint8_t c=0;c<128;c++)
{
    ep0InDataBfr[c]=0;
}
for (uint8_t c=1;c<9;c++)
{
    free(epOutBuffers[c]);
    epOutBuffers[c]=0;
    epOutDataCntrs[c]=0;
    epInBuffers[c]=0;
    epInDataCntrs[c]=0;
}
epOutBuffers[0]=ep0OutDataBfr;
epInBuffers[0]=ep0InDataBfr;
epOutDataCntrs[0]=0;
epInDataCntrs[0]=0;

// Init specific driver
USBCDCInit();

// start phy clock and stop HCLK gating
*(volatile uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE) &= ~((1 << USB_OTG_PCGCCTL_GATECLK_Pos) | (1 << USB_OTG_PCGCCTL_STOPCLK_Pos));
USB2_OTG_FS_DEVICE->DCTL |= (1 << USB_OTG_DCTL_SDIS_Pos); // soft diconnect
USB2_OTG_FS->GAHBCFG |= (1 << USB_OTG_GAHBCFG_GINT_Pos); // unmask global interrupt
USB2_OTG_FS_DEVICE->DCTL &= ~(1 << USB_OTG_DCTL_SDIS_Pos); // finally: connect!

//NVIC: Enable USB interrupts
NVIC_EnableIRQ(OTG_FS_IRQn);

}

uint8_t const* getEp0InDataBfr()
{
    return ep0InDataBfrPtr;
}

/**
 * sends data out over usb
 * epNr: the endpoint number starting at 0
 * data: pointer to the data to be sent
 * dlen: length of the data
 */
void prepareUSBTransfer(uint8_t epNr,const uint8_t*data,uint16_t dlen)
{
    USB_OTG_INEndpointTypeDef * inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + epNr*0x20));
    uint32_t epCtrl = inEndpoint->DIEPCTL;
    uint16_t npackets;

    if(dlen==0)
    {
        npackets = 1;
        USB2_OTG_FS_DEVICE->DIEPEMPMSK &= ~(1UL << epNr);
    }
    else
    {
        npackets = ((dlen + epInMaxPacketSizes[epNr] - 1) / epInMaxPacketSizes[epNr]);
        USB2_OTG_FS_DEVICE->DIEPEMPMSK |= (1UL << epNr);
    }
    
    //for (uint16_t c=0;c<dlen;c++)
    //{
    //    epInBuffers[epNr][c] = *(data+c);
    //}
    epInBuffers[epNr] = (uint8_t*)data;
    epInBytesTransferred[epNr]=0;
    epInDataCntrs[epNr] = dlen;
    
    inEndpoint->DIEPTSIZ =  (npackets << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | dlen;
    epCtrl &= ~(1 << USB_OTG_DIEPCTL_STALL_Pos);
    epCtrl |= (1 << USB_OTG_DIEPCTL_EPENA_Pos) | (1 << USB_OTG_DIEPCTL_CNAK_Pos);
    inEndpoint->DIEPCTL = epCtrl;

}

void prepareUSBReception(uint8_t epNr,uint16_t dataSize)
{
    uint16_t nPackets = (dataSize / epOutMaxPacketSizes[epNr]) + 1;
    USB_OTG_OUTEndpointTypeDef * outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*epNr));
    outEndpoint->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & dataSize);
    outEndpoint->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (nPackets << 19));
    outEndpoint->DOEPCTL &= ~(1 << USB_OTG_DOEPCTL_STALL_Pos);
    outEndpoint->DOEPCTL |= (1 << USB_OTG_DOEPCTL_CNAK_Pos) | (1 << USB_OTG_DOEPCTL_EPENA_Pos);
}

void prepareEP0Rception(void)
{
    USB_OTG_OUTEndpointTypeDef * outEndpoint = ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + 0x20*0));
    outEndpoint->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & epOutMaxPacketSizes[0]);
    outEndpoint->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
    outEndpoint->DOEPCTL &= ~(1 << USB_OTG_DOEPCTL_STALL_Pos);
    outEndpoint->DOEPCTL |= (1 << USB_OTG_DOEPCTL_CNAK_Pos) | (1 << USB_OTG_DOEPCTL_EPENA_Pos);
}

void setEndpointOutHandler(endPointHandler handler,uint8_t epNr)
{
    outHandlers[epNr] = handler;
}

void setTransferDoneHandler(void(*handler)(void),uint8_t epNr)
{
    transferDoneHandlers[epNr] = handler;
}

void setupEndpoint(uint8_t epNr,uint8_t direction,uint16_t maxPacketSize)
{
    if (direction == EP_DIRECTION_OUT)
    {
        epOutMaxPacketSizes[epNr]=maxPacketSize;
        epOutBuffers[epNr] = malloc(maxPacketSize);
        epOutDataCntrs[epNr] = 0;

    }
    else if (direction == EP_DIRECTION_IN)
    {
        epInMaxPacketSizes[epNr] = maxPacketSize;
        epInBuffers[epNr] = malloc(maxPacketSize);
        epInDataCntrs[epNr] = 0;
        epInBytesTransferred[epNr] = 0;
    }

}

void stallInEndpoint(uint8_t epNr)
{
    USB_OTG_INEndpointTypeDef * inEndpoint = ((USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + epNr*0x20));
    inEndpoint->DIEPCTL |= (1 << USB_OTG_DIEPCTL_STALL_Pos);

    // flush the fifo
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U); // maybe timeout here
    USB2_OTG_FS->GRSTCTL = ((1 << USB_OTG_GRSTCTL_TXFFLSH_Pos) | (epNr << 6));
    // wait until flushed
    while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH); // maybe timeout as well
}

void getSetupPacket(UsbSetupPacketType*setupPacket, uint8_t*buffer)
{
    setupPacket->bmRequestType = *(buffer+0);
    setupPacket->bRequest = *(buffer+1);
    setupPacket->wValue = (uint16_t)(*(buffer+2) | (*(buffer+3) << 8));
    setupPacket->wIndex = (uint16_t)(*(buffer+4) | (*(buffer+5) << 8));
    setupPacket->wLength = (uint16_t)(*(buffer+6) | (*(buffer+7) << 8));
} 

void setAddress(uint8_t address)
{
    uint32_t dcfg = USB2_OTG_FS_DEVICE->DCFG;
    dcfg &= ~(0x7F << USB_OTG_DCFG_DAD_Pos); 
    dcfg |= (address & 0xFF) << USB_OTG_DCFG_DAD_Pos;
    USB2_OTG_FS_DEVICE->DCFG = dcfg;
}

void setResetHandler(void(*handler)(void))
{
    usbDriverResetHandler = handler;
}