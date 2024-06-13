
/* Includes */
#include "main.h"
#include "vcnl36828p.h"
#include <stdio.h>
#include "stm32l4xx_hal.h"
// JOSEPH: add for light-sensor debug output @20220531
#include "usbd_cdc_if.h"
#include "cm32183e.h"
#include "string.h"
#include "i2c.h"
/* betta added */
#define DEFALUT_HI_THRESHOLD  333 //763
#define DEFALUT_LO_THRESHOLD  320//(DEFALUT_HI_THRESHOLD - 20)

extern USBD_HandleTypeDef hUsbDeviceFS;
extern double prev_lux;
////

extern I2C_HandleTypeDef hi2c1;
extern struct InterruptData Sensor_Interrupt;
volatile uint16_t Interrupt;


//Variables for Offset Value
//uint16_t CalibValue = 100;

//#define DEFALUT_CALIBRATION_VALUE 2388
#define DEFALUT_CALIBRATION_VALUE 18
uint32_t CalibValue = 6552;
uint32_t OffsetValue = 0;
#if 0
uint32_t LowThreshold = 150; // reduce the distance to 5 cm
uint32_t HighThreshold = 500; // reduce the distance to 3 cm
#else
//uint32_t LowThreshold = 700; // reduce the distance to 5 cm
//uint32_t HighThreshold = 2700; // reduce the distance to 3 cm
uint32_t LowThreshold = 146; // reduce the distance to 5 cm
uint32_t HighThreshold = 272; // reduce the distance to 3 cm
#endif
uint16_t AverageCount = 10; //Change the average count to the needed number of offset measurement
int32_t SEL_Offset;
bool is_VCNL36828P_changed = false;
bool write_prox_sensor_complete_flag;
/*
 * Function: GetProxSensorCompleteFlag
 * --------------------
 * gets the flag, i.e. write_prox_sensor_complete_flag
 *
 *  returns: boolean value returned
 *
 */
bool GetProxSensorCompleteFlag(void)
{
	return write_prox_sensor_complete_flag;
}

/*
 * Function: SetProxSensorCompleteFlag
 * --------------------
 * sets the flag, i.e. write_prox_sensor_complete_flag
 *
 *  Setting: boolean value
 *
 */
void SetProxSensorCompleteFlag(bool Setting)
{
	write_prox_sensor_complete_flag = Setting;
}

//****************************************************************************************************
//*****************************************Sensor API*************************************************

/*
 * Function: VCNL36828P_SET_PS_ON
 * --------------------
 * turns the PS Sensor On/Off
 *
 *  ps_on: VCNL36828P_PS_SD_EN or VCNL36828P_PS_SD_DIS
 *
 */
void VCNL36828P_SET_PS_ON(uint16_t ps_on)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_L);
	value = (value&~(VCNL36828P_PS_SD_EN|VCNL36828P_PS_SD_DIS))|ps_on;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_ST
 * --------------------
 * sets the PS_ST bit for initialization
 *
 *  ps_st: VCNL36828P_PS_ST_START or VCNL36828P_PS_ST_STOP
 *
 */
void VCNL36828P_SET_PS_ST(uint16_t ps_st)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_L);
	value = (value&~(uint16_t)VCNL36828P_START)|ps_st;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_HD
 * --------------------
 * sets the PS output high dynamic setting
 *
 *  ps_hd: VCNL36828P_PS_HD_EN or VCNL36828P_PS_HD_DIS
 *
 */
void VCNL36828P_SET_PS_HD(uint16_t ps_hd)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	value = (value&~(VCNL36828P_PS_HD_DIS|VCNL36828P_PS_HD_EN))|ps_hd;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_SP_INT
 * --------------------
 * sets the Sunlight Protect
 *
 *  ps_sp_int: VCNL36828P_PS_SP_INT_EN or VCNL36828P_PS_SP_INT_DIS
 *
 */
void VCNL36828P_SET_PS_SP_INT(uint16_t ps_sp_int)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	value = (value&~(VCNL36828P_PS_SP_INT_DIS|VCNL36828P_PS_SP_INT_EN))|ps_sp_int;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_SMART_PERS
 * --------------------
 * enable/disable Smart Persistence
 *
 *  Pers: VCNL36828P_PS_SMART_PERS_EN or VCNL36828P_PS_SMART_PERS_DIS
 *
 */
void VCNL36828P_SET_PS_SMART_PERS(uint16_t Pers)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	value = (value&~(VCNL36828P_PS_SMART_PERS_DIS|VCNL36828P_PS_SMART_PERS_EN))|Pers;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_PERS
 * --------------------
 * sets the Persistence
 *
 *  ps_pers: VCNL36828P_PS_PERS_1, VCNL36828P_PS_PERS_2,
 *  	VCNL36828P_PS_PERS_3, or VCNL36828P_PS_PERS_4
 *
 */
