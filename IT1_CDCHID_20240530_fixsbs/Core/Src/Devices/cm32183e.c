
/* Includes */
#include "main.h"
#include "cm32183e.h"
#include "i2c.h"
#include <stdio.h>

extern struct InterruptData Sensor_Interrupt;
extern double prev_lux;
extern void set_brightness(double lux);
uint32_t threshold_high, threshold_low;
bool write_ambient_light_sensor_complete_flag;
#if 0
static uint16_t cm32183e_cmd[] = { CM32183E_ALS_CONF_DEFAULT | CM32183E_ALS_CONF_PERS_1 | CM32183E_ALS_CONF_IT_200MS |
		CM32183E_ALS_CONF_SM_x2, 315, 285, 0
};
#else
// JOSEPH: change sensitivity from x2 to x1
//static uint16_t cm32183e_cmd[] = { CM32183E_ALS_CONF_DEFAULT | CM32183E_ALS_CONF_PERS_1 | CM32183E_ALS_CONF_IT_200MS | CM32183E_ALS_CONF_GAIN_x1 | CM32183E_ALS_CONF_SM_x1_8,
//		315, 285, 0
//};

static uint16_t cm32183e_cmd[] = { CM32183E_ALS_CONF_DEFAULT | CM32183E_ALS_CONF_PERS_1 | CM32183E_ALS_CONF_IT_100MS | CM32183E_ALS_CONF_GAIN_x1 | CM32183E_ALS_CONF_SM_x1_8,
		315, 285, 0
};
#endif

static const float change_sensitivity = 5; // in percent
#if 1
//static const float calibration_factor = 0.286;
static const float calibration_factor = 0.143;
//static const float calibration_factor = 0.012;
#else
static const float calibration_factor = 0.09;
#endif

uint16_t als_level[]  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535};			// als_code
uint16_t als_value[]  = {0, 200, 320, 502,  1004, 2005, 3058, 5005,  8008,  10010, 12000, 16000, 20000, 20000, 20000, 20000};	// lux
/* Main Functions */
/*
 * Function: GetAmbientLightSensorCompleteFlag
 * --------------------
 * gets the flag, i.e. write_ambient_light_sensor_complete_flag
 *
 *  returns: boolean value returned
 *
 */
bool GetAmbientLightSensorCompleteFlag(void)
{
	return write_ambient_light_sensor_complete_flag;
}

/*
 * Function: SetAmbientLightSensorCompleteFlag
 * --------------------
 * sets the flag, i.e. write_ambient_light_sensor_complete_flag
 *
 *  Setting: boolean value
 *
 */
void SetAmbientLightSensorCompleteFlag(bool Setting)
{
	write_ambient_light_sensor_complete_flag = Setting;
}

/*
 * Function: initialize_CM32183E
 * --------------------
 * The function is called to enable or disable the sensor, set interrupt threshold
 * to the default values, and enable interrupt.
 *
 *  als_code: the ambient light intensity (ALS output)
 *
 */
void initialize_CM32183E(uint16_t als_code)
{
	DPRINTF("iT1 initialize_CM32183E");
//	SetAmbientLightSensorCompleteFlag(false);
	disable_sensor();
	set_als_int_threshold(als_code);
	enable_interrupt();
	enable_sensor();
//	SetAmbientLightSensorCompleteFlag(true);

	// Delay some time after sensor is configured
	HAL_Delay(250U);
}

/*
 * Function: read_CM32183E
 * --------------------
 * The function is called to enable or disable the sensor,
 * convert the raw data output from the ALS to Lux reading,
 * set interrupt threshold to the default values, and
 * enable interrupt.
 *
 *  returns: current lux convert from als data
 */
float read_CM32183E(void)
{
	uint16_t cur_als_code = 0;
//	SetAmbientLightSensorCompleteFlag(false);
	clear_interrupt();
	disable_interrupt();
	cur_als_code = read_als_data();

	//  if (cur_als_code<0xffff)
	als_code = cur_als_code;
	float lux = 0.0f;
#if 1
	lux = als_code * calibration_factor;
#else
	for (i=sizeof(als_level);i>0;i--) {
		if (als_code > als_level[i]) {
			lux = als_value[i] + (als_value[i+1]-als_value[i])*((als_level[i+1] - als_code)/(als_level[i+1]-als_level[i]));
			break;
		}
	}
#endif
//	printf("read_CM32183E als_code=%d, cur_als_code=%d, lux=%6.2f\r\n", als_code, cur_als_code, lux);

	//  if (prev_lux==0) prev_lux = lux;
//	set_brightness(lux);
	set_als_int_threshold(als_code);
	enable_interrupt();
//	SetAmbientLightSensorCompleteFlag(true);
	return lux;
}

/*
 * Function: complete_CM32183E
 * --------------------
 * The function is called to clear and disable interrupt.
 *
 */
void complete_CM32183E(void)
{
//	SetAmbientLightSensorCompleteFlag(false);
	clear_interrupt();
	disable_interrupt();
//	SetAmbientLightSensorCompleteFlag(true);
}

