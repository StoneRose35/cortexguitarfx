#ifndef _I2C_H_
#define _I2C_H_
#include "stdint.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"

#define I2C_AWAIT_SLAVE_TRANSACTION 1
#define I2C_ERROR_MASTER_NACK 0
#define I2C_ERROR_SLAVE_ADDRESS_NACK 2
#define I2C_ERROR_DATA_NACK 3
#define I2C_BUFFER_LENGTH 128
#define I2C_BOARD_ADDRESS 27
void initI2c(uint8_t, uint8_t);

uint16_t I2CsendMultiple(I2C_TypeDef * i2cBlk,uint8_t * data, uint16_t nSend,uint8_t slave_address);
uint16_t I2CsendMultipleInternal(uint8_t * data, uint16_t nSend);
uint16_t I2CsendMultipleExternal(uint8_t * data, uint16_t nSend);

uint16_t I2CReceiveMultiple(I2C_TypeDef * i2cBlk,uint8_t* dataBfr,uint16_t nBytes,uint8_t slaveAddress);
uint16_t I2CReceiveMultipleInternal(uint8_t* dataBfr,uint16_t nBytes);
uint16_t I2CReceiveMultipleExternal(uint8_t* dataBfr,uint16_t nBytes);

uint8_t I2CsendMultipleExternalAsync(uint8_t * data, uint16_t nSend);
uint8_t I2CsendMultipleInternalAsync(uint8_t * data, uint16_t nSend);
uint8_t I2CsendMultipleAsync(I2C_TypeDef * i2cBlk,uint8_t * data, uint16_t nSend,uint8_t slave_address);

void setTargetAddressInternal(uint8_t address);
void setTargetAddressExternal(uint8_t address);
uint8_t getTargetAddressInternal();
uint8_t getTargetAddressExternal();
typedef struct {
    uint8_t * data;
    uint8_t senderAddress;
    uint16_t dataSize;
} I2CReceivedDataType;
volatile I2CReceivedDataType* I2CGetReceivedData(void);

#endif