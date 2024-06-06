#include "i2c.h"
#include "main.h"
#include <stdio.h>



/*
 * Function: ReadBytes
 * --------------------
 * reads a byte from the magnetometer register given a register address and returns data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
HAL_StatusTypeDef I2CReadWords(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize)
{
//	uint16_t timeout = 0;
//	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, DevAddress, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)ReadBuffer, BufferSize, I2C_TIMEOUT);
//	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
//  return status;
	HAL_StatusTypeDef status = HAL_OK;
	uint16_t timeout = 0;
	if(HAL_I2C_Master_Transmit(hi2c, (DevAddress << 1) | I2C_WRITE , (uint8_t*)&reg, 2, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , Buffer, BufferSize, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	return status;
}

/*
 * Function: ReadBytes
 * --------------------
 * reads a byte from the magnetometer register given a register address and returns data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
HAL_StatusTypeDef I2CReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t* Buffer, uint8_t BufferSize)
{
//	uint16_t timeout = 0;
//	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, DevAddress, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)ReadBuffer, BufferSize, I2C_TIMEOUT);
//	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
//  return status;
#if 0
	HAL_StatusTypeDef status = HAL_OK;
	uint16_t timeout = 0;
//	uint8_t read_data[64] = {0};
	if(HAL_I2C_Master_Transmit(hi2c, (DevAddress << 1) | I2C_WRITE , (uint8_t*)&reg, 1, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , Buffer, BufferSize, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
#else
	uint16_t timeout = 0;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, (DevAddress << 1)&0xFE, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)Buffer, BufferSize, I2C_TIMEOUT);
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
//	HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , Buffer, 0, I2C_TIMEOUT);
#endif
	return status;
}

/*
 * Function: WriteOneByte
 * --------------------
 * writes a byte to the magnetometer register given a register address and data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *  val: value of the byte to be written
 *
 */
void I2CWriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t* Buffer, uint8_t BufferSize)
{
	uint16_t timeout = 0;

	if(HAL_I2C_Mem_Write(hi2c, (DevAddress << 1)&0xFE, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)Buffer, BufferSize, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
}

/*
 * Function: ReadBytes
 * --------------------
 * reads a byte from the magnetometer register given a register address and returns data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
HAL_StatusTypeDef I2CReadBytes16(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize)
{
//	uint16_t timeout = 0;
//	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, DevAddress, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)ReadBuffer, BufferSize, I2C_TIMEOUT);
//	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
//  return status;
#if 0
	HAL_StatusTypeDef status = HAL_OK;
	uint16_t timeout = 0;
//	uint8_t read_data[64] = {0};
	if(HAL_I2C_Master_Transmit(hi2c, (DevAddress << 1) | I2C_WRITE , (uint8_t*)&reg, 1, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , Buffer, BufferSize, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
#else
	uint16_t timeout = 0;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, (DevAddress << 1)&0xFE, reg, I2C_MEMADD_SIZE_16BIT, (uint8_t *)Buffer, BufferSize, I2C_TIMEOUT);
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
//	HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , Buffer, 0, I2C_TIMEOUT);
#endif
	return status;
}

/*
 * Function: WriteOneByte
 * --------------------
 * writes a byte to the magnetometer register given a register address and data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *  val: value of the byte to be written
 *
 */
void I2CWriteBytes16(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t reg, uint8_t* Buffer, uint8_t BufferSize)
{
	uint16_t timeout = 0;

	if(HAL_I2C_Mem_Write(hi2c, (DevAddress << 1)&0xFE, reg, I2C_MEMADD_SIZE_16BIT, (uint8_t *)Buffer, BufferSize, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
}

/*
 * Function: ReadOneByte
 * --------------------
 * reads a byte from the magnetometer register given a register address and returns data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
uint8_t I2CReadByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg)
{
	uint16_t timeout = 0;
	uint8_t read_data = 0;
	if(HAL_I2C_Master_Transmit(hi2c, (DevAddress << 1) | I2C_WRITE , (uint8_t*)&reg, 1, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(hi2c, (DevAddress << 1) | I2C_READ , (uint8_t*)&read_data, 1, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	return read_data;

}

/*
 * Function: WriteOneByte
 * --------------------
 * writes a byte to the magnetometer register given a register address and data
 *
 *	hi2c: i2c channel
 *	DevAddress: device slave address
 *  reg: register address
 *  val: value of the byte to be written
 *
 */
void I2CWriteByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg, uint8_t val)
{
	uint16_t timeout = 0;
	uint16_t write_data = reg | (val<<8);
	if(HAL_I2C_Master_Transmit(hi2c, (DevAddress << 1) | I2C_WRITE , (uint8_t*)&write_data, 2, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
}