/*
 * Function: reset_CM32183E
 * --------------------
 * The function is called to clear, disable and enable interrupt.
 *
 */
void reset_CM32183E(void)
{
//	SetAmbientLightSensorCompleteFlag(false);
	clear_interrupt();
	disable_interrupt();
	enable_interrupt();
//	SetAmbientLightSensorCompleteFlag(true);
}

/*
 * Function: clear_interrupt
 * --------------------
 * reads ALS_STATUS register to clear interrupt
 *
 */
void clear_interrupt(void)
{
	uint16_t value = CM32183E_read_word(ALS_STATUS);
//	Sensor_Interrupt.ALS_IF_L = value & 0xFF;
//	Sensor_Interrupt.ALS_IF_H = (value & 0xFF00) >> 8;
}

/*
 * Function: enable_sensor
 * --------------------
 * enable ALS
 *
 */
void enable_sensor(void)
{

	cm32183e_cmd[ALS_CONF] &= ~CM32183E_ALS_CONF_SD;
//	DPRINTF("\r\n enable_sensor ALS_CONF=%x cm32183e_cmd[ALS_CONF]=%x", ALS_CONF, cm32183e_cmd[ALS_CONF]);
	CM32183E_write_word(ALS_CONF, cm32183e_cmd[ALS_CONF]);
}

/*
 * Function: disable_sensor
 * --------------------
 * disable ALS
 *
 */
void disable_sensor(void)
{

	cm32183e_cmd[ALS_CONF] |= CM32183E_ALS_CONF_SD;
//	DPRINTF("\r\n disable_sensor ALS_CONF=%x cm32183e_cmd[ALS_CONF]=%x", ALS_CONF, cm32183e_cmd[ALS_CONF]);
	CM32183E_write_word(ALS_CONF, cm32183e_cmd[ALS_CONF]);
}

/*
 * Function: enable_interrupt
 * --------------------
 * enable ALS interrupt
 *
 */
void enable_interrupt(void)
{
#if 0
	cm32183e_cmd[ALS_CONF] |= CM32183E_ALS_CONF_INT_EN;
	CM32183E_write_word(ALS_CONF, cm32183e_cmd[ALS_CONF]);
#endif
}

/*
 * Function: disable_interrupt
 * --------------------
 * disable ALS interrupt
 *
 */
void disable_interrupt(void)
{
#if 0
	cm32183e_cmd[ALS_CONF] &= ~CM32183E_ALS_CONF_INT_EN;
	CM32183E_write_word(ALS_CONF, cm32183e_cmd[ALS_CONF]);
#endif
}

/*
 * Function: set_als_int_threshold
 * --------------------
 * sets ALS high and low threshold
 *
 *  als_code: contains the raw data output from the ALS
 *
 *  returns: threshold_high ALS high threshold window settings
 */
void set_als_int_threshold(uint16_t als_code)
{
	//  uint32_t threshold_high, threshold_low;

	// Set ALS high threshold
	threshold_high = als_code * (100 + change_sensitivity) / 100;
	if (threshold_high > 65535)
		cm32183e_cmd[ALS_THDH] = 65535;
	else
		cm32183e_cmd[ALS_THDH] = threshold_high;

	CM32183E_write_word(ALS_THDH, cm32183e_cmd[ALS_THDH]);

	// Set ALS low threshold
	threshold_low = als_code * (100 - change_sensitivity) / 100;
	cm32183e_cmd[ALS_THDL]  = threshold_low;
	CM32183E_write_word(ALS_THDL, cm32183e_cmd[ALS_THDL]);

//	printf("set_als_int_threshold() als_code=%d, threshold_high=%lu, threshold_low=%lu\r\n", als_code, threshold_high, threshold_low);
}

/*
 * Function: read_als_data
 * --------------------
 * reads ALS data
 *
 *  returns: the raw data output from the ALS
 *
 */
uint16_t read_als_data(void)
{
	uint16_t value = CM32183E_read_word(ALS_DATA);

	return value;
}

/*
 * Function: CM32183E_read_word
 * --------------------
 * writes one 16-bit data to the command register
 *  command: the command register address
 *  val: the output data
 *
 *
 */
uint16_t CM32183E_read_word(uint8_t command)
{
	uint8_t data[2] = {0};
	uint16_t val;

	uint16_t timeout = 0;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&CM32183E_I2C1, (CM32183E_ADDR_7bit_ALS << 1)&0xFE, command, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, sizeof(data), I2C_TIMEOUT);
	while (HAL_I2C_GetState(&CM32183E_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
	val = ((uint16_t)data[1] << 8) | (uint16_t)data[0];
	return val;
}

/*
 * Function: CM32183E_read_word
 * --------------------
 * reads one 16-bit data from the command register
 *  command: the command register address
 *  val: the input data
 *
 */
void CM32183E_write_word(uint8_t command, uint16_t val)
{
	uint8_t data[3];
	uint16_t size = 3;
	uint16_t timeout = 0;

	if(HAL_I2C_Mem_Write(&CM32183E_I2C1, (CM32183E_ADDR_7bit_ALS << 1)&0xFE, command, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&val, sizeof(val), I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&CM32183E_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
}
