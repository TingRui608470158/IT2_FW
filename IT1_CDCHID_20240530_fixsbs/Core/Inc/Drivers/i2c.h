#ifndef INC_I2C_H_
#define INC_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define I2C_TIMEOUT	1000

extern HAL_StatusTypeDef I2CReadWords(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize);
extern uint8_t I2CReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t* Buffer, uint8_t BufferSize);
extern void I2CWriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t* Buffer, uint8_t BufferSize);
extern HAL_StatusTypeDef I2CReadBytes16(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize);
extern void I2CWriteBytes16(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize);
extern uint8_t I2CReadByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg);
extern void I2CWriteByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t val);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_I2C_H_ */
