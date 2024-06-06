/*
 * icm42688p.c
 *
 *  Created on: 2022年3月2日
 *      Author: LEAPSY
 */

/* Includes */
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "minmax.h"
#include "icm42688p.h"
#include "stm32l4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

/* User Configuration */
#define ICM42688_SPI					(&hspi1)

#define POLYNOMIAL	0x1021
#define WIDTH		(8 * sizeof(uint16_t))			// Checksum width
#define MSb			((uint16_t)1 << (WIDTH - 1))

#define ICM42688_CSB_GPIO_Port		GPIOA
#define ICM42688_CSB_Pin					GPIO_PIN_4

static float gyro_scale_factor;
static float accel_scale_factor;
static int16_t gyro_last_sample[3] = {};
//static int16_t accel_last_sample[3] = {};
static float gyro_scale;
static float accel_scale = 1.f;
static float gyro_range = 2000.f * (180.0 / M_PI);
static float accel_range = 16 * CONSTANTS_ONE_G;
static float accel_clip_limit = 16 * CONSTANTS_ONE_G / 1.f;

extern bool imuFound, magFound;
uint8_t icm42688_isInterrupted = 0;

// variables to store sensor data
//static axises gyro_value;
//static axises accel_value;
//static axises mag_value;
volatile axises accel_angle_value;
volatile axises gyro_angle_value;
volatile uint32_t timestamp;

// gyro and accel bias estimation
static volatile float accel_error[3] = {0};
static volatile float gyro_error[3] = {0};
//static float roll, pitch, yaw;

uint32_t timeMs, currentTime, previousTime;
float elapsedTime;

volatile bool write_mem_sensor_complete_flag;


/* Static Functions */
static void     cs_high();
static void     cs_low();

static void     select_user_bank(userbank ub);

static uint8_t  read_single_icm42688_reg(userbank ub, uint8_t reg);
static void     write_single_icm42688_reg(userbank ub, uint8_t reg, uint8_t val);
static uint8_t* read_multiple_icm42688_reg(userbank ub, uint8_t reg, uint8_t* reg_val, uint8_t len);


static float 	constrain(const float x, const float a, const float b);
//static void transfer_imu_time_data(uint8_t* data, int32_t len);

#if 1 // betta added
void icm42688_enable_tap_detection()
{
/*
 Tap Detection configuration parameters
1. TAP_TMAX (Register 0x47h in Bank 4)
2. TAP_TMIN (Register 0x47h in Bank 4)
3. TAP_TAVG (Register 0x47h in Bank 4)
4. TAP_MIN_JERK_THR (Register 0x46h in Bank 4)
5. TAP_MAX_PEAK_TOL (Register 0x46h in Bank 4)
6. TAP_ENABLE (Register 0x56h in Bank 0)
*/
/*
Initialize Sensor in a typical configuration
1. Set accelerometer ODR (Register 0x50h in Bank 0)
ACCEL_ODR = 15 for 500 Hz (ODR of 200Hz or 1kHz may also be used)
2. Set power modes and filter configurations as shown below
▪ For ODR up to 500Hz, set Accel to Low Power mode (Register 0x4Eh in Bank 0)
ACCEL_MODE = 2 and ACCEL_LP_CLK_SEL = 0, (Register 0x4Dh in Bank 0) for low power mode

Set filter settings as follows: ACCEL_DEC2_M2_ORD = 2 (Register 0x53h in Bank 0); ACCEL_UI_FILT_BW = 4
(Register 0x52h in Bank 0)
• For ODR of 1kHz, set Accel to Low Noise mode (Register 0x4Eh in Bank 0) ACCEL_MODE = 1
Set filter settings as follows: ACCEL_UI_FILT_ORD = 2 (Register 0x53h in Bank 0); ACCEL_UI_FILT_BW =
0 (Register 0x52h in Bank 0)
3. Wait 1 millisecond
*/
	uint8_t new_val = read_single_icm42688_reg(ub_0, PWR_MGMT0);
	new_val |= 0x2;
	new_val &= 0x3E;
	write_single_icm42688_reg(ub_0, PWR_MGMT0, new_val); // reg 0x4E

	new_val = read_single_icm42688_reg(ub_0, INTF_CONFIG1); // reg 0x4D
	new_val &= 0x07;
	write_single_icm42688_reg(ub_0, INTF_CONFIG1, new_val);

	new_val = read_single_icm42688_reg(ub_0, ACCEL_CONFIG1); // reg 0x53
	new_val |= (2<<1);
	write_single_icm42688_reg(ub_0, ACCEL_CONFIG1, new_val);


	new_val = read_single_icm42688_reg(ub_0, GYRO_ACCEL_CONFIG0); // reg 0x52
	new_val |= (4<<4);
	write_single_icm42688_reg(ub_0, GYRO_ACCEL_CONFIG0, new_val);
	HAL_Delay(1);


/*
Initialize APEX hardware
1. Set TAP_TMAX to 2 (Register 0x47h in Bank 4) bits[5:6]
2. Set TAP_TMIN to 3 (Register 0x47h in Bank 4) bits[0:2]
3. Set TAP_TAVG to 3 (Register 0x47h in Bank 4) bits[3:4]
4. Set TAP_MIN_JERK_THR to 17 (Register 0x46h in Bank 4) bits[2:7]
5. Set TAP_MAX_PEAK_TOL to 2 (Register 0x46h in Bank 4) bits[0:1]
6. Wait 1 millisecond
7. Enable TAP source for INT1 by setting bit 0 in register INT_SOURCE6 (Register 0x4Dh in Bank 4) to 1. Or if INT2 is
selected for TAP, enable TAP source by setting bit 0 in register INT_SOURCE7 (Register 0x4Eh in Bank 4) to 1.
8. Wait 50 milliseconds
9. Turn on TAP feature by setting TAP_ENABLE(=>bit6) to 1 (Register 0x56h in Bank 0)
*/
	new_val = 3 | (3<<3) | (2<<5);
	write_single_icm42688_reg(ub_4, APEX_CONFIG8, new_val); // reg 0x47

	new_val = 2 | (17<<2);
	write_single_icm42688_reg(ub_4, APEX_CONFIG7, new_val); // reg 0x47
	HAL_Delay(1);

	new_val = read_single_icm42688_reg(ub_4, INT_SOURCE6);
	new_val |= 1;
	write_single_icm42688_reg(ub_4, INT_SOURCE6, new_val);
	HAL_Delay(50);

	new_val = read_single_icm42688_reg(ub_0, APEX_CONFIG0); // reg 0x56
	new_val |= 1<<6;
	write_single_icm42688_reg(ub_0, APEX_CONFIG0, new_val);
}