void VCNL36828P_SET_PS_PERS(uint16_t ps_pers)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	value = (value&~(VCNL36828P_PS_PERS_1|VCNL36828P_PS_PERS_2|VCNL36828P_PS_PERS_3|VCNL36828P_PS_PERS_4))|ps_pers;
	VCNL36828P_write_word(VCNL36828P_PS_CONF1_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_INT
 * --------------------
 * sets the Interrupt Mode
 *
 *  ps_int: VCNL36828P_PS_INT_DIS, VCNL36828P_PS_INT_LOGIC,
 *  	VCNL36828P_PS_INT_FIRST_HIGH, or VCNL36828P_PS_INT_EN
 *
 */
void VCNL36828P_SET_PS_INT(uint16_t ps_int)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	value = (value&~(VCNL36828P_PS_INT_DIS|VCNL36828P_PS_INT_LOGIC|VCNL36828P_PS_INT_FIRST_HIGH|VCNL36828P_PS_INT_EN))|ps_int;

	DPRINTF("VCNL36828P_SET_PS_INT: value=%x\r\n", value);

	VCNL36828P_write_word(VCNL36828P_PS_CONF1_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_PERIOD
 * --------------------
 * sets the PS measurement period
 *
 *  ps_period: VCNL36828P_PS_PERIOD_50ms, VCNL36828P_PS_PERIOD_100ms,
 *  	VCNL36828P_PS_PERIOD_200ms, or VCNL36828P_PS_PERIOD_400ms
 *
 */
void VCNL36828P_SET_PS_PERIOD(uint16_t ps_period)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	value = (value&~(VCNL36828P_PS_PERIOD_50ms|VCNL36828P_PS_PERIOD_100ms|VCNL36828P_PS_PERIOD_200ms|VCNL36828P_PS_PERIOD_400ms))|ps_period;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_IT
 * --------------------
 * sets the PS integration time
 *
 *  ps_it: VCNL36828P_PS_IT_1T, VCNL36828P_PS_IT_2T,
 *  	VCNL36828P_PS_IT_4T, or VCNL36828P_PS_IT_8T
 *
 */
void VCNL36828P_SET_PS_IT(uint16_t ps_it)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	value = (value&~(VCNL36828P_PS_IT_1T|VCNL36828P_PS_IT_2T|VCNL36828P_PS_IT_4T|VCNL36828P_PS_IT_8T))|ps_it;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_MPS
 * --------------------
 * sets the Multi Pulse (MPS) setting
 *
 *  ps_mps: VCNL36828P_PS_MPS_1, VCNL36828P_PS_MPS_2,
 *  	VCNL36828P_PS_MPS_4, or VCNL36828P_PS_MPS_8
 *
 */
void VCNL36828P_SET_PS_MPS(uint16_t ps_mps)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	value = (value&~(VCNL36828P_PS_MPS_1|VCNL36828P_PS_MPS_2|VCNL36828P_PS_MPS_4|VCNL36828P_PS_MPS_8))|ps_mps;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_ITB
 * --------------------
 * sets the Integration Time Bank (Pulse Width)
 *
 *  ps_itb: VCNL36828P_PS_ITB_25 or VCNL36828P_PS_ITB_50
 *
 */
void VCNL36828P_SET_PS_ITB(uint16_t ps_itb)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	value = (value&~(VCNL36828P_PS_ITB_25|VCNL36828P_PS_ITB_50))|ps_itb;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_HG
 * --------------------
 * sets High Gain setting
 *
 *  ps_hg: VCNL36828P_PS_HG_x1 or VCNL36828P_PS_HG_x2
 *
 */
void VCNL36828P_SET_PS_HG(uint16_t ps_hg)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	value = (value&~(VCNL36828P_PS_HG_x1|VCNL36828P_PS_HG_x2))|ps_hg;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_SENS
 * --------------------
 * sets the PS sensitivity setting
 *
 *  ps_sens: VCNL36828P_PS_SENS_NORMAL or VCNL36828P_PS_SENS_HIGH
 *
 */
void VCNL36828P_SET_PS_SENS(uint16_t ps_sens)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);
	value = (value&~(VCNL36828P_PS_SENS_NORMAL|VCNL36828P_PS_SENS_HIGH))|ps_sens;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_OFFSET
 * --------------------
 * sets the PS offset by factory
 *
 *  ps_offset: VCNL36828P_PS_OFFSET_EN or VCNL36828P_PS_OFFSET_DIS
 *
 */
void VCNL36828P_SET_PS_OFFSET(uint16_t ps_offset)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);
	value = (value&~(VCNL36828P_PS_OFFSET_DIS|VCNL36828P_PS_OFFSET_EN))|ps_offset;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_LED
 * --------------------
 * sets the PS VCSEL current driving setting
 *
 *  ps_led: VCNL36828P_PS_LED_8_3mA,
 *  		VCNL36828P_PS_LED_9_7mA,
 *  		VCNL36828P_PS_LED_11_7mA,
 *  		VCNL36828P_PS_LED_13_1mA,
 *  		VCNL36828P_PS_LED_15_7mA,
 *  		VCNL36828P_PS_LED_17_1mA,
 *  		VCNL36828P_PS_LED_19_1mA,
 *  		or VCNL36828P_PS_LED_20_4mA
 *
 */
void VCNL36828P_SET_PS_LED(uint16_t ps_led)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);
	value = (value&~(VCNL36828P_PS_LED_8_3mA|VCNL36828P_PS_LED_9_7mA|
			VCNL36828P_PS_LED_11_7mA|VCNL36828P_PS_LED_13_1mA|VCNL36828P_PS_LED_15_7mA|
			VCNL36828P_PS_LED_17_1mA|VCNL36828P_PS_LED_19_1mA|VCNL36828P_PS_LED_20_4mA))|ps_led;
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_AF
 * --------------------
 * sets the Sensor Mode (Active Force or Auto Mode)
 *
 *  ps_af: VCNL36828P_PS_AF_EN,
 *  		or VCNL36828P_PS_AF_DIS
 *
 */
void VCNL36828P_SET_PS_AF(uint16_t ps_af)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF3_L);
	value = (value&~(VCNL36828P_PS_AF_EN|VCNL36828P_PS_AF_DIS))|ps_af;
	VCNL36828P_write_word(VCNL36828P_PS_CONF3_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_TRIG
 * --------------------
 * enable/disable Trigger for AF Mode
 *
 *  TriggerBit: VCNL36828P_PS_TRIG_EN,
 *  		or VCNL36828P_PS_TRIG_DIS
 *
 */
void VCNL36828P_SET_PS_TRIG(uint16_t TriggerBit)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF3_L);
	value = (value&~(VCNL36828P_PS_TRIG_EN|VCNL36828P_PS_TRIG_DIS))|TriggerBit;
	VCNL36828P_write_word(VCNL36828P_PS_CONF3_L, value);
}

/*
 * Function: VCNL36828P_SET_PS_SP
 * --------------------
 * sets PS short period setting
 *
 *  ps_sp: VCNL36828P_PS_SPERIOD_PERIOD,
 *  		VCNL36828P_PS_SPERIOD_6_25ms,
 *  		VCNL36828P_PS_SPERIOD_12_5ms
 *  		or VCNL36828P_PS_SPERIOD_25ms
 *
 */
void VCNL36828P_SET_PS_SP(uint16_t ps_sp)
{
	uint16_t value = VCNL36828P_read_word(VCNL36828P_PS_CONF3_H);
	value = (value&~(VCNL36828P_PS_SPERIOD_PERIOD|VCNL36828P_PS_SPERIOD_6_25ms|
			VCNL36828P_PS_SPERIOD_12_5ms|VCNL36828P_PS_SPERIOD_25ms))|ps_sp;
	VCNL36828P_write_word(VCNL36828P_PS_CONF3_H, value);
}

