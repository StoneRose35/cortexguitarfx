#ifndef _I2C_H_
#define _I2C_H_
#include "stdint.h"


#define I2C_ERROR_ARBITRATION_LOST 1
#define I2C_ERROR_SLAVE_ADDRESS_NACK 2
#define I2C_ERROR_DATA_NACK 3
void initI2c(uint8_t slaveAdress);


uint8_t masterTransmit(uint8_t data,uint8_t lastCmd);
uint8_t masterReceive(uint8_t lastCmd);


#endif