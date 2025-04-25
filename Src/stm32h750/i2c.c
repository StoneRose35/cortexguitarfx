#include "drivers/i2c.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "system.h"
#include "uart.h"
#include "stdio.h"

static volatile uint8_t slave_address_internal, slave_address_external;
static volatile uint8_t firstCommand;
extern volatile uint32_t task;
static uint8_t receiveBuffer[I2C_BUFFER_LENGTH];
static volatile uint16_t nReceived=0;
static volatile uint16_t nSend=0;
static volatile uint8_t bmI2CStatus=0; // bit 0: NACK occurred, bit 1: await slve transaction
volatile I2CReceivedDataType i2cReceivedData = 
{
   .data = receiveBuffer,
   .dataSize = 0,
   .senderAddress = 0  
};

void I2C1_ER_IRQHandler(void)
{
    if ((I2C1->ISR & (1 << I2C_ISR_ARLO_Pos))!= 0)
    {
        bmI2CStatus |= 1 << I2C_AWAIT_SLAVE_TRANSACTION;
        I2C1->OAR1 &= ~(1 << I2C_OAR1_OA1EN_Pos);
        I2C1->OAR1 = (I2C_BOARD_ADDRESS << 1);
        I2C1->OAR1 |= 1 << I2C_OAR1_OA1EN_Pos;
        I2C1->CR1 |= (1 << I2C_CR1_ADDRIE_Pos);
        I2C1->ICR = (1 << I2C_ICR_ARLOCF_Pos);
        #ifdef I2C_DBG
        sendStringBlocking("I2C ERR ARLO\r\n");
        #endif
    }
    else
    {
        #ifdef I2C_DBG
        sendStringBlocking("I2C ERR \r\n");
        #endif
    }
}
void I2C1_EV_IRQHandler(void)
{
    if ((I2C1->ISR & (1 << I2C_ISR_NACKF_Pos))!= 0)
    {
        bmI2CStatus |= (1 << I2C_ERROR_MASTER_NACK);
        I2C1->ICR = (1 << I2C_ICR_NACKCF_Pos);
        #ifdef I2C_DBG
        sendStringBlocking("I2C ERR NACK\r\n");
        #endif
    }

    if ((I2C1->ISR & (1 << I2C_ISR_ADDR_Pos))!= 0)
    {
        // currently only slave receiver is implemented, thus, enable rxne interrupt
        I2C1->ISR |= (1 << I2C_CR1_RXIE_Pos);
        if ((task & (1 << TASK_I2C_DATA_RECEIVED))==0)
        {
            i2cReceivedData.senderAddress = (I2C1->ISR & I2C_ISR_ADDCODE_Msk) >> I2C_ISR_ADDCODE_Pos;
            nReceived = 0;
        }
        I2C1->ICR = (1 << I2C_ICR_ADDRCF_Pos);

        #ifdef I2C_DBG
        sendStringBlocking("I2C ADDR\r\n");
        #endif
    }

    if ((I2C1->ISR & (1 << I2C_ISR_RXNE_Pos))!= 0)
    {
        uint8_t data = (uint8_t)I2C1->RXDR;
        if ((task & (1 << TASK_I2C_DATA_RECEIVED))==0)
        {
            receiveBuffer[nReceived++] = data;
        }
        #ifdef I2C_DBG
        sendStringBlocking("I2C RXNE\r\n");
        #endif
    }

    if ((I2C1->ISR & (1 << I2C_ISR_STOPF_Pos)))
    {
        bmI2CStatus &= ~(1 << I2C_AWAIT_SLAVE_TRANSACTION);
        I2C1->ICR = (1 << I2C_ICR_STOPCF_Pos);
        if ((task & (1 << TASK_I2C_DATA_RECEIVED))==0)
        {
            i2cReceivedData.dataSize = nReceived;
            task |= (1 << TASK_I2C_DATA_RECEIVED);
        }
        #ifdef I2C_DBG
        char charBfr[32];
        sprintf(charBfr,"I2C STOPF, data size %d\r\n",nReceived);
        sendStringBlocking(charBfr);
        #endif
    }

}

// I2C2
static void config_i2c_pin(uint8_t pinnr)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (2 << ((pinnr & 0xF)<<1));
    gpio->MODER=regbfr;
    regbfr = gpio->OTYPER;
    regbfr |= 1 << (pinnr & 0xF);
    gpio->OTYPER = regbfr;
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    gpio->PUPDR = regbfr;
    regbfr = gpio->AFR[(pinnr & 0xF)>>3];
    regbfr &= ~(0xF << ((pinnr & 0x7) << 2));
    regbfr |= 4 << ((pinnr & 0x7) << 2);
    gpio->AFR[(pinnr & 0xF)>>3] = regbfr; 
}

void dummy_read(volatile uint32_t reg)
{}

