#ifndef _I2C_H_
#define _I2C_H_
#include "stdint.h"


#define I2C_ERROR_ARBITRATION_LOST 1
#define I2C_ERROR_SLAVE_ADDRESS_NACK 2
#define I2C_ERROR_DATA_NACK 3
#define I2C_GENERIC_TX_ERROR 4



void setTargetAddress(uint8_t address);
uint8_t getTargetAddress();
void startMasterReceive(uint8_t lastCmd);

void I2CSetMasterMode();

void I2CSetSlaveMode();


#include "stdint.h"


#define I2C_AWAIT_SLAVE_TRANSACTION 1
#define I2C_ERROR_MASTER_NACK 0
#define I2C_ERROR_SLAVE_ADDRESS_NACK 2
#define I2C_ERROR_DATA_NACK 3
#define I2C_BUFFER_LENGTH 128
#define I2C_BOARD_ADDRESS 27
void initI2c(uint8_t);

uint16_t I2CsendMultiple(uint8_t * data, uint16_t nSend,uint8_t slave_address);

uint16_t I2CReceiveMultiple(uint8_t* dataBfr,uint16_t nBytes,uint8_t slaveAddress);


typedef struct {
    uint8_t * data;
    uint8_t senderAddress;
    uint16_t dataSize;
} I2CReceivedDataType;
volatile I2CReceivedDataType* I2CGetReceivedData(void);

#endif