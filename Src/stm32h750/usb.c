#include "stdlib.h"
#include "drivers/usb.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "usb/usb_common.h"
#include "usb/usb_config.h"
#include "uart.h"


// handler
void(*ep0OUTHandler)(void*data)=0;
void(*ep1OUTHandler)(void*data)=0;
void(*ep2OUTHandler)(void*data)=0;
void(*ep3OUTHandler)(void*data)=0;
void(*ep4OUTHandler)(void*data)=0;
void(*ep5OUTHandler)(void*data)=0;
void(*ep6OUTHandler)(void*data)=0;
void(*ep7OUTHandler)(void*data)=0;
void(*ep8OUTHandler)(void*data)=0;

void OTG_FS_EP1_OUT_IRQHandler(void)
{

}

void OTG_FS_EP1_IN_IRQHandler(void)
{

}

void OTG_FS_IRQHandler(void)
{
    uint32_t coreInterrupts = USB2_OTG_FS->GINTSTS;

    //usb reset, we know that we're in device mode
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_USBRST_Pos))
    {
        #ifdef USB_DBG
        sendStringBlocking("USB reset\r\n");
        #endif
        // set NAK for all OUT endpoints
        for (uint8_t c=0;c<8;c++)
        {
            *((uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (c+1)*0x20)) |= (1 << USB_OTG_DOEPCTL_SNAK_Pos);
        }
        // unmask interrupts: INEP0 control 0 IN endpoint
        // OUTEP0 control 0 OUT endpoint
        // STUPM setup phase done
        // XFRCM transfer completed interrupt in IN endpoint
        // XFRC transfer complete interrupt in OUT endpoint
        // TOM timeout condition in IN endpoint
        USB2_OTG_FS_DEVICE->DAINTMSK |= (1 << USB_OTG_DAINTMSK_IEPM_Pos) | (1 << USB_OTG_DAINTMSK_OEPM_Pos);
        USB2_OTG_FS_DEVICE->DOEPMSK |= (1 << USB_OTG_DOEPMSK_STUPM_Pos) | (1 << USB_OTG_DOEPMSK_XFRCM_Pos);
        USB2_OTG_FS_DEVICE->DIEPMSK |= (1 << USB_OTG_DIEPMSK_XFRCM_Pos) | (1 << USB_OTG_DIEPMSK_TOM_Pos);

        // set fifo ram size to 64 bytes plus 12*4bytes, rounded up -> 128bytes
        USB2_OTG_FS->GRXFSIZ=0x80;//0x400; // stay with default
        USB2_OTG_FS->DIEPTXF0_HNPTXFSIZ= (0x200 << 16) | (0x200 << 16);

        // set setupcount to 3 to receive up to 3 setup packages at once
        ((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE))->DOEPTSIZ |= 3 << USB_OTG_DOEPTSIZ_STUPCNT_Pos;

        USB2_OTG_FS_DEVICE->DCFG &= ~(0x7F << USB_OTG_DCFG_DAD_Pos); // set device address 0 on reset
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_USBRST_Pos);

    }

    // RX FIFO not empty: something has been received
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_RXFLVL_Pos))
    {

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
        USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_ENUMDNE_Pos);
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
            sendStringBlocking("USB SOF\r\n");
        #endif
        // maybe useful to measure the liveliness of the usb host...
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
    if (coreInterrupts & (1 << USB_OTG_GINTSTS_OEPINT_Pos))
    {
        uint32_t deviceInterrupts = (USB2_OTG_FS_DEVICE->DAINT >> 16) & 0xFFFF;
        for (uint8_t c=0;c<9;c++)
        {
            if (deviceInterrupts & 1)
            {
                if (c==0) // endpoint 0 OUT, check for setup packets
                {
                    if (((USB_OTG_OUTEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE))->DOEPINT & (1 << USB_OTG_DOEPINT_STUP_Pos))
                    {
                        // handle setup phase
                        uint32_t packageInfo = USB2_OTG_FS->GRXSTSP;
                        uint8_t byteCount = (packageInfo >> 3) & 0x3FF;
                        // consistency: endpoint should be 0 and
                        // PKTSTS (bits 20:17) should be 4 (SETUP transaction completed)
                        
                        if ((byteCount >> 2) > 0)
                        {
                            uint32_t * setupdata;
                            setupdata = (uint32_t*)malloc(byteCount);
                            for (uint8_t c=0;c<(byteCount >> 2);c++)
                            {
                                *(setupdata + c) = *((uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE));
                            }
                            // do whatever is needed when a setup packet has been received
                            UsbSetupPacketType * setupPacket = (UsbSetupPacketType*)setupdata;
                            switch (setupPacket->mbRequestType)
                            {
                                case SETUP_PACKET_REQ_GET_DESCRIPTOR:
                                    
                                    break;
                            }
                            free(setupdata);
                        }
                        
                    }
                    else if (ep0OUTHandler!=0)
                    {

                        //ep0OUTHandler()
                    }
                }
                else
                {

                }
            }
            deviceInterrupts >>=1;
        }
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

for (uint8_t c=0;c<15;c++)
{
    USB2_OTG_FS->DIEPTXF[c]=0U;
}

// disable VBUS sensing
USB2_OTG_FS_DEVICE->DCTL |= (1 << USB_OTG_DCTL_SDIS_Pos);
USB2_OTG_FS->GCCFG &= ~(1 << USB_OTG_GCCFG_VBDEN_Pos);
USB2_OTG_FS->GOTGCTL |= (1 << USB_OTG_GOTGCTL_BVALOEN_Pos) | (1 << USB_OTG_GOTGCTL_BVALOVAL_Pos);

// restart PHY clock, set Register PCGCCTL to zero which is too far away to be part of the structure
*(volatile uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)=0;

USB2_OTG_FS_DEVICE->DCFG |= (3 << USB_OTG_DCFG_DSPD_Pos); // speed is internal phy, full speed

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

USB2_OTG_FS_DEVICE->DIEPMSK &= ~(1 << USB_OTG_DIEPEACHMSK1_TXFURM_Pos); // mask TX underrun interrupt, pretty useless since all interrupts have meen masked before

USB2_OTG_FS->GINTMSK = 0; // mask all usb interrupts
USB2_OTG_FS->GINTSTS = 0xBFFFFFFF; // clear pending interrupts
// umask all device mode interrupts
USB2_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |
USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |
USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM |
USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM;

USB2_OTG_FS_DEVICE->DCFG &= ~(0x7F << USB_OTG_DCFG_DAD_Pos); // set device address 0

// start phy clock and stop HCLK gating
*(volatile uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE) &= ~((1 << USB_OTG_PCGCCTL_GATECLK_Pos) | (1 << USB_OTG_PCGCCTL_STOPCLK_Pos));
USB2_OTG_FS_DEVICE->DCTL |= (1 << USB_OTG_DCTL_SDIS_Pos); // soft diconnect
USB2_OTG_FS->GAHBCFG |= (1 << USB_OTG_GAHBCFG_GINT_Pos); // unmask global interrupt
USB2_OTG_FS_DEVICE->DCTL &= ~(1 << USB_OTG_DCTL_SDIS_Pos); // finally: connect!

//NVIC: Enable USB interrupts
NVIC_EnableIRQ(OTG_FS_IRQn);

// so far everythin is done, the remaining initialization is interrupt based

/*

// core initialization according to 57.15.1 from reference manual
USB2_OTG_FS->GAHBCFG |= (1 << USB_OTG_GAHBCFG_GINT_Pos);// | (1 << USB_OTG_GAHBCFG_PTXFELVL_Pos);
//USB2_OTG_FS->GINTSTS |= (1 << USB_OTG_GINTSTS_RXFLVL_Pos); // Rx FIFO non-empty

USB2_OTG_FS->GUSBCFG &= ~((1 << USB_OTG_GUSBCFG_HNPCAP_Pos));
USB2_OTG_FS->GUSBCFG |= (1 << USB_OTG_GUSBCFG_PHYSEL_Pos) | (1 << USB_OTG_GUSBCFG_SRPCAP_Pos) | (1 << USB_OTG_GUSBCFG_FDMOD_Pos); // set usb 1.1 full speed with internal transceiver

while (!(USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL));
USB2_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
while ((USB2_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);

//USB2_OTG_FS->GCCFG |= (1 << USB_OTG_GCCFG_PWRDWN_Pos);

USB2_OTG_FS->GUSBCFG &= ~(0xF << USB_OTG_GUSBCFG_TRDT_Pos);
USB2_OTG_FS->GUSBCFG |= (0x9 << USB_OTG_GUSBCFG_TRDT_Pos); 
USB2_OTG_FS->GINTMSK |= (1 << USB_OTG_GINTMSK_OTGINT_Pos) | (1 << USB_OTG_GINTMSK_MMISM_Pos) | (1 << USB_OTG_GINTMSK_SRQIM_Pos);

// device initialization according to 57.15.3 from reference manual
//USB2_OTG_FS_DEVICE->DCFG &= ~(3 << USB_OTG_DCFG_DSPD_Pos);
USB2_OTG_FS_DEVICE->DCFG |= (3 << USB_OTG_DCFG_DSPD_Pos);
USB2_OTG_FS_DEVICE->DCFG &= ~(1 << USB_OTG_DCFG_NZLSOHSK_Pos);
USB2_OTG_FS_DEVICE->DCFG &= ~(0x7F << USB_OTG_DCFG_DAD_Pos); // set device address 0 on startup

USB2_OTG_FS_DEVICE->DCTL &= ~(1 << USB_OTG_DCTL_SDIS_Pos);
USB2_OTG_FS->GINTMSK |= (1 << USB_OTG_GINTMSK_RSTDEM_Pos) 
                      | (1 << USB_OTG_GINTMSK_ENUMDNEM_Pos) 
                      | (1 << USB_OTG_GINTMSK_ESUSPM_Pos) 
                      | (1 << USB_OTG_GINTMSK_USBSUSPM_Pos)
                      | (1 << USB_OTG_GINTMSK_SOFM_Pos);
*/

}