/*
 * Function: VCNL36828P_SET_PS_LowThreshold
 * --------------------
 * sets the Low Threshold
 *
 *  LowThreshold: Value between 0d0 and 0d4095
 *
 */
void VCNL36828P_SET_PS_LowThreshold(uint16_t LowThreshold)
{
	VCNL36828P_write_word(VCNL36828P_PS_THDL, LowThreshold);
}

/*
 * Function: VCNL36828P_SET_PS_HighThreshold
 * --------------------
 * sets the High Threshold
 *
 *  HighThreshold: Value between 0d0 and 0d4095
 *
 */
void VCNL36828P_SET_PS_HighThreshold(uint16_t HighThreshold)
{
	VCNL36828P_write_word(VCNL36828P_PS_THDH, HighThreshold);
}

/*
 * Function: VCNL36828P_SET_PS_CANC
 * --------------------
 * sets the PS Cancellation
 *
 *  CancelValue: Value between 0d0 and 0d4095
 *
 */
void VCNL36828P_SET_PS_CANC(uint16_t CancelValue)
{
	VCNL36828P_write_word(VCNL36828P_PS_CANC, CancelValue);
}

/*
 * Function: Reset_Sensor
 * --------------------
 * resets the Sensor to the default value
 *
 *
 */
void Reset_Sensor()
{
	DPRINTF("Reset_Sensor");
	VCNL36828P_write_word(VCNL36828P_PS_CONF2_L, 0x0000);
	VCNL36828P_write_word(VCNL36828P_PS_CONF3_L, 0x0000);
	VCNL36828P_write_word(VCNL36828P_PS_THDL, 0x0000);
	VCNL36828P_write_word(VCNL36828P_PS_THDH, 0x0000);
	VCNL36828P_write_word(VCNL36828P_PS_CANC, 0x0000);
}

/*
 * Function: VCNL36828P_WRITE_Reg
 * --------------------
 * writes Register value
 *
 *  Reg: VCNL36828P_PS_THDL,
 *  	VCNL36828P_PS_THDH,
 *  	VCNL36828P_PS_CANC,
 *  	VCNL36828P_PS_DATA,
 *  	VCNL36828P_INT_FLAG,
 *  	or VCNL36828P_ID_CMD
 *  Data_lsb: the first byte of data, which is the least-significant byte of the data word.
 *  Data_msb: the second byte of data for the word sent, which is the most-significant byte.
 *
 */
void VCNL36828P_WRITE_Reg(uint8_t Reg, uint8_t Data_lsb, uint8_t Data_msb)
{
	uint16_t value;
	value = ((uint16_t)Data_msb << 8) | (uint16_t)Data_lsb;
	VCNL36828P_write_word(Reg, value);
}

/*
 * Function: VCNL36828P_READ_Reg
 * --------------------
 * reads Register value
 *
 *  Reg: VCNL36828P_PS_THDL,
 *  	VCNL36828P_PS_THDH,
 *  	VCNL36828P_PS_CANC,
 *  	VCNL36828P_PS_DATA,
 *  	VCNL36828P_INT_FLAG,
 *  	or VCNL36828P_ID_CMD
 *
 *  returns: Register Value between 0d0/0x00 and 0d65535/0xFFFF
 *			except for VCNL36828P_PS_THDH, VCNL36828P_PS_THDL and
 *			VCNL36828P_PS_CANC
 *
 */
uint16_t VCNL36828P_READ_Reg(uint8_t Reg)
{
	uint16_t RegValue = VCNL36828P_read_word(Reg);
	return RegValue;
}

/*
 * Function: VCNL36828P_GET_PS_DATA
 * --------------------
 * reads Proximity Data
 *
 *  returns: PS Data between 0d0 and 0d65535
 *
 */
uint16_t VCNL36828P_GET_PS_DATA()
{
	uint16_t ProxValue = VCNL36828P_read_word(VCNL36828P_PS_DATA);
	HAL_Delay(100);
	return ProxValue;
}

/*
 * Function: VCNL36828P_GET_PS_INT_FLAG
 * --------------------
 * reads the PS Interrupt Flag
 *
 *  returns: interrupt flag status
 *
 */
uint16_t VCNL36828P_GET_PS_INT_FLAG()
{
	uint16_t Interrupt = VCNL36828P_read_word(VCNL36828P_INT_FLAG);
	//  HAL_Delay(100);
	return Interrupt;
}

/*
 * Function: VCNL36828P_GET_ID
 * --------------------
 * reads the ID
 *
 *  returns: the ID
 *
 */
uint16_t VCNL36828P_GET_ID()
{
	uint16_t Get_ID = VCNL36828P_read_word(VCNL36828P_ID_CMD);
	return Get_ID;
}

/*
 * Function: VCNL36828P_GET_START_Bit
 * --------------------
 * reads the START bit
 *
 *  returns: an integer value returned
 *  		0
 *  		1 - Must be set to "1" when power on ready
 *
 */