void initI2c(uint8_t slaveAddressInt,uint8_t slaveAddressExt)
{

    RCC->APB1LENR |= (1 << I2C_ENR_INTERNAL) | (1 << I2C_ENR_EXTERNAL);
    

    // taken from the libDaisy code
    I2C_BLOCK_INTERNAL->TIMINGR = (0x6 << I2C_TIMINGR_PRESC_Pos) 
                    | (0x9 << I2C_TIMINGR_SCLDEL_Pos)
                    | (0 << I2C_TIMINGR_SDADEL_Pos)
                    | (0x43 << I2C_TIMINGR_SCLH_Pos)
                    | (0x5F << I2C_TIMINGR_SCLL_Pos);
    I2C_BLOCK_EXTERNAL->TIMINGR = (0x6 << I2C_TIMINGR_PRESC_Pos) 
                    | (0x9 << I2C_TIMINGR_SCLDEL_Pos)
                    | (0 << I2C_TIMINGR_SDADEL_Pos)
                    | (0x43 << I2C_TIMINGR_SCLH_Pos)
                    | (0x5F << I2C_TIMINGR_SCLL_Pos);
    config_i2c_pin(I2C_SCL_INTERNAL);
    config_i2c_pin(I2C_SDA_INTERNAL);

    config_i2c_pin(I2C_SCL_EXTERNAL);
    config_i2c_pin(I2C_SDA_EXTERNAL);

    I2C_BLOCK_INTERNAL->CR1 |= (1 << I2C_CR1_PE_Pos);
    I2C_BLOCK_EXTERNAL->CR1 |= (1 << I2C_CR1_NACKIE_Pos) | (1 << I2C_CR1_ERRIE_Pos) | (1 << I2C_CR1_RXIE_Pos) | (1 << I2C_CR1_STOPIE_Pos) | (1 << I2C_CR1_ADDRIE_Pos);
    I2C_BLOCK_EXTERNAL->CR1 |= (1 << I2C_CR1_PE_Pos);
    I2C_BLOCK_EXTERNAL->OAR1 = (I2C_BOARD_ADDRESS << 1);
    I2C_BLOCK_EXTERNAL->OAR1 |= (1 << I2C_OAR1_OA1EN_Pos);
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_EnableIRQ(I2C1_ER_IRQn);
    slave_address_internal = slaveAddressInt;
    slave_address_external = slaveAddressExt;
    firstCommand = 1;
}

uint16_t I2CsendMultipleInternal(uint8_t * data, uint16_t nSend)
{
    return I2CsendMultiple(I2C_BLOCK_INTERNAL,data,nSend,slave_address_internal);
}

uint16_t I2CsendMultipleExternal(uint8_t * data, uint16_t nSend)
{
    return I2CsendMultiple(I2C_BLOCK_EXTERNAL,data,nSend,slave_address_external);
}

uint16_t I2CsendMultiple(I2C_TypeDef * i2cBlk,uint8_t * data, uint16_t nSend,uint8_t slave_address)
{
    uint32_t regbfr;
    uint16_t sCnt=0;
    while ((i2cBlk->ISR & (1 << I2C_ISR_TXE_Pos))==0);
    i2cBlk->CR1 &= ~((1 << I2C_CR1_RXIE_Pos) | (1 << I2C_CR1_STOPIE_Pos) | (1 << I2C_CR1_ADDRIE_Pos)); 
    i2cBlk->OAR1 &= ~(1 << I2C_OAR1_OA1EN_Pos);
    regbfr = i2cBlk->CR2;
    regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos) | (0xFF << I2C_CR2_NBYTES_Pos));
    regbfr |= (slave_address << (I2C_CR2_SADD_Pos+1)) | (1 << I2C_CR2_START_Pos) | (nSend << I2C_CR2_NBYTES_Pos) | (1 << I2C_CR2_AUTOEND_Pos);
    i2cBlk->CR2 = regbfr;

    while (sCnt < nSend && bmI2CStatus == 0)
    {
        i2cBlk->TXDR = data[sCnt];
        sCnt++;
        while ((i2cBlk->ISR & I2C_ISR_TXE)==0 && bmI2CStatus == 0);
    }
    nSend=0;
    while ((bmI2CStatus & (1 << I2C_AWAIT_SLAVE_TRANSACTION))!= 0); // block if a slave transaction is awaited
    i2cBlk->ICR = (1 << I2C_ICR_STOPCF_Pos);
    i2cBlk->CR1 |= ((1 << I2C_CR1_RXIE_Pos) | (1 << I2C_CR1_STOPIE_Pos)| (1 << I2C_CR1_ADDRIE_Pos)); 
    i2cBlk->OAR1 |= (1 << I2C_OAR1_OA1EN_Pos);
    return sCnt;
}

/*uint8_t masterReceiveInternal(uint8_t lastCmd)
{
    return masterReceive(I2C_BLOCK_INTERNAL,lastCmd,slave_address_internal);
}

uint8_t masterReceiveExternal(uint8_t lastCmd)
{
    return masterReceive(I2C_BLOCK_EXTERNAL,lastCmd,slave_address_external);
}*/

/**
 * receives a single bytes using one i2c transfer
 */
