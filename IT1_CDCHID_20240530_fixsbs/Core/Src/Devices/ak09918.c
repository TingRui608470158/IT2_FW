/* ak09918.c
*
*  Created on: 2022/6/28
*      Author: LEAPSY
*/

/* Includes */
#include "ak09918.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "i2c.h"

static uint8_t	read_single_ak09918_reg(uint8_t reg);
static void     write_single_ak09918_reg(uint8_t reg, uint8_t val);
static uint8_t* read_multiple_ak09918_reg(uint8_t reg, uint8_t* data, uint8_t len);

/*
 * Function: ak09918_init
 * --------------------
 * magnetometer's init function
 *
 *  returns: uint8_t value returned
 *
 */
uint8_t ak09918_init()
{
	printf("ak09918_init\r\n");
	if (!ak09918_who_am_i())
		return 0;

	ak09918_soft_reset();	// Soft Reset
	ak09918_operation_mode_setting(AK09918_mode_cont_100hz);	// Continuous Mode 4

	return 1;
}

/*
 * Function: ak09918_mag_read
 * --------------------
 * reads the most current data from AK09918
 *
 *  data: 16-bit data out for each 3-axis magnetic component. raw data.
 *
 *  returns: boolean value returned
 *
 */
bool ak09918_mag_read(axises* data)
{
	uint8_t drdy, hofl, val;	// data ready, overflow

	val = read_single_ak09918_reg(AK09918_REG_ST1);
	drdy = val & 0x01;
	if(!drdy) return false;
	uint8_t temp[6];
	read_multiple_ak09918_reg(AK09918_REG_HXL, temp, sizeof(temp));

	val = read_single_ak09918_reg(AK09918_REG_ST2);
	hofl = val & 0x08;
	if(hofl)	return false;

	// combine into 16 bit values
	data->x = (int16_t)(temp[1] << 8 | temp[0]);
	data->y = (int16_t)(temp[3] << 8 | temp[2]);
	data->z = (int16_t)(temp[5] << 8 | temp[4]);

	return true;
}
// JOSEPH: add raw read to enhance performance @20220513
bool ak09918_mag_read_raw(uint8_t* data)
{
	uint8_t drdy, hofl, val;	// data ready, overflow

	val = read_single_ak09918_reg(AK09918_REG_ST1);
	drdy = val & 0x01;
	if(!drdy) return false;

//	data = read_multiple_ak09918_reg(AK09918_REG_HXL, 6);
//		read_multiple_ak09918_reg(AK09918_REG_HXL, data, sizeof(data));
	read_multiple_ak09918_reg(AK09918_REG_HXL, data, 6);


	val = read_single_ak09918_reg(AK09918_REG_ST2);
	hofl = val & 0x08;
	if(hofl)	return false;

	return true;
}

/*
 * Function: ak09918_mag_read_uT
 * --------------------
 * reads the most current data from AK09918, and
 * convert from scale factor to actual (real) μT
 *
 *  data: 16-bit data out for each 3-axis magnetic component. raw data.
 *
 *  returns: boolean value returned
 *
 */
bool ak09918_mag_read_uT(axises* data)
{
	axises temp;
	bool new_data = ak09918_mag_read(&temp);
	if(!new_data)	return false;

	// Sensitivity: 0.15 μT/LSB (typ.)
	data->x = (float)(temp.x * 0.15);
	data->y = (float)(temp.y * 0.15);
	data->z = (float)(temp.z * 0.15);

	return true;
}

/*
 * Function: ak09918_who_am_i
 * --------------------
 * check the WHO AM I byte, expected value is 0x0C (decimal 12)
 *
 *  returns: boolean value returned
 *
 */
bool ak09918_who_am_i()
{
	printf("ak09918_who_am_i:\r\n");
	uint8_t ak09918c_id = read_single_ak09918_reg(AK09918_REG_WIA2);
	printf("ak09918_who_am_i=%x == %x ?\r\n", ak09918c_id, AK09918C_ID);
	if(ak09918c_id == AK09918C_ID)
		return true;
	else
		return false;
}

/*
 * Function: ak09918_soft_reset
 * --------------------
 * Soft reset the magnetometer
 *
 */
void ak09918_soft_reset()
{
	write_single_ak09918_reg(AK09918_REG_CNTL3, 0x01);
	HAL_Delay(100);
}

/*
 * Function: ak09918_register_check
 * --------------------
 * reads back configuration in case it was set improperly
 *
 *  ub: user bank
 *  reg: register address
 *  set_bits: register bits to be set
 *  clear_bits: register bits to be cleared
 *
 *  returns: boolean value returned
 *
 */
bool ak09918_register_check(uint8_t reg, uint8_t set_bits, uint8_t clear_bits)
{
	bool success = true;
	const uint8_t reg_value = read_single_ak09918_reg(reg);

	if (set_bits && ((reg_value & set_bits) != set_bits)) {
		success = false;
	}

	if (clear_bits && ((reg_value & clear_bits) != 0)) {
		success = false;
	}

	return success;
}

/*
 * Function: ak09918_register_set_and_clear_bits
 * --------------------
 * sets and clears register bits for hardware configuration
 *
 *  ub: user bank
 *  reg: register address
 *  set_bits: register bits to be set
 *  clear_bits: register bits to be cleared
 *
 *  returns: boolean value returned
 *
 */
void ak09918_register_set_and_clear_bits(uint8_t reg, uint8_t set_bits, uint8_t clear_bits)
{
	const uint8_t orig_val = read_single_ak09918_reg(reg);
	uint8_t val = (orig_val & ~clear_bits) | set_bits;

	if (orig_val != val) {
		write_single_ak09918_reg(reg, val);
	}
}

/*
 * Function: ak09918_operation_mode_setting
 * --------------------
 * select a mode from the modes of operation
 *
 *  mode: operation mode setting
 *  		“00000”: power-down mode
 *  		“00001”: single measurement mode
 *  		“00010”: continuous measurement mode 1
 *  		“00100”: continuous measurement mode 2
 *  		“00110”: continuous measurement mode 3
 *  		“01000”: continuous measurement mode 4
 *  		“10000”: self-test mode
 *  		Other code settings are prohibited
 *
 */
void ak09918_operation_mode_setting(AK09918_mode_e mode)
{
	write_single_ak09918_reg(AK09918_REG_CNTL2, mode);
	HAL_Delay(100);
}

/*
 * Function: read_multiple_ak09918_reg
 * --------------------
 * reads registers from the magnetometer given a starting register address, number of bytes,
 * and returns a pointer to store data
 *
 *  reg: register address
 *  len: the number of bytes of data
 *
 *  returns: a pointer to store data returned
 *
 */
static uint8_t* read_multiple_ak09918_reg(uint8_t reg, uint8_t* data, uint8_t len)
{
	I2CReadBytes(&AK09918C_I2C1, MAG_SLAVE_ADDR, reg, data, len);

	return data;
}

/*
 * Function: read_single_ak09918_reg
 * --------------------
 * reads a byte from the magnetometer register given a register address and returns data
 *
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
static uint8_t read_single_ak09918_reg(uint8_t reg)
{
	return I2CReadByte(&AK09918C_I2C1, MAG_SLAVE_ADDR, reg);
}

/*
 * Function: write_single_ak09918_reg
 * --------------------
 * writes a byte to the magnetometer register given a register address and data
 *
 *  reg: register address
 *  val: value of the byte to be written
 *
 */
static void write_single_ak09918_reg(uint8_t reg, uint8_t val)
{
	return I2CWriteByte(&AK09918C_I2C1, MAG_SLAVE_ADDR, reg, val);
}