bool VCNL36828P_GET_START_Bit()
{
	uint16_t GET_START_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_L);
	if((GET_START_Bit & 0x0080) == 0x0080){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_SD_Bit
 * --------------------
 * reads the PS_SD bit
 *
 *  returns: an integer value returned
 *  		0 - PS shutdown
 *  		1 - PS enable
 *
 */
bool VCNL36828P_GET_PS_SD_Bit()
{
	uint16_t GET_PS_SD_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_L);
	if((GET_PS_SD_Bit & 0x0001) == 0x0001){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_HD_Bit
 * --------------------
 * reads the PS_HD bit
 *
 *  returns: an integer value returned
 *  		0 - Disable
 *  		1 - Enable
 *
 */
bool VCNL36828P_GET_PS_HD_Bit()
{
	uint16_t GET_PS_HD_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	if((GET_PS_HD_Bit & 0x4000) == 0x4000){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_SP_INT_Bit
 * --------------------
 * reads the SUN_INT bit
 *
 *  returns: an integer value returned
 *  		0 - Disable
 *  		1 - Enable
 *
 */
bool VCNL36828P_GET_PS_SP_INT_Bit()
{
	uint16_t GET_PS_SP_INT_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	if((GET_PS_SP_INT_Bit & 0x2000) == 0x2000){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_SMART_PERS_Bit
 * --------------------
 * reads the PS_SMART_PERS bit
 *
 *  returns: an integer value returned
 *  		0 - Disable
 *  		1 - Enable
 *
 */
bool VCNL36828P_GET_PS_SMART_PERS_Bit()
{
	uint16_t GET_PS_SMART_PERS_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);
	if((GET_PS_SMART_PERS_Bit & 0x1000) == 0x1000){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_PERS_Bit
 * --------------------
 * reads the PS_PERS bit
 *
 *  returns: an integer value returned
 *  		0 - 1
 *  		1 - 2
 *  		2 - 3
 *  		3 - 4
 *
 */
int32_t VCNL36828P_GET_PS_PERS_Bit()
{
	int32_t PERS;
	uint16_t GET_PS_PERS_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);

	if ((GET_PS_PERS_Bit & 0x0C00) == 0x0000)
		PERS = 0;
	if ((GET_PS_PERS_Bit & 0x0C00) == 0x0400)
		PERS = 1;
	if ((GET_PS_PERS_Bit & 0x0C00) == 0x0800)
		PERS = 2;
	if ((GET_PS_PERS_Bit & 0x0C00) == 0x0C00)
		PERS = 3;

	return PERS;
}

/*
 * Function: VCNL36828P_GET_PS_INT_Bit
 * --------------------
 * reads the PS_INT bit
 *
 *  returns: an integer value returned
 *  		0 - PS Interrupt Disable
 *  		1 - Logic High/Low mode
 *  		2 - 1st trigger by high threshold window
 *  		3 - Trigger by each high/low threshold window
 *
 */
int32_t VCNL36828P_GET_PS_INT_Bit()
{
	int32_t INT;
	uint16_t GET_PS_INT_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF1_H);

	if ((GET_PS_INT_Bit & 0x0300) == 0x0000)
		INT = 0;
	if ((GET_PS_INT_Bit & 0x0300) == 0x0100)
		INT = 1;
	if ((GET_PS_INT_Bit & 0x0300) == 0x0200)
		INT = 2;
	if ((GET_PS_INT_Bit & 0x0300) == 0x0300)
		INT = 3;

	return INT;
}

/*
 * Function: VCNL36828P_GET_PS_PERIOD_Bit
 * --------------------
 * reads the PS_PERIOD bit
 *
 *  returns: an integer value returned
 *  		0 - 50m
 *  		1 - 100ms
 *  		2 - 200ms
 *  		3 - 400ms
 *
 */
int32_t VCNL36828P_GET_PS_PERIOD_Bit()
{
	int32_t Period;
	uint16_t GET_PS_PERIOD_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);

	if ((GET_PS_PERIOD_Bit & 0x00C0) == 0x0000)
		Period = 0;
	if ((GET_PS_PERIOD_Bit & 0x00C0) == 0x0040)
		Period = 1;
	if ((GET_PS_PERIOD_Bit & 0x00C0) == 0x0080)
		Period = 2;
	if ((GET_PS_PERIOD_Bit & 0x00C0) == 0x00C0)
		Period = 3;

	return Period;
}

/*
 * Function: VCNL36828P_GET_PS_IT_Bit
 * --------------------
 * reads the PS_IT bit
 *
 *  returns: an integer value returned
 *  		0 - 1T
 *  		1 - 2T
 *  		2 - 3T
 *  		3 - 4T
 *
 */
int32_t VCNL36828P_GET_PS_IT_Bit()
{
	int32_t T;
	uint16_t GET_PS_IT_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);

	if ((GET_PS_IT_Bit & 0x0030) == 0x0000)
		T = 0;
	if ((GET_PS_IT_Bit & 0x0030) == 0x0010)
		T = 1;
	if ((GET_PS_IT_Bit & 0x0030) == 0x0020)
		T = 2;
	if ((GET_PS_IT_Bit & 0x0030) == 0x0030)
		T = 3;

	return T;
}

/*
 * Function: VCNL36828P_GET_PS_MPS_Bit
 * --------------------
 * reads the PS_MPS bit
 *
 *  returns: an integer value returned
 *  		0 - 1
 *  		1 - 2
 *  		2 - 4
 *  		3 - 8
 *
 */
int32_t VCNL36828P_GET_PS_MPS_Bit()
{
	int32_t MPS;
	uint16_t GET_PS_MPS_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);

	if ((GET_PS_MPS_Bit & 0x000C) == 0x0000)
		MPS = 0;
	if ((GET_PS_MPS_Bit & 0x000C) == 0x0004)
		MPS = 1;
	if ((GET_PS_MPS_Bit & 0x000C) == 0x0008)
		MPS = 2;
	if ((GET_PS_MPS_Bit & 0x000C) == 0x000C)
		MPS = 3;

	return MPS;
}

/*
 * Function: VCNL36828P_GET_PS_ITB_Bit
 * --------------------
 * reads the PS_ITB bit
 *
 *  returns: an integer value returned
 *  		0 - 25us
 *  		1 - 50us
 *
 */
bool VCNL36828P_GET_PS_ITB_Bit()
{
	uint16_t GET_PS_ITB_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	if((GET_PS_ITB_Bit & 0x0002) == 0x0002){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_HG_Bit
 * --------------------
 * reads the PS_HG bit
 *
 *  returns: an integer value returned
 *  		0 - x1
 *  		1 - x2
 *
 */
bool VCNL36828P_GET_PS_HG_Bit()
{
	uint16_t GET_PS_HG_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_L);
	if((GET_PS_HG_Bit & 0x0001) == 0x0001){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_SENS_Bit
 * --------------------
 * reads the PS_SENS bit
 *
 *  returns: an integer value returned
 *  		0 - Normal
 *  		1 - High
 *
 */
bool VCNL36828P_GET_PS_SENS_Bit()
{
	uint16_t GET_PS_SENS_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);
	if((GET_PS_SENS_Bit & 0x2000) == 0x2000){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_OFFSET_Bit
 * --------------------
 * reads the PS_OFFSET bit
 *
 *  returns: an integer value returned
 *  		0 - Disable
 *  		1 - Enable
 *
 */
bool VCNL36828P_GET_PS_OFFSET_Bit()
{
	uint16_t GET_PS_OFFSET_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);
	if((GET_PS_OFFSET_Bit & 0x1000) == 0x1000){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_LED_Bit
 * --------------------
 * reads the PS_LED bit
 *
 *  returns: an integer value returned
 * 			0 - 8.3mA
 * 			1 - 9.7mA
 * 			2 - 11.7mA
 * 			3 - 13.1mA
 * 			4 - 15.7mA
 * 			5 - 17.1mA
 * 			6 - 19.1mA
 * 			7 - 20.4mA
 *
 */
int32_t VCNL36828P_GET_PS_LED_Bit()
{
	int32_t LED;
	uint16_t GET_PS_LED_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF2_H);

	if ((GET_PS_LED_Bit & 0x0700) == 0x0000)
		LED = 0;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0100)
		LED = 1;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0200)
		LED = 2;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0300)
		LED = 3;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0400)
		LED = 4;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0500)
		LED = 5;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0600)
		LED = 6;
	if ((GET_PS_LED_Bit & 0x0700) == 0x0700)
		LED = 7;

	return LED;
}

