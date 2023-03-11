#include "fmc.h"
#include "stm32h750/stm32h750xx.h"
#include "stdint.h"
#include "systick.h"

/**
 * @brief SDRAM Implementation for the Daisy Seed Board, Config Values directly taken from 
 *        https://github.com/electro-smith/libDaisy
 * 
 */

typedef struct 
{
    uint8_t commandMode;
    uint8_t autoRefreshNumber;
    uint16_t modeRegisterDefinition;
} FmcSdramCommandType;



void setFmcGpio(const uint8_t * pins,uint8_t npins,uint32_t port)
{
    GPIO_TypeDef *gpio;
    uint32_t moderBfr,otypeBfr,pupdBfr,ospeedBfr,afrlBfr,afrhBfr;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);

    moderBfr = gpio->MODER;
    otypeBfr = gpio->OTYPER;
    pupdBfr = gpio->PUPDR;
    ospeedBfr = gpio->OSPEEDR;
    afrlBfr = gpio->AFR[0];
    afrhBfr = gpio->AFR[1];

    for(uint8_t c=0;c<npins;c++)
    {
        moderBfr &= ~(3 << ((pins[c] & 0xF)<<1));
        moderBfr |= (2 << ((pins[c] & 0xF)<<1));

        otypeBfr &= ~(1 << (pins[c] & 0xF));

        pupdBfr &= ~(3 << ((pins[c] & 0xF)<<1));

        ospeedBfr |= (3 << ((pins[c] & 0xF)<<1));


        if (((pins[c] & 0xF)>>3) != 0)
        {
            afrhBfr &= ~(0xF << ((pins[c] & 0x7) << 2));
            afrhBfr |= 12 << ((pins[c] & 0x7) << 2);
        }
        else
        {
            afrlBfr &= ~(0xF << ((pins[c] & 0x7) << 2));
            afrlBfr |= 12 << ((pins[c] & 0x7) << 2);
        }
    }
    gpio->MODER = moderBfr;
    gpio->OTYPER = otypeBfr;
    gpio->PUPDR = pupdBfr;
    gpio->OSPEEDR = ospeedBfr;
    gpio->AFR[0] = afrlBfr;
    gpio->AFR[1] = afrhBfr;
}

void fmcSendCommand(FmcSdramCommandType * cmd)
{
    FMC_Bank5_6_R->SDCMR = (cmd->commandMode << FMC_SDCMR_MODE_Pos) 
    | (1 << FMC_SDCMR_CTB1_Pos)
    | ((cmd->autoRefreshNumber-1) << FMC_SDCMR_NRFS_Pos)
    | (cmd->modeRegisterDefinition << FMC_SDCMR_MRD_Pos);
}

void initFmcSdram()
{
    FmcSdramCommandType sdramCmd;
    RCC->AHB4ENR |= (1 << RCC_AHB4ENR_GPIOEEN_Pos) | 
                    (1 << RCC_AHB4ENR_GPIOGEN_Pos) |
                    (1 << RCC_AHB4ENR_GPIODEN_Pos) |
                    (1 << RCC_AHB4ENR_GPIOIEN_Pos) |
                    (1 << RCC_AHB4ENR_GPIOHEN_Pos) |
                    (1 << RCC_AHB4ENR_GPIOFEN_Pos) |
                    (1 << RCC_AHB4ENR_GPIOCEN_Pos);
    RCC->AHB3ENR |= (1 << RCC_AHB3ENR_FMCEN_Pos);

    const uint8_t gpioEPins[] = {1,0,13,8,9,11,14,7,10,12,15};
    setFmcGpio(gpioEPins,sizeof(gpioEPins),'E'-'A');

    const uint8_t gpioGPins[] = {15,8,5,4,2,1,0};
    setFmcGpio(gpioGPins,sizeof(gpioGPins),'G'-'A');

    const uint8_t gpioDPins[] = {0,1,15,14,10,9,8};
    setFmcGpio(gpioDPins,sizeof(gpioDPins),'D'-'A');    

    const uint8_t gpioIPins[] = {7,6,5,3,2,9,4,1,10,0};
    setFmcGpio(gpioIPins,sizeof(gpioIPins),'I'-'A');

    const uint8_t gpioHPins[] = {15,13,14,2,3,12,11,10,8,9,5};
    setFmcGpio(gpioHPins,sizeof(gpioHPins),'H'-'A');

    const uint8_t gpioFPins[] = {0,2,1,3,4,5,13,12,15,11,14};
    setFmcGpio(gpioFPins,sizeof(gpioFPins),'F'-'A');

    // initialize fmc peripheral

    FMC_Bank5_6_R->SDCR[0] = (1  << FMC_SDCRx_NC_Pos) | // 9 column bits 
                              (2 << FMC_SDCRx_NR_Pos) | // 13 row bits
                              (2 << FMC_SDCRx_MWID_Pos) | // 32 bit memory bus width
                              (1 << FMC_SDCRx_NB_Pos) |// 4 internal banks
                              (3 << FMC_SDCRx_CAS_Pos) |// 3 cycles cas latency
                              (0 << FMC_SDCRx_WP_Pos) | // no write protection
                              (2 << FMC_SDCRx_SDCLK_Pos) |// 2 clock period
                              (1 << FMC_SDCRx_RBURST_Pos) | // single read are bursts
                              (0 << FMC_SDCRx_RPIPE_Pos); //no read pipe delay
    
    FMC_Bank5_6_R->SDTR[0] =  ((10-1) << FMC_SDTRx_TRCD_Pos) | // row to column delay
                              ((16-1) << FMC_SDTRx_TRCD_Pos) | // row precharge delay
                              ((3-1) << FMC_SDTRx_TWR_Pos) | // write recovery delay
                              ((8-1) << FMC_SDTRx_TRC_Pos) | // row cycle delay
                              ((4-1) << FMC_SDTRx_TRAS_Pos) | // self refresh time
                              ((7-1) << FMC_SDTRx_TXSR_Pos) | // exit self refresh
                              ((2-1) << FMC_SDTRx_TMRD_Pos); // load mode register to active delay     
    // enable the fmc
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

    // configure the actual sdram
    sdramCmd.commandMode = FMC_SDRAM_CMD_CLK_ENABLE; // enable the clock
    sdramCmd.autoRefreshNumber=1;
    sdramCmd.modeRegisterDefinition=0;
    fmcSendCommand(&sdramCmd);

    waitSysticks(10); // wait 100ms

    sdramCmd.commandMode = FMC_SDRAM_CMD_PALL; // preload all
    sdramCmd.autoRefreshNumber=1;
    sdramCmd.modeRegisterDefinition=0;
    fmcSendCommand(&sdramCmd);

    sdramCmd.commandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE; // autorefresh
    sdramCmd.autoRefreshNumber=4;
    sdramCmd.modeRegisterDefinition=0;
    fmcSendCommand(&sdramCmd);

    sdramCmd.commandMode = FMC_SDRAM_CMD_LOAD_MODE;
    sdramCmd.autoRefreshNumber=1;
    sdramCmd.modeRegisterDefinition = SDRAM_MODEREG_BURST_LENGTH_4
             | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_3
             | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
    fmcSendCommand(&sdramCmd);

    // set refresh rate
    FMC_Bank5_6_R->SDRTR = 0x81A - 20;

}