uint8_t icm42688_get_tap_detection_status()
{
	/*
	Output registers
	1. Read interrupt register (Register 0x38h in Bank 0) for TAP_DET_INT
	2. Read the tap count in TAP_NUM (Register 0x35h in Bank 0)
	3. Read the tap axis in TAP_AXIS (Register 0x35h in Bank 0)
	4. Read the polarity of tap pulse in TAP_DIR (Register 0x35h in Bank 0)
	 */
	uint8_t new_val = read_single_icm42688_reg(ub_0, INT_STATUS3) & 0x01; // reg 0x38
	if (new_val) {
		// TAP_NUM of reg 0x35, 1=> single tap, 2 => double tap, 0 => no tap
		//new_val = (read_single_icm42688_reg(ub_0, APEX_DATA4)>>3 ) & 3; // TAP_NUM
		new_val = read_single_icm42688_reg(ub_0, APEX_DATA4);

	}

	return new_val;

}
void icm42688_enable_tilt_detection2()
{
	uint8_t new_val;
#if 0
/*
 • Initialize Sensor in a typical configuration
1. Set accelerometer ODR (Register 0x50h in Bank 0)
ACCEL_ODR = 9 for 50 Hz or 10 for 25 Hz
2. Set Accel to Low Power mode (Register 0x4Eh in Bank 0) ACCEL_MODE = 2
and (Register 0x4Dh in Bank 0), ACCEL_LP_CLK_SEL = 0, for low power mode
3. Set DMP ODR (Register 0x56h in Bank 0)
DMP_ODR = 0 for 25 Hz, 2 for 50 Hz
4. Wait 1 millisecond
 */
	//////////
	// 2.
	uint8_t new_val = read_single_icm42688_reg(ub_0, PWR_MGMT0);
	new_val |= 0x2;
	new_val &= 0x3E;
	write_single_icm42688_reg(ub_0, PWR_MGMT0, new_val); // reg 0x4E

	new_val = read_single_icm42688_reg(ub_0, INTF_CONFIG1); // reg 0x4D
	new_val &= 0x07;
	write_single_icm42688_reg(ub_0, INTF_CONFIG1, new_val);

	// 3.
	new_val = read_single_icm42688_reg(ub_0, APEX_CONFIG0); // reg 0x56
	new_val |= 2; //  for 50 Hz
	new_val &= 0xFE;

	write_single_icm42688_reg(ub_0, APEX_CONFIG0, new_val);


	HAL_Delay(1);
/*
• Initialize APEX hardware
1. Set DMP_MEM_RESET_EN to 1 (Register 0x4Bh in Bank 0)
2. Wait 1 millisecond
3. Set TILT_WAIT_TIME (Register 0x43h in Bank 4) if default value does not meet needs
4. Wait 1 millisecond
5. Set DMP_INIT_EN(bit6) to 1 (Register 0x4Bh in Bank 0)
6. Enable Tilt Detection, source for INT1 by setting bit 3 in register INT_SOURCE6 (Register 0x4Dh in Bank 4) to 1. Or
if INT2 is selected for Tilt Detection, enable Tilt Detection source by setting bit 3 in register INT_SOURCE7 (Register
0x4Eh in Bank 4) to 1.
7. Wait 50 milliseconds
8. Turn on Tilt Detection feature by setting TILT_ENABLE(bit4) to 1 (Register 0x56h in Bank 0
 */
// 1.
	new_val = read_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET); // reg 0x4B
	new_val |= 32;
	write_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET, new_val);
	HAL_Delay(1);