/*
 * Function: VCNL36828P_GET_PS_AF_Bit
 * --------------------
 * reads the PS_AF bit
 *
 *  returns: an integer value returned
 * 			0 - Disable
 * 			1 - Enable
 *
 */
bool VCNL36828P_GET_PS_AF_Bit()
{
	uint16_t GET_PS_AF_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF3_L);
	if((GET_PS_AF_Bit & 0x0010) == 0x0010){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_TRIG_Bit
 * --------------------
 * reads the PS_TRIG bit
 *
 *  returns: an integer value returned
 * 			0 - Disable
 * 			1 - Enable
 *
 */
bool VCNL36828P_GET_PS_TRIG_Bit()
{
	uint16_t GET_PS_TRIG_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF3_L);
	if((GET_PS_TRIG_Bit & 0x0020) == 0x0020){return 1;}
	else
		return 0;
}

/*
 * Function: VCNL36828P_GET_PS_SPERIOD_Bit
 * --------------------
 * reads PS short period setting
 *
 *  returns: an integer value returned
 * 			0 - follow PS_PERIOD register setting
 * 			1 - 6.25ms
 * 			2 - 12.5ms
 * 			3 - 25ms
 *
 */
int32_t VCNL36828P_GET_PS_SPERIOD_Bit()
{
	int32_t SPERIOD;
	uint16_t GET_PS_SPERIOD_Bit = VCNL36828P_read_word(VCNL36828P_PS_CONF3_H);

	if ((GET_PS_SPERIOD_Bit & 0xC000) == 0x0000)
		SPERIOD = 0;
	if ((GET_PS_SPERIOD_Bit & 0xC000) == 0x4000)
		SPERIOD = 1;
	if ((GET_PS_SPERIOD_Bit & 0xC000) == 0x8000)
		SPERIOD = 2;
	if ((GET_PS_SPERIOD_Bit & 0xC000) == 0xC000)
		SPERIOD = 3;

	return SPERIOD;
}

/*
 * Function: VCNL36828P_GET_PS_LowThreshold
 * --------------------
 * reads the Low Threshold
 *
 *  returns: LowThreshold
 *
 */
uint16_t VCNL36828P_GET_PS_LowThreshold()
{
	uint16_t LowThreshold = VCNL36828P_read_word(VCNL36828P_PS_THDL);
	return LowThreshold;
}

/*
 * Function: VCNL36828P_GET_PS_HighThreshold
 * --------------------
 * reads the High Threshold
 *
 *  returns: HighThreshold
 *
 */
uint16_t VCNL36828P_GET_PS_HighThreshold()
{
	uint16_t HighThreshold = VCNL36828P_read_word(VCNL36828P_PS_THDH);
	return HighThreshold;
}

/*
 * Function: I2Cm_bWriteBytes
 * --------------------
 * writes the command
 *
 *  Reg_num: HighThreshold
 *  Data_lsb: the first byte of data, which is the least-significant byte of the data word.
 *  Data_msb: the second byte of data for the word sent, which is the most-significant byte.
 *  bCnt: the number of bytes of data to be written
 *
 */
void I2Cm_bWriteBytes(uint8_t Reg_num, uint8_t Data_lsb, uint8_t Data_msb, uint8_t bCnt)
{
	if (Reg_num == (uint8_t)0x00)
	{
		VCNL36828P_WRITE_Reg(Reg_num, Data_lsb, Data_msb);
	}

	if (Reg_num == (uint8_t)0x01)
	{
		VCNL36828P_WRITE_Reg(Reg_num, Data_lsb, Data_msb);
	}

	/*
	 * Write command (0x02) for Device 1 add: 0x60
	 */
	if (Reg_num == (uint8_t)0x02)
	{
		VCNL36828P_WRITE_Reg(Reg_num, Data_lsb, Data_msb);
	}
}

/*
 * Function: I2Cm_fReadBytes
 * --------------------
 * reads the command
 *
 *  Reg_num: the command register value
 *  Data: the address of the starting element of a byte array
 *  bCnt: the number of bytes of data to be read
 *
 */
void I2Cm_fReadBytes(uint8_t Reg_num, uint8_t *Data, uint8_t bCnt)
{
	uint16_t Reg_data = VCNL36828P_READ_Reg(Reg_num);
	if (bCnt == 2) {
		Data[0] = Reg_data & 0xff;
		Data[1] = (Reg_data&0xff00)>>8;
	}
}

/*
 * Function: get_ps_if
 * --------------------
 * reads the PS Interrupt Flag
 *
 *
 */
uint16_t get_ps_if()
{
//	SetProxSensorCompleteFlag(false);

	Interrupt = VCNL36828P_GET_PS_INT_FLAG();

//	VCNL36828P_SET_PS_INT(VCNL36828P_PS_INT_EN);
//	VCNL36828P_SET_PS_TRIG(VCNL36828P_PS_TRIG_EN);
//	VCNL36828P_SET_PS_AF(VCNL36828P_PS_AF_EN);
//	SetProxSensorCompleteFlag(true);

//	uint16_t ps = VCNL36828P_GET_PS_DATA();
//	DPRINTF("get_ps_if, Interrupt=0x%04x, ps=%d : %s", Interrupt, ps, (Interrupt&VCNL36828P_PS_IF_AWAY)?"away":(Interrupt&VCNL36828P_PS_IF_CLOSE)?"close":"--");
	return Interrupt;
}

/*
 * Function: check_ps_if_event
 * --------------------
 * reads the PS Interrupt event
 *
 *  device_ps: the address of the starting element of a PS Data array
 *
 */