/*
uint8_t masterReceive(I2C_TypeDef * i2cBlk,uint8_t lastCmd,uint8_t slaveAddress)
{
    uint8_t res;
    uint32_t regbfr;
    (void)lastCmd;
    while ((i2cBlk->ISR & (1 << I2C_ISR_BUSY_Pos)) != 0);

    //regbfr = i2cBlk->CR2;
    //regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos));
    regbfr = (slaveAddress << (I2C_CR2_SADD_Pos+1)) | (1 << I2C_CR2_START_Pos) | (0x1 << I2C_CR2_NBYTES_Pos) 
                | (1 << I2C_CR2_RD_WRN_Pos) | (1 << I2C_CR2_AUTOEND_Pos);
    i2cBlk->CR2 = regbfr;

    while ((i2cBlk->ISR & (1UL << I2C_ISR_RXNE_Pos)) == 0 && bmI2CStatus == 0);
    if ((bmI2CStatus & (1 << I2C_ERROR_MASTER_NACK)) == 0)
    {
        res = (uint8_t)i2cBlk->RXDR;
        return res;
    }
    else
    {
        bmI2CStatus &= ~(1 << I2C_ERROR_MASTER_NACK);
        return 0xFF;
    }
}
*/

/*
 receives nBytes as master from slaveAddress, in case of errors less than nBytes are received
*/
uint16_t I2CReceiveMultiple(I2C_TypeDef * i2cBlk,uint8_t* dataBfr,uint16_t nBytes,uint8_t slaveAddress)
{
    uint32_t regbfr;
    uint16_t dataCnt=0;
    uint16_t bytesReceived=0;
    uint16_t packetByteCnt=0;
    uint8_t bytesToReceive;
    
    i2cBlk->CR1 &= ~((1 << I2C_CR1_RXIE_Pos) | (1 << I2C_CR1_STOPIE_Pos) | (1 << I2C_CR1_ADDRIE_Pos)); 
    i2cBlk->CR1 |= (1 << I2C_CR1_ERRIE_Pos);
    i2cBlk->OAR1 &= ~(1 << I2C_OAR1_OA1EN_Pos);
    // flush read fifo
    while((i2cBlk->ISR & (1UL << I2C_ISR_RXNE_Pos))!= 0)
    {
        (void)i2cBlk->RXDR;
    }
    while (bytesReceived < nBytes && bmI2CStatus==0)
    {
        if (nBytes - bytesReceived > 0xff)
        {
            bytesToReceive = 0xFF;
        }
        else
        {
            bytesToReceive = nBytes - bytesReceived;
        }
        regbfr = i2cBlk->CR2;
        //regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos));
        regbfr = (slaveAddress << (I2C_CR2_SADD_Pos+1)) | (1 << I2C_CR2_START_Pos) | (bytesToReceive << I2C_CR2_NBYTES_Pos) 
              | (1 << I2C_CR2_RD_WRN_Pos) | (1 << I2C_CR2_AUTOEND_Pos);
        i2cBlk->CR2 = regbfr;
        packetByteCnt = 0;
        while ((i2cBlk->ISR & (1UL << I2C_ISR_BUSY_Pos))!=0 && packetByteCnt < bytesToReceive && bmI2CStatus==0)
        {
            while ((i2cBlk->ISR & (1UL << I2C_ISR_RXNE_Pos)) == 0 && packetByteCnt < bytesToReceive && bmI2CStatus == 0);
            if (bmI2CStatus == 0)
            {
                *(dataBfr + dataCnt++) = (uint8_t)i2cBlk->RXDR;
                packetByteCnt++;
            }
        }
        bytesReceived += dataCnt;
    }
    while ((bmI2CStatus & (1 << I2C_AWAIT_SLAVE_TRANSACTION))!= 0); // block if a slave transaction is awaited
    i2cBlk->ICR = (1 << I2C_ICR_STOPCF_Pos);
    i2cBlk->CR1 |= ((1 << I2C_CR1_RXIE_Pos) | (1 << I2C_CR1_STOPIE_Pos)| (1 << I2C_CR1_ADDRIE_Pos)); 
    i2cBlk->OAR1 |= (1 << I2C_OAR1_OA1EN_Pos);
    return bytesReceived;
}

uint16_t I2CReceiveMultipleInternal(uint8_t* dataBfr,uint16_t nBytes)
{
    return I2CReceiveMultiple(I2C_BLOCK_INTERNAL,dataBfr,nBytes,slave_address_internal);
}
uint16_t I2CReceiveMultipleExternal(uint8_t* dataBfr,uint16_t nBytes)
{
    return I2CReceiveMultiple(I2C_BLOCK_EXTERNAL,dataBfr,nBytes,slave_address_external);
}

void setTargetAddressInternal(uint8_t address)
{
    slave_address_internal = address;
}

void setTargetAddressExternal(uint8_t address)
{
    slave_address_external = address;
}

uint8_t getTargetAddressInternal()
{
    return slave_address_internal;
}

uint8_t getTargetAddressExternal()
{
    return slave_address_external;
}

volatile I2CReceivedDataType* I2CGetReceivedData(void)
{
    return &i2cReceivedData;
}