// 3.
	new_val = read_single_icm42688_reg(ub_4, APEX_CONFIG4) | (1<<6);
	write_single_icm42688_reg(ub_4, APEX_CONFIG4, new_val); // reg 0x43
	HAL_Delay(1);

// 5.
	new_val = read_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET); // reg 0x4B
	new_val |= 64;
	new_val &= ~(1<<5);
	write_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET, new_val);

// 6.
	new_val = read_single_icm42688_reg(ub_4, INT_SOURCE6);
	new_val |= 8;
	write_single_icm42688_reg(ub_4, INT_SOURCE6, new_val);
	HAL_Delay(50);

#endif
// 8.
	new_val = read_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET); // reg 0x4B
	new_val |= 32;
	write_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET, new_val);
	HAL_Delay(1);
/*
	new_val = read_single_icm42688_reg(ub_0, APEX_CONFIG0); // reg 0x56
	new_val |= 16; // TILT_ENABLE
	write_single_icm42688_reg(ub_0, APEX_CONFIG0, new_val);
*/
}
void icm42688_enable_tilt_detection()
{
/*
 • Initialize Sensor in a typical configuration
1. Set accelerometer ODR (Register 0x50h in Bank 0)
ACCEL_ODR = 9 for 50 Hz or 10 for 25 Hz
2. Set Accel to Low Power mode (Register 0x4Eh in Bank 0) ACCEL_MODE = 2
and (Register 0x4Dh in Bank 0), ACCEL_LP_CLK_SEL = 0, for low power mode
3. Set DMP ODR (Register 0x56h in Bank 0)
DMP_ODR = 0 for 25 Hz, 2 for 50 Hz
4. Wait 1 millisecond
 */
	//////////
	// 2.
	uint8_t new_val = read_single_icm42688_reg(ub_0, PWR_MGMT0);
	new_val |= 0x2;
	new_val &= 0x3E;
	write_single_icm42688_reg(ub_0, PWR_MGMT0, new_val); // reg 0x4E

	new_val = read_single_icm42688_reg(ub_0, INTF_CONFIG1); // reg 0x4D
	new_val &= 0x07;
	write_single_icm42688_reg(ub_0, INTF_CONFIG1, new_val);

	// 3.
	new_val = read_single_icm42688_reg(ub_0, APEX_CONFIG0); // reg 0x56
	new_val |= 2; //  for 50 Hz
	new_val &= 0xFE;

	write_single_icm42688_reg(ub_0, APEX_CONFIG0, new_val);


	HAL_Delay(1);
/*
• Initialize APEX hardware
1. Set DMP_MEM_RESET_EN(bit5) to 1 (Register 0x4Bh in Bank 0)
2. Wait 1 millisecond
3. Set TILT_WAIT_TIME (Register 0x43h in Bank 4) if default value does not meet needs
4. Wait 1 millisecond
5. Set DMP_INIT_EN(bit6) to 1 (Register 0x4Bh in Bank 0)
6. Enable Tilt Detection, source for INT1 by setting bit 3 in register INT_SOURCE6 (Register 0x4Dh in Bank 4) to 1. Or
if INT2 is selected for Tilt Detection, enable Tilt Detection source by setting bit 3 in register INT_SOURCE7 (Register
0x4Eh in Bank 4) to 1.
7. Wait 50 milliseconds
8. Turn on Tilt Detection feature by setting TILT_ENABLE(bit4) to 1 (Register 0x56h in Bank 0
 */
// 1.
	new_val = read_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET); // reg 0x4B
	write_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET, 32);
	HAL_Delay(1);
// 3.
	new_val = read_single_icm42688_reg(ub_4, APEX_CONFIG4) | (1<<6);
	write_single_icm42688_reg(ub_4, APEX_CONFIG4,  64); // reg 0x43
	HAL_Delay(1);

// 5.
	new_val = read_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET); // reg 0x4B
	//new_val &= ~(1<<5) & 0xFF;
	write_single_icm42688_reg(ub_0, SIGNAL_PATH_RESET, 64);

// 6.
	new_val = read_single_icm42688_reg(ub_4, INT_SOURCE6);
	write_single_icm42688_reg(ub_4, INT_SOURCE6, 8|new_val);
	HAL_Delay(50);

// 8.
	new_val = read_single_icm42688_reg(ub_0, APEX_CONFIG0); // reg 0x56
 // TILT_ENABLE
	write_single_icm42688_reg(ub_0, APEX_CONFIG0, new_val|16);
}
uint8_t icm42688_get_tilt_detection_status()
{
	/*
	Output registers
	1. Read interrupt register (Register 0x38h in Bank 0) for TILT_DET_INT
	 */
	//return read_single_icm42688_reg(ub_0, INT_STATUS3) & 0x08; // reg 0x38
	return read_single_icm42688_reg(ub_0, INT_STATUS3);
}
#endif // betta added