void check_ps_if_event(uint16_t *device_ps)
{
	//	DPRINTF("check_ps_if_event ENTER\r\n");
	get_ps_if();
	DPRINTF("check_ps_if_event, Interrupt=0x%04x\r\n", Interrupt);
//	if (Interrupt == 0xffff || Interrupt == 0) {
//		Interrupt = get_ps_if(VCNL36828P_SLAVE_ADD1_7bit);
//	}
//	//	if (Interrupt == 0xffff || Interrupt == 0) {
//	//		DPRINTF("check_ps_if_event reset p-sensor when Interrupt=0x%04x\r\n", Interrupt);
//	//		GPIO_InitTypeDef GPIO_InitStruct = {0};
//	//		/*Configure GPIO pins : PROX_INT_H_Pin ALS_INTN_H_Pin IMU_INT1_H_Pin */
//	//		GPIO_InitStruct.Pin = PROX_INT_H_Pin|ALS_INTN_H_Pin|IMU_INT1_H_Pin;
//	//		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//	//		GPIO_InitStruct.Pull = GPIO_PULLUP;
//	//		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//	//		initialize_VCNL36828P();
//	//		return;
//	//	}
//	SetProxSensorCompleteFlag(false);
//	is_VCNL36828P_changed = false;
//	bool is_close = (Interrupt & VCNL36828P_PS_IF_CLOSE) == VCNL36828P_PS_IF_CLOSE ? 1 : 0;
//	bool is_away = (Interrupt & VCNL36828P_PS_IF_AWAY) == VCNL36828P_PS_IF_AWAY ? 1 : 0;
//
//	//  DPRINTF("check_ps_if_event is_close=%d, is_away=%d, Interrupt=0x%04x\r\n", is_close, is_away, Interrupt);
//	if (is_close!=Sensor_Interrupt.PS_IF_CLOSE || is_away!=Sensor_Interrupt.PS_IF_AWAY) {
//		Sensor_Interrupt.PS_IF_CLOSE = is_close;
//		Sensor_Interrupt.PS_IF_AWAY = is_away;
//		is_VCNL36828P_changed = true;
//
//	}
//
//	//		  is_VCNL36828P_changed
//	//  Sensor_Interrupt.PS_IF_AWAY = (Interrupt & VCNL36828P_PS_IF_AWAY) == VCNL36828P_PS_IF_AWAY ? 1 : 0;
//	//  Sensor_Interrupt.PS_IF_CLOSE = (Interrupt & VCNL36828P_PS_IF_CLOSE) == VCNL36828P_PS_IF_CLOSE ? 1 : 0;
//	Sensor_Interrupt.PS_SPFLAG = (Interrupt & VCNL36828P_PS_SPFLAG) == VCNL36828P_PS_SPFLAG ? 1 : 0;
//	Sensor_Interrupt.PS_INVALID = (Interrupt & VCNL36828P_PS_INVALID) == VCNL36828P_PS_INVALID ? 1 : 0;
//	if (is_VCNL36828P_changed) {
//		if (Sensor_Interrupt.PS_IF_CLOSE) {
//			DPRINTF("******** PS_IF_CLOSE data=%d, %d ******** (%d)\r\n", device_ps[0], device_ps[1], is_VCNL36828P_changed);
//			device_ps[1] = VCNL36828P_GET_PS_DATA();
//		} else if (Sensor_Interrupt.PS_IF_AWAY) {
//			DPRINTF("######## PS_IF_AWAY  data=%d, %d ######## (%d)\r\n", device_ps[0], device_ps[1], is_VCNL36828P_changed);
//			device_ps[0] = VCNL36828P_GET_PS_DATA();
//		}
//
//		SetBrightnessCompleteFlag(true);
//		//  	  Set_I2C_Brightness();
//	}
//	SetProxSensorCompleteFlag(true);


}

/*
 * Function: set_ps_int_threshold
 * --------------------
 * sets the PS Interrupt threshold
 *
 *  LowThreshold: Value between 0d0 and 0d4095
 *  HighThreshold: Value between 0d0 and 0d4095
 *
 */
void set_ps_int_threshold(uint16_t LowThreshold, uint16_t HighThreshold)
{
	VCNL36828P_SET_PS_LowThreshold(LowThreshold);
	VCNL36828P_SET_PS_HighThreshold(HighThreshold);
}

/*
 * Function: get_ps_int_threshold
 * --------------------
 * gets the PS Interrupt threshold
 *
 *  LowThreshold: the low threshold
 *  HighThreshold: the high threshold
 *
 */
void get_ps_int_threshold(uint16_t *LowThreshold, uint16_t *HighThreshold)
{
	*LowThreshold = VCNL36828P_GET_PS_LowThreshold();
	*HighThreshold = VCNL36828P_GET_PS_HighThreshold();
}

/*
 * Function: read_vcnl36828_ps
 * --------------------
 * reads the PS data
 *
 *  returns: the PS data
 *
 */
uint16_t read_vcnl36828_ps()
{
	uint8_t buff[2];
	uint8_t lsb, msb;
	uint16_t ps_value;
	I2Cm_fReadBytes(VCNL36828P_PS_DATA, buff, 2);
	lsb = buff[0];
	msb = buff[1];
	ps_value = ((uint16_t)msb << 8) | (uint16_t)lsb;
	return ps_value;
}

/*
 * Function: AF_Mode
 * --------------------
 * runs the AF mode
 *
 */
#if 1 // betta changed
void AF_Mode()
{
	DPRINTF("do AF_Mode\r\n");
	uint32_t User_Data[USER_DATA_SIZE];
	int calibration_loop = 0;
	bool calibration_status = false;
	uint32_t PS_DATA[AverageCount];

	//1.)Initialization

	VCNL36828P_SET_PS_AF(VCNL36828P_PS_AF_DIS);

	//2.) Setting up PS
	//PS_CONF1_L and PS_CONF1_H
	//have been set up during the initialization

	//PS_CONF2_L
	// Full PS counts (PS_HD = 1, PS_HG = x2, PS_IT = 8T)
	// PS_IT * PS_ITB * PS_MPS < PS_PERIOD
	//PS_PERIOD doesn't have to be set for AF Mode
	VCNL36828P_SET_PS_PERIOD(VCNL36828P_PS_PERIOD_200ms);
	//Set the Persistence
//	VCNL36828P_SET_PS_PERS(VCNL36828P_PS_PERS_4);
	VCNL36828P_SET_PS_PERS(VCNL36828P_PS_PERS_1);
	//Set the Interrupt Mode
	VCNL36828P_SET_PS_INT(VCNL36828P_PS_INT_EN);
	//Enable/Disable Smart Persistence
	VCNL36828P_SET_PS_SMART_PERS(VCNL36828P_PS_SMART_PERS_EN);
	//PS_ST will be set when starting the measurement

	//PS_CONF2_H
	//Set the Integration Time
//	VCNL36828P_SET_PS_IT(VCNL36828P_PS_IT_8T);
//	VCNL36828P_SET_PS_IT(VCNL36828P_PS_IT_4T);
	// The maximum value of a 12-bit binary number is 212 - 1, or 4095.
	VCNL36828P_SET_PS_IT(VCNL36828P_PS_IT_1T);
	//Set the MPS
//	VCNL36828P_SET_PS_MPS(VCNL36828P_PS_MPS_4);
	VCNL36828P_SET_PS_MPS(VCNL36828P_PS_MPS_1);
	//Set the IT Bank
	VCNL36828P_SET_PS_ITB(VCNL36828P_PS_ITB_50);
	//Enable/Disable the High Gain setting
	//VCNL36828P_SET_PS_HG(VCNL36828P_PS_HG_x2);
	// The maximum value of a 12-bit binary number is 212 - 1, or 4095.
	VCNL36828P_SET_PS_HG(VCNL36828P_PS_HG_x1);

	//Set the Sensitivity
	//VCNL36828P_SET_PS_SENS(VCNL36828P_PS_SENS_HIGH);
	VCNL36828P_SET_PS_SENS(VCNL36828P_PS_SENS_NORMAL);
	//Set the offset
	VCNL36828P_SET_PS_OFFSET(VCNL36828P_PS_OFFSET_EN);
	//Set the VCSEL current driving
//	VCNL36828P_SET_PS_LED(VCNL36828P_PS_LED_8_3mA);
	VCNL36828P_SET_PS_LED(VCNL36828P_PS_LED_9_7mA);

	//PS_CONF3_L
	//Set the PS sunlight protect
	//VCNL36828P_SET_PS_SP_INT(VCNL36828P_PS_SP_INT_EN); // betta masked
	//PS_AF has been set during initialization and PS_TRIG needs to be set before doing proximity measurement

	//PS_CONF3_H
	//Set the Sensor output Bit Size
	//CH: turn on PS_HD to increase the sensor throughput
	VCNL36828P_SET_PS_HD(VCNL36828P_PS_HD_EN);
//	VCNL36828P_SET_PS_HD(VCNL36828P_PS_HD_DIS);

	//3.) Starting the PS
// betta
	VCNL36828P_SET_PS_ST(VCNL36828P_START);
	VCNL36828P_SET_PS_ON(VCNL36828P_PS_SD_EN);
	VCNL36828P_SET_PS_CANC(2);


	bool use_default=false;
	Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);

	if (User_Data[proximity]>1000) {
		HighThreshold = DEFALUT_HI_THRESHOLD;
		LowThreshold = DEFALUT_LO_THRESHOLD;
		User_Data[proximity]=HighThreshold;
		use_default=true;
	}
	else
	{
		HighThreshold = User_Data[proximity];
		LowThreshold = User_Data[proximity]-20;
	}

	// brightness level
	if (User_Data[brightness]>5) {
		User_Data[brightness]=5;
		use_default=true;
	}else if(User_Data[brightness]<0)
	{
		User_Data[brightness]=0;
		use_default=true;
	}
	if (use_default)
		Flash_Write_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, (uint64_t *)User_Data, USER_DATA_SIZE, FLASH_TYPEPROGRAM_DOUBLEWORD);

	VCNL36828P_SET_PS_HighThreshold(HighThreshold);
	VCNL36828P_SET_PS_LowThreshold(LowThreshold);
	//Clear initial interrupt
	VCNL36828P_GET_PS_INT_FLAG();
	HAL_Delay(1000);
}
#else
void AF_Mode()
{
	DPRINTF("do AF_Mode\r\n");
	int calibration_loop = 0;
	uint16_t ps_data;
	//1.)Initialization
	//Ensure that PS_ST = 1 before setting up PS_AF = Auto
	//Setting up AF Mode
	VCNL36828P_SET_PS_AF(VCNL36828P_PS_AF_DIS);

	//2.) Setting up PS
	//PS_CONF1_L and PS_CONF1_H
	//have been set up during the initialization

	//PS_CONF2_L
	// Full PS counts (PS_HD = 1, PS_HG = x2, PS_IT = 8T)
	// PS_IT * PS_ITB * PS_MPS < PS_PERIOD
	//PS_PERIOD doesn't have to be set for AF Mode
	VCNL36828P_SET_PS_PERIOD(VCNL36828P_PS_PERIOD_200ms);
	//Set the Persistence
//	VCNL36828P_SET_PS_PERS(VCNL36828P_PS_PERS_4);
	VCNL36828P_SET_PS_PERS(VCNL36828P_PS_PERS_1);
	//Set the Interrupt Mode
	VCNL36828P_SET_PS_INT(VCNL36828P_PS_INT_EN);
	//Enable/Disable Smart Persistence
	VCNL36828P_SET_PS_SMART_PERS(VCNL36828P_PS_SMART_PERS_EN);
	//PS_ST will be set when starting the measurement

	//PS_CONF2_H
	//Set the Integration Time
//	VCNL36828P_SET_PS_IT(VCNL36828P_PS_IT_8T);
	VCNL36828P_SET_PS_IT(VCNL36828P_PS_IT_4T);
	//Set the MPS
//	VCNL36828P_SET_PS_MPS(VCNL36828P_PS_MPS_4);
	VCNL36828P_SET_PS_MPS(VCNL36828P_PS_MPS_1);
	//Set the IT Bank
	VCNL36828P_SET_PS_ITB(VCNL36828P_PS_ITB_50);
	//Enable/Disable the High Gain setting
	VCNL36828P_SET_PS_HG(VCNL36828P_PS_HG_x2);

	//Set the Sensitivity
//	VCNL36828P_SET_PS_SENS(VCNL36828P_PS_SENS_HIGH);
	VCNL36828P_SET_PS_SENS(VCNL36828P_PS_SENS_NORMAL);
	//Set the offset
	VCNL36828P_SET_PS_OFFSET(VCNL36828P_PS_OFFSET_EN);
	//Set the VCSEL current driving
//	VCNL36828P_SET_PS_LED(VCNL36828P_PS_LED_8_3mA);
	VCNL36828P_SET_PS_LED(VCNL36828P_PS_LED_9_7mA);

	//PS_CONF3_L
	//Set the PS sunlight protect
	VCNL36828P_SET_PS_SP_INT(VCNL36828P_PS_SP_INT_EN);
	//PS_AF has been set during initialization and PS_TRIG needs to be set before doing proximity measurement

	//PS_CONF3_H
	//Set the Sensor output Bit Size
	//CH: turn on PS_HD to increase the sensor throughput
	VCNL36828P_SET_PS_HD(VCNL36828P_PS_HD_EN);
//	VCNL36828P_SET_PS_HD(VCNL36828P_PS_HD_DIS);

	//3.) Starting the PS
	VCNL36828P_SET_PS_ST(VCNL36828P_START);
	VCNL36828P_SET_PS_ON(VCNL36828P_PS_SD_EN);

	// JOSEPH: add for p-sensor calibration when first time ME/Hardware assembly @20220520
//	if (Btn_Pressed_Reset == Brightness_Btn_GetState())
//		calibration_loop = 5000;
//	calibration_loop = 0;
	CalibValue = DEFALUT_CALIBRATION_VALUE;
//	VCNL36828P_SET_PS_CANC(OffsetValue);

	for (int loop=0;loop<calibration_loop;loop++) {
		//4.) Threshold Setting and Offset Measurement
		CalibValue = 0;
		for(int i=0;i<AverageCount;i++)
		{
			//Enable trigger to start offset measurement
			VCNL36828P_SET_PS_TRIG(VCNL36828P_PS_TRIG_EN);
			//CalibValue += VCNL36828P_GET_PS_DATA();
			ps_data = VCNL36828P_GET_PS_DATA();
			DPRINTF("ps_data: %d\r\n", ps_data);
			CalibValue += ps_data;
		}

		//Calculate the average of the offset measurement
		CalibValue /= AverageCount;
		DPRINTF("CalibValue: %lu\r\n", CalibValue);

		// JOSEPH: add for light-sensor test @20220531
//		nowProximityStatus = PS_CLOSE_EVENT;
//		float old = prev_lux;
		char buffer[100];
		float lux = read_CM32183E();
		sprintf(buffer, "CalibValue:%lu\tLUX:%6.2f\t(als_code:%u H:%lu, L:%lu)\r\n", CalibValue, lux, als_code, threshold_high, threshold_low);
		//	  sprintf(buffer, "CalibValue:%d\tLUX:%6.2f (%6.2f)\t(als_code:%d H:%lu, L:%lu)\tR:0x%zx L:0x%zx \r\n", CalibValue, lux, old, als_code, threshold_high, threshold_low, brightness.left_b, brightness.right_b);
//		sprintf(buffer, "CalibValue:%d\tLUX:%6.2f\t(als_code:%d H:%lu, L:%lu)\tR:%d%% L:%d%% \r\n", CalibValue, lux, als_code, threshold_high, threshold_low, brightness.left_b, brightness.right_b);
//		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)&buffer[0], strlen(buffer));
//
//		if(USBD_CDC_TransmitPacket(&hUsbDeviceFS) == USBD_OK)
//		{
//		}
		sendCdcData((uint8_t*)&buffer[0], strlen(buffer));
	}

	//Perform Offset Measurement
	if(SEL_Offset == 0) OffsetValue = 0;
	if(SEL_Offset == 1) OffsetValue = CalibValue;
	//Set Cancellation register to eliminate offset
	VCNL36828P_SET_PS_CANC(OffsetValue);
	//Set Low Threshold
	VCNL36828P_SET_PS_LowThreshold(LowThreshold);
	//Set High Threshold
	VCNL36828P_SET_PS_HighThreshold(HighThreshold);

	DPRINTF("Default CalibValue: %lu, LowThreshold: %lu, HighThreshold: %lu\r\n", CalibValue, LowThreshold, HighThreshold);
	//Clear initial interrupt
	VCNL36828P_GET_PS_INT_FLAG();

	HAL_Delay(1000);
}
#endif