/*
 * Function: icm42688_init
 * --------------------
 * sensor init function covers the procedures
 * sets clock source, reset device, wakeup and check WHOAMI
 * sets the accel range to 16G and 100Hz as default
 * sets the gyro range to 2000DPS and 100Hz as default
 *
 *  returns: uint8_t value returned, where 0 typically means
 *  		 sensor not found and 1 means found.
 *
 */
uint8_t icm42688_init(void)
{
	printf("icm42688_init\r\n");

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ICM42688_CSB_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(ICM42688_CSB_GPIO_Port, &GPIO_InitStruct);

	// select clock source to gyro
	icm42688_clock_source(1);

	// reset the ICM42688
	icm42688_device_reset();
	while(!icm42688_device_reset_done()) {
//		printf("try icm42688_device_reset_done\r\n");
	}

	icm42688_wakeup();

	// select clock source to gyro
	icm42688_clock_source(1);

	// check the WHO AM I byte, expected value is 0x75 (decimal 117)
	if (!icm42688_who_am_i())
		return 0;

//	SetMEMSensorCompleteFlag(true);

	if (icm42688_configure()) {
		// if configure succeeded then start reading from FIFO

		// Configure Gyro Sensitivity - Full Scale Range (default +/- 2000deg/s)
		icm42688_gyro_full_scale_select(_2000dps);
		icm42688_gyro_odr_select(_100hz);
		// Configure Accelerometer Sensitivity - Full Scale Range (default +/- 16g)
		icm42688_accel_full_scale_select(_16g);
#if 1 // betta changed to support tap detection
		icm42688_accel_odr_select(_500hz);
		icm42688_enable_tap_detection();

/* To support tilt */
/*
		icm42688_accel_odr_select(_50hz);
		icm42688_enable_tilt_detection();
*/
#else
		icm42688_accel_odr_select(_100hz);
#endif
		calculate_IMU_error();
		HAL_Delay(20);
	} else {
		printf("icm42688_configure failed\r\n");
		HAL_Delay(2000);
		return 0;
	}
	return 1;
}



/*
 * Function: icm42688_gyro_read
 * --------------------
 * reads the most current data from ICM42688
 *
 *  data: 16-bit data out for each 3-axis gyroscope component. raw data.
 *
 */
void icm42688_gyro_read(axises* data)
{
	// grab the data from the ICM42688
	uint8_t temp[6];
	read_multiple_icm42688_reg(ub_0, GYRO_DATA_X1, temp, 6);

	// combine into 16 bit values
	data->x = (int16_t)(temp[0] << 8 | temp[1]);	// the gyroscope measurement in the x direction, rad/s
	data->y = (int16_t)(temp[2] << 8 | temp[3]);	// the gyroscope measurement in the y direction, rad/s
	data->z = (int16_t)(temp[4] << 8 | temp[5]);	// the gyroscope measurement in the z direction, rad/s
}

// JOSEPH: add raw read to enhance performance @20220513
void icm42688_gyro_read_raw(uint8_t* data)
{
	// grab the data from the ICM42688
	read_multiple_icm42688_reg(ub_0, GYRO_DATA_X1, data, 6);
}

/*
 * Function: icm42688_accel_read
 * --------------------
 * reads the most current data from ICM42688
 *
 *  data: 16-bit data out for each 3-axis accelerometer component. raw data.
 *
 */
void icm42688_accel_read(axises* data)
{
	// grab the data from the ICM42688
	uint8_t temp[6];
	read_multiple_icm42688_reg(ub_0, ACCEL_DATA_X1, temp, 6);

	// combine into 16 bit values
	data->x = (int16_t)(temp[0] << 8 | temp[1]);
	data->y = (int16_t)(temp[2] << 8 | temp[3]);
	data->z = (int16_t)(temp[4] << 8 | temp[5]) - accel_scale_factor;
	// Substrates scale factor because calibration function offset gravity acceleration.
}

// JOSEPH: add raw read to enhance performance @20220513
void icm42688_accel_read_raw(uint8_t* data)
{
	// grab the data from the ICM42688
	read_multiple_icm42688_reg(ub_0, ACCEL_DATA_X1, data , 6);
}


/*
 * Function: icm42688_gyro_read_dps
 * --------------------
 * reads the most current data from ICM42688, and
 * convert from scale factor to actual (real) dps
 *
 *  data: 16-bit data out for each 3-axis gyroscope component. raw data.
 *
 */
void icm42688_gyro_read_dps(axises* data)
{
	icm42688_gyro_read(data);

	data->x /= gyro_scale_factor;
	data->y /= gyro_scale_factor;
	data->z /= gyro_scale_factor;
}

/*
 * Function: icm42688_accel_read_g
 * --------------------
 * reads the most current data from ICM42688, and
 * convert from scale factor to actual (real) g
 *
 *  data: 16-bit data out for each 3-axis accelerometer component. raw data.
 *
 */
void icm42688_accel_read_g(axises* data)
{
	icm42688_accel_read(data);

	data->x /= accel_scale_factor;
	data->y /= accel_scale_factor;
	data->z /= accel_scale_factor;
}



/* Sub Functions */
/*
 * Function: icm42688_who_am_i
 * --------------------
 * check the WHO AM I byte, expected value is 0x47 (decimal 71)
 *
 *  returns: boolean value returned
 *
 */
bool icm42688_who_am_i(void)
{
	uint8_t icm42688_id = read_single_icm42688_reg(ub_0, WHO_AM_I);
	printf("icm42688_who_am_i=%x == %x ?\r\n", icm42688_id, ICM42688_ID);
//		HAL_Delay(500);
	if(icm42688_id == ICM42688_ID)
		return true;
	else
		return false;
}



/*
 * Function: icm42688_device_reset
 * --------------------
 * reset the ICM42688
 *
 */
void icm42688_device_reset(void)
{
	write_single_icm42688_reg(ub_0, DEVICE_CONFIG, 0x01);
	HAL_Delay(150);
}

/*
 * Function: icm42688_device_reset_done
 * --------------------
 * software reset completes
 *
 *  returns: boolean value returned
 *
 */
bool icm42688_device_reset_done(void)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, INT_STATUS);

	if((new_val & RESET_DONE_INT) == RESET_DONE_INT)
		return true;
	else
		return false;
}

/*
 * Function: icm42688_wakeup
 * --------------------
 * wakes up ICM42688
 *
 */
void icm42688_wakeup(void)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, PWR_MGMT0);
	new_val &= 0x1F;

	write_single_icm42688_reg(ub_0, PWR_MGMT0, new_val);
	HAL_Delay(100);
}

/*
 * Function: icm42688_sleep
 * --------------------
 * ICM42688 goes back to sleep
 *
 */
void icm42688_sleep(void)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, PWR_MGMT0);
	new_val |= 0x00;

	write_single_icm42688_reg(ub_0, PWR_MGMT0, new_val);
	HAL_Delay(100);
}

/*
 * Function: icm42688_clock_source
 * --------------------
 * select clock source to gyroscope
 *
 *  source: clock source
 *
 */
void icm42688_clock_source(uint8_t source)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, INTF_CONFIG1);
	new_val |= source;

	write_single_icm42688_reg(ub_0, INTF_CONFIG1, new_val);
}



/*
 * Function: icm42688_gyro_full_scale_select
 * --------------------
 * reads back the degrees per second rate and
 * uses it to compensate the gyroscope's reading to dps
 *
 *  full_scale: full scale range
 *
 */
void icm42688_gyro_full_scale_select(gyro_full_scale full_scale)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, GYRO_CONFIG0);

	// sets the gyro full scale range to +- supplied value other than default
	switch(full_scale)
	{
	case _2000dps :
		new_val |= 0x00;
		gyro_scale_factor = 16.4;
		break;
	case _1000dps :
		new_val |= 0x20;
		gyro_scale_factor = 32.8;
		break;
	case _500dps :
		new_val |= 0x40;
		gyro_scale_factor = 65.5;
		break;
	case _250dps :
		new_val |= 0x60;
		gyro_scale_factor = 131.0;
		break;
	case _125dps :
		new_val |= 0x80;
		gyro_scale_factor = 262.0;
		break;
	case _62_5dps :
		new_val |= 0xA0;
		gyro_scale_factor = 524.3;
		break;
	case _31_25dps :
		new_val |= 0xC0;
		gyro_scale_factor = 1048.6;
		break;
	case _15_625dps :
		new_val |= 0xE0;
		gyro_scale_factor = 2097.2;
		break;
	}

	write_single_icm42688_reg(ub_0, GYRO_CONFIG0, new_val);
}

/*
 * Function: icm42688_gyro_odr_select
 * --------------------
 * Read back the degrees per second rate and
 * use it to compensate the gyroscope's reading to dps
 *
 *  full_scale: full scale range
 *
 */
void icm42688_gyro_odr_select(odr odr)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, GYRO_CONFIG0);

	switch(odr)
	{
	case _32khz :
		new_val |= 0x01;
		break;
	case _16khz :
		new_val |= 0x02;
		break;
	case _8khz :
		new_val |= 0x03;
		break;
	case _4khz :
		new_val |= 0x04;
		break;
	case _2khz :
		new_val |= 0x05;
		break;
	case _1khz :
		new_val |= 0x06;
		break;
	case _200hz :
		new_val |= 0x07;
		break;
	case _100hz :
		new_val |= 0x08;
		break;
	case _50hz :
		new_val |= 0x09;
		break;
	case _25hz :
		new_val |= 0x0A;
		break;
	case _12_5hz :
		new_val |= 0x0B;
		break;
	case _6_25hz :
	case _3_125hz :
	case _1_5625hz :
	case _400hz :
		break;
	case _500hz :
		new_val |= 0x0F;
		break;
	}

	write_single_icm42688_reg(ub_0, GYRO_CONFIG0, new_val);
}