/*
 * Function: initialize_VCNL36828P
 * --------------------
 * initiates P sensor and its default configuration
 *
 */
void initialize_VCNL36828P(void)
{
	DPRINTF("initialize_VCNL36828P\r\n");
//	SetProxSensorCompleteFlag(false);

	/* Reset Sensor to default value */
#if defined(VCNL36828P_I2C1_ADD1)
	Reset_Sensor(VCNL36828P_SLAVE_ADD1_7bit);
#elif defined(VCNL36828P_I2C1_ADD2)
	Reset_Sensor(VCNL36828P_SLAVE_ADD2_7bit);
#endif /* defined(VCNL36828P_I2C1_ADD1) */

	/* Choose to turn on/off offset cancellation measurement */
	//Only applicable for  Auto, AF and Low Power Mode
	//Auto Calibration will perform the offset measurement automatically based on the selected settings
	/* SEL_Offset - Select the offset mode:
	 * 0 - Turn Off offset cancellation measurement
	 * 1 - Turn On offset cancellation measurement
	 */
	SEL_Offset = 1;

	AF_Mode();
//	SetProxSensorCompleteFlag(true);
}

/*
 * Function: VCNL36828P_write_word
 * --------------------
 * writes the data to a command register
 *
 *  command: the command register address
 *  val: the input data
 *
 */
void VCNL36828P_write_word(uint8_t command, uint16_t val)
{
	I2CWriteBytes(&VCNL36828P_I2C1, VCNL36828P_SLAVE_ADD1_7bit, command, (uint8_t*)&val, sizeof(val));
}

/*
 * Function: VCNL36828P_read_word
 * --------------------
 * reads the data from a command register
 *  command: the command register address
 *  val: the output data
 *
 */
uint16_t VCNL36828P_read_word(uint8_t command)
{
	uint8_t data[2] = {0};

	I2CReadBytes(&VCNL36828P_I2C1, VCNL36828P_SLAVE_ADD1_7bit, command, data, sizeof(data));
	uint16_t val = ((uint16_t)data[1] << 8) | (uint16_t)data[0];
	return val;
}