/*
 * Function: icm42688_accel_full_scale_select
 * --------------------
 * Read accelerometer full scale range and
 * use it to compensate the accelerometer's reading to gs
 *
 *  full_scale: full scale range
 *
 */
void icm42688_accel_full_scale_select(accel_full_scale full_scale)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, ACCEL_CONFIG0);

	// sets the accelerometer full scale range to +- supplied value other than default
	switch(full_scale)
	{
	case _16g :
		new_val |= 0x00;
		accel_scale_factor = 2048;
		break;
	case _8g :
		new_val |= 0x20;
		accel_scale_factor = 4096;
		break;
	case _4g :
		new_val |= 0x40;
		accel_scale_factor = 8192;
		break;
	case _2g :
		new_val |= 0x60;
		accel_scale_factor = 16384;
		break;
	}

	write_single_icm42688_reg(ub_0, ACCEL_CONFIG0, new_val);
}

/*
 * Function: icm42688_accel_odr_select
 * --------------------
 * enables ODR sensitivity
 *
 *  odr: sample rate
 *
 */
void icm42688_accel_odr_select(odr odr)
{
	uint8_t new_val = read_single_icm42688_reg(ub_0, ACCEL_CONFIG0);

	switch(odr)
	{
	case _32khz :
		new_val |= 0x01;
		break;
	case _16khz :
		new_val |= 0x02;
		break;
	case _8khz :
		new_val |= 0x03;
		break;
	case _4khz :
		new_val |= 0x04;
		break;
	case _2khz :
		new_val |= 0x05;
		break;
	case _1khz :
		new_val |= 0x06;
		break;
	case _200hz :
		new_val |= 0x07;
		break;
	case _100hz :
		new_val |= 0x08;
		break;
	case _50hz :
		new_val |= 0x09;
		break;
	case _25hz :
		new_val |= 0x0A;
		break;
	case _12_5hz :
		new_val |= 0x0B;
		break;
	case _6_25hz :
		new_val |= 0x0C;
		break;
	case _3_125hz :
		new_val |= 0x0D;
		break;
	case _1_5625hz :
		new_val |= 0x0E;
		break;
	case _400hz :
		break;
	case _500hz :
		new_val |= 0x0F;
		break;
	}

	write_single_icm42688_reg(ub_0, ACCEL_CONFIG0, new_val);
}

/*
 * Function: icm42688_int_config
 * --------------------
 * setup INT1 and INT2 interrupt mode, drive circuit and
 * interrupt polarity
 *
 *  mode: interrupt mode, drive circuit or interrupt polarity
 *
 */
void icm42688_int_config(uint8_t mode)
{
	write_single_icm42688_reg(ub_0, INT_CONFIG, mode);
}

/*
 * Function: icm42688_int_source_0
 * --------------------
 * setup UI FSYNC interrupt, PLL ready interrupt, Reset done interrupt,
 * UI data ready interrupt, FIFO threshold interrupt, FIFO full interrupt,
 * and UI AGC ready interrupt
 *
 *  mode: mode in UI FSYNC interrupt, PLL ready interrupt, Reset done interrupt,
 *  		UI data ready interrupt, FIFO threshold interrupt, FIFO full interrupt,
 * 			or UI AGC ready interrupt
 *
 */
void icm42688_int_source_0(uint8_t mode)
{
	write_single_icm42688_reg(ub_0, INT_SOURCE0, mode);
	HAL_Delay(60);
}

/*
 * Function: icm42688_accel_update_clip_limit
 * --------------------
 * setup Min/Max limits to acceleromter's self-test response
 *
 */
void icm42688_accel_update_clip_limit(void)
{
	// 99.9% of potential max
	accel_clip_limit = constrain((accel_range / accel_scale) * 0.999f, 0.f, (float)INT16_MAX);
}

/*
 * Function: icm42688_gyro_set_scale
 * --------------------
 * rescales last gyroscope's scale on scale change
 *
 *  scale: scale change
 *
 */
void icm42688_gyro_set_scale(float scale)
{
	uint8_t i;
	uint32_t size = 3;

	if (fabsf(scale - gyro_scale) > FLT_EPSILON) {
		// rescale last sample on scale change
		float rescale = gyro_scale / scale;

		for(i=0 ;i<size; i++) {
			gyro_last_sample[i] = roundf(gyro_last_sample[i] * rescale);
		}

		gyro_scale = scale;
	}
}

/*
 * Function: icm42688_register_check
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
bool icm42688_register_check(userbank ub, uint8_t reg, uint8_t set_bits, uint8_t clear_bits)
{
	bool success = true;

	const uint8_t reg_value = read_single_icm42688_reg(ub, reg);

	if (set_bits && ((reg_value & set_bits) != set_bits)) {
		success = false;
	}

	if (clear_bits && ((reg_value & clear_bits) != 0)) {
		success = false;
	}

	return success;
}

/*
 * Function: icm42688_register_set_and_clear_bits
 * --------------------
 * sets and clears register bits for hardware configuration
 *
 *  ub: user bank
 *  reg: register address
 *  set_bits: register bits to be set
 *  clear_bits: register bits to be cleared
 *
 */
void icm42688_register_set_and_clear_bits(userbank ub, uint8_t reg, uint8_t set_bits, uint8_t clear_bits)
{
	const uint8_t orig_val = read_single_icm42688_reg(ub, reg);
	uint8_t val = (orig_val & ~clear_bits) | set_bits;

	if (orig_val != val) {
		write_single_icm42688_reg(ub, reg, val);
	}
}



/*
 * Function: gyro_set_range
 * --------------------
 * sets the full-scale range of the gyroscope
 *
 *  range: selected range
 *
 */
void gyro_set_range(float range) {
	gyro_range = range;
}

/*
 * Function: accel_set_range
 * --------------------
 * sets the full-scale range of the acceleromter
 *
 *  range: selected range
 *
 */
void accel_set_range(float range) {
	accel_range = range;
	icm42688_accel_update_clip_limit();
}

/*
 * Function: icm42688_configure
 * --------------------
 * gets/sets hardware configuration and reads back configuration
 * in case it was set improperly
 *
 *  returns: boolean value returned
 *
 */
bool icm42688_configure()
{
	uint8_t i;
	// first set and clear all configured register bits
	for(i=0 ;i<size_register_bank0_cfg;i++)
		icm42688_register_set_and_clear_bits(ub_0, icm42688_register_bank0_cfg[i].reg,
				icm42688_register_bank0_cfg[i].set_bits, icm42688_register_bank0_cfg[i].clear_bits);

	for(i=0 ;i<size_register_bank1_cfg;i++)
		icm42688_register_set_and_clear_bits(ub_0, icm42688_register_bank1_cfg[i].reg,
				icm42688_register_bank1_cfg[i].set_bits, icm42688_register_bank1_cfg[i].clear_bits);

	for(i=0 ;i<size_register_bank2_cfg;i++)
		icm42688_register_set_and_clear_bits(ub_0, icm42688_register_bank2_cfg[i].reg,
				icm42688_register_bank2_cfg[i].set_bits, icm42688_register_bank2_cfg[i].clear_bits);

	// now check that all are configured
	bool success = true;

	for(i=0 ;i<size_register_bank0_cfg;i++)
		if (!icm42688_register_check(ub_0, icm42688_register_bank0_cfg[i].reg,
				icm42688_register_bank0_cfg[i].set_bits, icm42688_register_bank0_cfg[i].clear_bits)) {
			success = false;
		}

	for(i=0 ;i<size_register_bank1_cfg;i++)
		if (!icm42688_register_check(ub_0, icm42688_register_bank1_cfg[i].reg,
				icm42688_register_bank1_cfg[i].set_bits, icm42688_register_bank1_cfg[i].clear_bits)) {
			success = false;
		}


	for(i=0 ;i<size_register_bank2_cfg;i++)
		if (!icm42688_register_check(ub_0, icm42688_register_bank2_cfg[i].reg,
				icm42688_register_bank2_cfg[i].set_bits, icm42688_register_bank2_cfg[i].clear_bits)) {
			success = false;
		}

	// 20-bits data format used
	//  the only FSR settings that are operational are ±2000dps for gyroscope and ±16g for accelerometer
	accel_set_range(16.f * CONSTANTS_ONE_G);	// setting the accel scale to 16G
	gyro_set_range(to_degrees(2000.f));			// setting the gyro scale to 2000DPS

	return success;
}

/* enables the data ready interrupt */
void icm42688_enable_data_ready_interrupt(void)
{
	/* setting the interrupt */
	icm42688_int_config(INT1_DRIVE_CIRCUIT | INT1_POLARITY);	// setup interrupt, pulse
	icm42688_int_source_0(UI_DRDY_INT1_EN);					// set to data ready
}

/* enables the reset done interrupt */
void icm42688_enable_reset_done_interrupt(void)
{
	icm42688_int_source_0(RESET_DONE_INT1_EN);				// set to reset done (disable interrupt)
}

/* checks the data ready interrupt */
uint8_t icm42688_is_interrupted(void)
{
	icm42688_isInterrupted = read_single_icm42688_reg(ub_0, INT_STATUS);

	return icm42688_isInterrupted & 0x08;
}



/*
 * Function: calculate_IMU_error
 * --------------------
 * reads the errors from the sensors when not in motion
 *
 */
void calculate_IMU_error(void)
{
	// We can call this function in the setup section to calculate
	// the accelerometer and gyro data error. From here we will get
	// the error values used in the above equations printed on the
	// Serial Monitor.
	// Note that we should place the IMU flat in order to get the
	// proper values, so that we then can the correct values.

	axises temp;
	uint8_t tmp[60];

	// Read accelerometer values 200 times
	for(int i = 0; i < 200; i++)
	{
		icm42688_accel_read_g(&temp);

		// Sum all readings
		accel_error[0] += ((atan((temp.y) / sqrt(pow((temp.x), 2) + pow((temp.z), 2))) * 180 / M_PI));;
		accel_error[1] += ((atan(-1 * (temp.x) / sqrt(pow((temp.y), 2) + pow((temp.z), 2))) * 180 / M_PI));
	}

	//Divide the sum by 200 to get the error value
	accel_error[0] /= 200;
	accel_error[1] /= 200;

	// Read gyro values 200 times
	for(int i = 0; i < 200; i++)
	{
		icm42688_gyro_read_dps(&temp);

		// Sum all readings
		gyro_error[0] += temp.x;
		gyro_error[1] += temp.y;
		gyro_error[2] += temp.z;
	}

	//Divide the sum by 200 to get the error value
	gyro_error[0] /= 200;
	gyro_error[1] /= 200;
	gyro_error[2] /= 200;
	sprintf(tmp,"(%.1f, %.1f), (%.1f, %.1f, %.1f)\n", accel_error[0], accel_error[1], gyro_error[0], gyro_error[1], gyro_error[2]);
	sendCdcData((uint8_t*)tmp, strlen(tmp));
}

/* Static Functions */
/*
 * Function: cs_high
 * --------------------
 * switches CS pin to high
 *
 */
static void cs_high()
{
	HAL_GPIO_WritePin(ICM42688_CSB_GPIO_Port, ICM42688_CSB_Pin, GPIO_PIN_SET);
}

/*
 * Function: cs_low
 * --------------------
 * switches CS pin to low
 *
 */
static void cs_low()
{
	HAL_GPIO_WritePin(ICM42688_CSB_GPIO_Port, ICM42688_CSB_Pin, GPIO_PIN_RESET);
}

/*
 * Function: select_user_bank
 * --------------------
 * switches register bank
 *
 *  ub: user bank (0-4)
 *
 */
static void select_user_bank(userbank ub)
{
	uint8_t write_reg[2];
	write_reg[0] = WRITE | REG_BANK_SEL;
	write_reg[1] = ub;

	cs_low();
	HAL_SPI_Transmit(ICM42688_SPI, write_reg, 2, 10);
	cs_high();
}

/*
 * Function: read_single_icm42688_reg
 * --------------------
 * reads a byte from ICM42688 register given a register address and returns data
 *
 *  ub: user bank
 *  reg: register address
 *
 *  returns: value of the byte returned
 *
 */
static uint8_t read_single_icm42688_reg(userbank ub, uint8_t reg)
{
	uint8_t read_reg = READ | reg;
	uint8_t reg_val;
	select_user_bank(ub);

	cs_low();
	HAL_SPI_Transmit(ICM42688_SPI, &read_reg, 1, 1000);
	HAL_SPI_Receive(ICM42688_SPI, &reg_val, 1, 1000);
	cs_high();

	return reg_val;
}

/*
 * Function: write_single_icm42688_reg
 * --------------------
 * writes a byte to ICM42688 register given a register address and data
 *
 *  ub: user bank
 *  reg: register address
 *  val: value of the byte to be written
 *
 */
static void write_single_icm42688_reg(userbank ub, uint8_t reg, uint8_t val)
{
	uint8_t write_reg[2];
	write_reg[0] = WRITE | reg;
	write_reg[1] = val;

	select_user_bank(ub);

	cs_low();
	HAL_SPI_Transmit(ICM42688_SPI, write_reg, 2, 1000);
	cs_high();
}

/*
 * Function: read_multiple_icm42688_reg
 * --------------------
 * reads registers from ICM42688 given a starting register address, number of bytes,
 * and returns a pointer to store data
 *
 *  ub: user bank
 *  reg: register address
 *  len: the number of bytes
 *
 *  returns: a pointer to store data returned
 *
 */
static uint8_t* read_multiple_icm42688_reg(userbank ub, uint8_t reg, uint8_t* reg_val, uint8_t len)
{
	uint8_t read_reg = READ | reg;
//	static uint8_t reg_val[14];
	select_user_bank(ub);

	cs_low();
	HAL_SPI_Transmit(ICM42688_SPI, &read_reg, 1, 1000);
	HAL_SPI_Receive(ICM42688_SPI, reg_val, len, 1000);
	cs_high();

	return reg_val;
}



/*
 * Function: constrain
 * --------------------
 * constrains a number to be within a range.
 *
 *  x: the number to constrain
 *  a: the lower end of the range
 *  b: the upper end of the range.
 *
 *  returns:
 *  	x: if x is between a and b.
 *  	a: if x is less than a.
 *  	b: if x is greater than b.
 *
 */
static float constrain(const float x, const float a, const float b)
{
	if(x < a) {
		return a;
	}
	else if(b < x) {
		return b;
	}
	else
		return x;
}

