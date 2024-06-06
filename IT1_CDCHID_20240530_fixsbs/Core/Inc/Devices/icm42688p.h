/*
 * icm42688p.h
 *
 *  Created on: 2022年2月22日
 *      Author: LEAPSY
 */

#ifndef INC_ICM42688P_H_
#define INC_ICM42688P_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "imu.h"

#ifndef _BV
  #define _BV(x) (1 << (x))
#endif

#ifndef __FLT_EPSILON__
#define __FLT_EPSILON__ 1.19209290e-07f
#endif

#if !defined(FLT_EPSILON)
#define FLT_EPSILON     __FLT_EPSILON__
#endif

/* Defines */
#define READ							0x80
#define WRITE							0x00

/* Register Bit Definition */
#define Bit0 (1 << 0)
#define Bit1 (1 << 1)
#define Bit2 (1 << 2)
#define Bit3 (1 << 3)
#define Bit4 (1 << 4)
#define Bit5 (1 << 5)
#define Bit6 (1 << 6)
#define Bit7 (1 << 7)

#define SPI_SPEED 					24 * 1000 * 1000 // 24 MHz SPI

#define TEMPERATURE_SENSITIVITY 	132.48f // LSB/C
#define TEMPERATURE_OFFSET 			25.f // C

#define CONSTANTS_ONE_G 			9.80665f

// Sensor Configuration
#define FIFO_SAMPLE_DT 		1e6f / 8000.f     // 8000 Hz accel & gyro ODR configured
#define GYRO_RATE 			1e6f / FIFO_SAMPLE_DT
#define ACCEL_RATE 			1e6f / FIFO_SAMPLE_DT

#define SENSOR_MAX_SAMPLES	1280
/* ICM-42688 Registers */
#define ICM42688_ID						0x47

/* Typedefs */
typedef enum
{
	ub_0 = 0 << 0,
	ub_1 = 1 << 0,
	ub_2 = 2 << 0,
	ub_4 = 3 << 0
} userbank;

typedef enum
{
	_2000dps,
	_1000dps,
	_500dps,
	_250dps,
	_125dps,
	_62_5dps,
	_31_25dps,
	_15_625dps
} gyro_full_scale;

typedef enum
{
	_2g,
	_4g,
	_8g,
	_16g
} accel_full_scale;

typedef enum
{
	_32khz = 320000000,
	_16khz = 160000000,
	_8khz = 80000000,
	_4khz = 40000000,
	_2khz = 20000000,
	_1khz = 10000000,
	_200hz = 2000000,
	_100hz = 1000000,
	_50hz = 500000,
	_25hz = 250000,
	_12_5hz = 125000,
	_6_25hz = 62500,
	_3_125hz = 31250,
	_1_5625hz = 15625,
	_400hz = 4000000,
	_500hz = 5000000
} odr;

// USER BANK 0 REGISTER MAP
typedef enum
{
	DEVICE_CONFIG      = 0x11,

	INT_CONFIG         = 0x14,

	FIFO_CONFIG        = 0x16,

	TEMP_DATA1         = 0x1D,
	TEMP_DATA0         = 0x1E,

	ACCEL_DATA_X1      = 0x1F,
	ACCEL_DATA_X0      = 0x20,
	ACCEL_DATA_Y1      = 0x21,
	ACCEL_DATA_Y0      = 0x22,
	ACCEL_DATA_Z1      = 0x23,
	ACCEL_DATA_Z0      = 0x24,
	GYRO_DATA_X1       = 0x25,
	GYRO_DATA_X0       = 0x26,
	GYRO_DATA_Y1       = 0x27,
	GYRO_DATA_Y0       = 0x28,
	GYRO_DATA_Z1       = 0x29,
	GYRO_DATA_Z0       = 0x2A,
	TMST_FSYNCH        = 0x2B,
	TMST_FSYNCL        = 0x2C,

	INT_STATUS         = 0x2D,
	FIFO_COUNTH        = 0x2E,
	FIFO_COUNTL        = 0x2F,
	FIFO_DATA          = 0x30,

	APEX_DATA0         = 0x31,
	APEX_DATA1         = 0x32,
	APEX_DATA2         = 0x33,
	APEX_DATA3         = 0x34,
	APEX_DATA4         = 0x35,
	APEX_DATA5         = 0x36,

	INT_STATUS2        = 0x37,
	INT_STATUS3        = 0x38,

	SIGNAL_PATH_RESET  = 0x4B,
	INTF_CONFIG0       = 0x4C,
	INTF_CONFIG1       = 0x4D,
	PWR_MGMT0          = 0x4E,
	GYRO_CONFIG0       = 0x4F,
	ACCEL_CONFIG0      = 0x50,
	GYRO_CONFIG1       = 0x51,
	GYRO_ACCEL_CONFIG0 = 0x52,
	ACCEL_CONFIG1      = 0x53,

	TMST_CONFIG        = 0x54,
	APEX_CONFIG0       = 0x56,
	SMD_CONFIG	       = 0x57,

	FIFO_CONFIG1       = 0x5F,
	FIFO_CONFIG2       = 0x60,
	FIFO_CONFIG3       = 0x61,

	FSYNC_CONFIG       = 0x62,

	INT_CONFIG0        = 0x63,
	INT_CONFIG1        = 0x64,

	INT_SOURCE0        = 0x65,
	INT_SOURCE1        = 0x66,
	INT_SOURCE3        = 0x68,
	INT_SOURCE4        = 0x69,

	SELF_TEST_CONFIG   = 0x70,

	WHO_AM_I           = 0x75,
	REG_BANK_SEL       = 0x76,
} user_bank_0_register;

// USER BANK 1 REGISTER MAP
typedef enum
{
	SENSOR_CONFIG0		= 0x03,

	GYRO_CONFIG_STATIC2 = 0x0B,
	GYRO_CONFIG_STATIC3 = 0x0C,
	GYRO_CONFIG_STATIC4 = 0x0D,
	GYRO_CONFIG_STATIC5 = 0x0E,
	GYRO_CONFIG_STATIC6 = 0x0F,
	GYRO_CONFIG_STATIC7 = 0x10,
	GYRO_CONFIG_STATIC8 = 0x11,
	GYRO_CONFIG_STATIC9 = 0x12,
	GYRO_CONFIG_STATIC10 = 0x13,

	XG_ST_DATA			= 0x5F,
	YG_ST_DATA			= 0x60,
	ZG_ST_DATA			= 0x61,

	TMSTVAL0			= 0x62,
	TMSTVAL1			= 0x63,
	TMSTVAL2			= 0x64,

	INTF_CONFIG4        = 0x7A,
	INTF_CONFIG5        = 0x7B,
	INTF_CONFIG6        = 0x7C,
} user_bank_1_register;

// USER BANK 2 REGISTER MAP
typedef enum
{
	ACCEL_CONFIG_STATIC2 = 0x03,
	ACCEL_CONFIG_STATIC3 = 0x04,
	ACCEL_CONFIG_STATIC4 = 0x05,

	XA_ST_DATA			 = 0x3B,
	YA_ST_DATA			 = 0x3C,
	ZA_ST_DATA			 = 0x3D,
} user_bank_2_register;

// USER BANK 4 REGISTER MAP
typedef enum
{
	APEX_CONFIG1 		= 0x40,
	APEX_CONFIG2 		= 0x41,
	APEX_CONFIG3 		= 0x42,
	APEX_CONFIG4 		= 0x43,
	APEX_CONFIG5 		= 0x44,
	APEX_CONFIG6 		= 0x45,
	APEX_CONFIG7 		= 0x46,
	APEX_CONFIG8 		= 0x47,
	APEX_CONFIG9 		= 0x48,

	ACCEL_WOM_X_THR 	= 0x4A,
	ACCEL_WOM_Y_THR 	= 0x4B,
	ACCEL_WOM_Z_THR 	= 0x4C,

	INT_SOURCE6 		= 0x4D,
	INT_SOURCE7 		= 0x4E,
	INT_SOURCE8 		= 0x4F,
	INT_SOURCE9 		= 0x50,
	INT_SOURCE10 		= 0x51,

	OFFSET_USER0 		= 0x77,
	OFFSET_USER1 		= 0x78,
	OFFSET_USER2 		= 0x79,
	OFFSET_USER3 		= 0x7A,
	OFFSET_USER4 		= 0x7B,
	OFFSET_USER5 		= 0x7C,
	OFFSET_USER6 		= 0x7D,
	OFFSET_USER7 		= 0x7E,
	OFFSET_USER8 		= 0x7F,
} user_bank_4_register;


//---------------- BANK0 Register bits

// DEVICE_CONFIG
typedef enum
{
	SOFT_RESET_CONFIG = Bit0, //
} device_config_bit;

// INT_CONFIG
typedef enum
{
	INT2_MODE           = Bit5,
	INT2_DRIVE_CIRCUIT  = Bit4,
	INT2_POLARITY       = Bit3,
	INT1_MODE           = Bit2,
	INT1_DRIVE_CIRCUIT  = Bit1,
	INT1_POLARITY       = Bit0,
} int_config_bit;

// FIFO_CONFIG
typedef enum
{
	// 7:6 FIFO_MODE
	FIFO_MODE_STOP_ON_FULL = Bit7 | Bit6, // 11: STOP-on-FULL Mode
} fifo_config_bit;

// INT_STATUS
typedef enum
{
	RESET_DONE_INT = Bit4,
	DATA_RDY_INT   = Bit3,
	FIFO_THS_INT   = Bit2,
	FIFO_FULL_INT  = Bit1,
} int_statis_bit;

// SIGNAL_PATH_RESET
typedef enum
{
	ABORT_AND_RESET = Bit3,
	FIFO_FLUSH      = Bit1,
} signal_path_reset_bit;

// PWR_MGMT0
typedef enum
{
	GYRO_MODE_LOW_NOISE  = Bit3 | Bit2, // 11: Places gyroscope in Low Noise (LN) Mode
	ACCEL_MODE_LOW_NOISE = Bit1 | Bit0, // 11: Places accelerometer in Low Noise (LN) Mode
} pwr_mgmt0_bit;

// GYRO_CONFIG0
typedef enum
{
	// 7:5 GYRO_FS_SEL
	GYRO_FS_SEL_2000_DPS 	= 0,            		// 0b000 = ±2000dps (default)
	GYRO_FS_SEL_1000_DPS 	= Bit5,         		// 0b001 = ±1000 dps
	GYRO_FS_SEL_500_DPS  	= Bit6,         		// 0b010 = ±500 dps
	GYRO_FS_SEL_250_DPS  	= Bit6 | Bit5,  		// 0b011 = ±250 dps
	GYRO_FS_SEL_125_DPS  	= Bit7,         		// 0b100 = ±125 dps
	GYRO_FS_SEL_62_5_DPS  	= Bit7 | Bit5,  		// 0b101 = ±62.5 dps
	GYRO_FS_SEL_31_25_DPS  	= Bit7 | Bit6,  		// 0b110 = ±31.25 dps
	GYRO_FS_SEL_15_625_DPS  = Bit7 | Bit6 | Bit5,	// 0b111 = ±15.625 dps

	// 3:0 GYRO_ODR
	//  0001: 32kHz
	GYRO_ODR_32KHZ_SET   = Bit0,
	GYRO_ODR_32KHZ_CLEAR = Bit3 | Bit2 | Bit0,
	//  0010: 16kHz
	GYRO_ODR_16KHZ_SET   = Bit1,
	GYRO_ODR_16KHZ_CLEAR = Bit3 | Bit2 | Bit0,
	//  0011: 8kHz
	GYRO_ODR_8KHZ_SET    = Bit1 | Bit0,
	GYRO_ODR_8KHZ_CLEAR  = Bit3 | Bit2,
	//  0100: 4kHz
	GYRO_ODR_4KHZ_SET    = Bit2,
	GYRO_ODR_4KHZ_CLEAR  = Bit3 | Bit1 | Bit0,
	//  0101: 2kHz
	GYRO_ODR_2KHZ_SET    = Bit2 | Bit0,
	GYRO_ODR_2KHZ_CLEAR  = Bit3 | Bit1,
	//  0110: 1kHz (default)
	GYRO_ODR_1KHZ_SET    = Bit2 | Bit1,
	GYRO_ODR_1KHZ_CLEAR  = Bit3 | Bit0,
	//  0111: 200Hz
	GYRO_ODR_200HZ_SET   = Bit2 | Bit1 | Bit0,
	GYRO_ODR_200HZ_CLEAR = Bit3,
	//  1000: 100Hz
	GYRO_ODR_100HZ_SET   = Bit3,
	GYRO_ODR_100HZ_CLEAR = Bit2 | Bit1 | Bit0,
	//  1001: 50Hz
	GYRO_ODR_50HZ_SET    = Bit3 | Bit0,
	GYRO_ODR_50HZ_CLEAR  = Bit2 | Bit1,
	//  1010: 25Hz
	GYRO_ODR_25HZ_SET    = Bit3 | Bit1,
	GYRO_ODR_25HZ_CLEAR  = Bit2 | Bit0,
	//  1011: 12.5Hz
	GYRO_ODR_12_5HZ_SET   = Bit3 | Bit1 | Bit0,
	GYRO_ODR_12_5HZ_CLEAR = Bit2,
	//  1111: 500Hz
	GYRO_ODR_500HZ_SET   = Bit3 | Bit2 | Bit1 | Bit0,
	GYRO_ODR_500HZ_CLEAR = (0 << 0),
} gyro_config0_bit;

// ACCEL_CONFIG0
typedef enum
{
	// 7:5 ACCEL_FS_SEL
	ACCEL_FS_SEL_16G 	= 0, 			// 000: ±16g (default)
	ACCEL_FS_SEL_8G  	= Bit5,        	// 001: ±8g
	ACCEL_FS_SEL_4G  	= Bit6, 		// 010: ±4g
	ACCEL_FS_SEL_2G  	= Bit6 | Bit5, 	// 011: ±2g

	// 3:0 ACCEL_ODR
	//  0001: 32kHz (LN mode)
	ACCEL_ODR_32KHZ_SET   = Bit0,
	ACCEL_ODR_32KHZ_CLEAR = Bit3 | Bit2 | Bit0,
	//  0010: 16kHz (LN mode)
	ACCEL_ODR_16KHZ_SET   = Bit1,
	ACCEL_ODR_16KHZ_CLEAR = Bit3 | Bit2 | Bit0,
	//  0011: 8kHz (LN mode)
	ACCEL_ODR_8KHZ_SET    = Bit1 | Bit0,
	ACCEL_ODR_8KHZ_CLEAR  = Bit3 | Bit2,
	//  0100: 4kHz (LN mode)
	ACCEL_ODR_4KHZ_SET    = Bit2,
	ACCEL_ODR_4KHZ_CLEAR  = Bit3 | Bit1 | Bit0,
	//  0101: 2kHz (LN mode)
	ACCEL_ODR_2KHZ_SET    = Bit2 | Bit0,
	ACCEL_ODR_2KHZ_CLEAR  = Bit3 | Bit1,
	//  0110: 1kHz (LN mode) (default)
	ACCEL_ODR_1KHZ_SET    = Bit2 | Bit1,
	ACCEL_ODR_1KHZ_CLEAR  = Bit3 | Bit0,
	//  0111: 200Hz (LP or LN mode)
	ACCEL_ODR_200HZ_SET   = Bit2 | Bit1 | Bit0,
	ACCEL_ODR_200HZ_CLEAR = Bit3,
	//  1000: 100Hz (LP or LN mode)
	ACCEL_ODR_100HZ_SET   = Bit3,
	ACCEL_ODR_100HZ_CLEAR = Bit2 | Bit1 | Bit0,
	//  1001: 50Hz (LP or LN mode)
	ACCEL_ODR_50HZ_SET    = Bit3 | Bit0,
	ACCEL_ODR_50HZ_CLEAR  = Bit2 | Bit1,
	//  1010: 25Hz (LP or LN mode)
	ACCEL_ODR_25HZ_SET    = Bit3 | Bit1,
	ACCEL_ODR_25HZ_CLEAR  = Bit2 | Bit0,
	//  1011: 12.5Hz (LP or LN mode)
	ACCEL_ODR_12_5HZ_SET   = Bit3 | Bit1 | Bit0,
	ACCEL_ODR_12_5HZ_CLEAR = Bit2,
	//  1100: 6.25kHz (LP mode)
	ACCEL_ODR_6_25HZ_SET   = Bit3 | Bit2,
	ACCEL_ODR_6_25HZ_CLEAR = Bit1 | Bit0,
	//  1101: 3.125Hz (LP mode)
	ACCEL_ODR_3_125HZ_SET   = Bit3 | Bit2 | Bit0,
	ACCEL_ODR_3_125HZ_CLEAR = Bit1,
	//  1110: 1.5625Hz (LP mode)
	ACCEL_ODR_1_5625HZ_SET   = Bit3 | Bit2 | Bit1,
	ACCEL_ODR_1_5625HZ_CLEAR = Bit0,
	//  1111: 500Hz (LP or LN mode)
	ACCEL_ODR_500HZ_SET    = Bit3 | Bit2 | Bit1 | Bit0,
	ACCEL_ODR_500HZ_CLEAR  = (0 << 0),
} accel_config0_bit;

// GYRO_CONFIG1
typedef enum
{
	GYRO_UI_FILT_ORD = Bit3, // 10: 3rd Order
	GYRO_DEC2_M2_ORD = Bit1, // 10: 3rd Order
} gyro_config1_bit;

// GYRO_ACCEL_CONFIG0
typedef enum
{
	// 7:4 ACCEL_UI_FILT_BW
	ACCEL_UI_FILT_BW = Bit7 | Bit6 | Bit5 | Bit4, // 0: BW=ODR/2

	// 3:0 GYRO_UI_FILT_BW
	GYRO_UI_FILT_BW  = Bit3 | Bit2 | Bit1 | Bit0, // 0: BW=ODR/2
} gyro_accel_config0_bit;

// ACCEL_CONFIG1
typedef enum
{
	ACCEL_UI_FILT_ORD = Bit4 | Bit3, // 00: 1st Order
} accel_config1_bit;

// TMST_CONFIG
typedef enum
{
	TMST_TO_REGS_EN = Bit4, // 0: TMST_VALUE[19:0] read always returns 0s
							// 1: TMST_VALUE[19:0] read returns timestamp value
	TMST_RES		= Bit3, // Time Stamp resolution: When set to 0 (default),
							// time stamp resolution is 1 μs.
							// When set to 1, resolution is 16μs
	TMST_DELTA_EN	= Bit2,	// Time Stamp delta enable: When set to 1, the time stamp field
							// contains the measurement of time since the last occurrence of ODR.
	TMST_FSYNC_EN	= Bit1,	// Time Stamp register FSYNC enable (default). When set to 1,
							// the contents of the Timestamp feature of FSYNC is enabled.
							// The user also needs to select FIFO_TMST_FSYNC_EN in order to
							// propagate the timestamp value to the FIFO.
	TMST_EN			= Bit0,	// 0: Time Stamp register disable
							// 1: Time Stamp register enable (default)
} tmst_config_bit;

// FIFO_CONFIG1
typedef enum
{
	FIFO_RESUME_PARTIAL_RD = Bit6,
	FIFO_WM_GT_TH          = Bit5,
	FIFO_HIRES_EN          = Bit4,
	FIFO_TEMP_EN           = Bit2,
	FIFO_GYRO_EN           = Bit1,
	FIFO_ACCEL_EN          = Bit0,
} fifo_config1_bit;

// INT_CONFIG0
typedef enum
{
	// 3:2 FIFO_THS_INT_CLEAR
	CLEAR_ON_FIFO_READ = Bit3,
} int_config0_bit;

// INT_SOURCE0
typedef enum
{
	UI_FSYNC_INT1_EN   = Bit6,
	PLL_RDY_INT1_EN    = Bit5,
	RESET_DONE_INT1_EN = Bit4,
	UI_DRDY_INT1_EN    = Bit3,
	FIFO_THS_INT1_EN   = Bit2, // FIFO threshold interrupt routed to INT1
	FIFO_FULL_INT1_EN  = Bit1,
	UI_AGC_RDY_INT1_EN = Bit0,
} int_source0_bit;

// REG_BANK_SEL
typedef enum
{
	USER_BANK_0 = 0,           	// 0: Select USER BANK 0.
	USER_BANK_1 = Bit0,        	// 1: Select USER BANK 1.
	USER_BANK_2 = Bit1,        	// 2: Select USER BANK 2.
	USER_BANK_3 = Bit1 | Bit0, 	// 3: Select USER BANK 3.
	USER_BANK_4 = Bit2, 		// 4: Select USER BANK 4.
} reg_bank_sel_bit;


//---------------- BANK1 Register bits

// GYRO_CONFIG_STATIC2
typedef enum
{
	GYRO_AAF_DIS = Bit1,
	GYRO_NF_DIS  = Bit0,
} gyro_config_static2_bit;


//---------------- BANK2 Register bits

// ACCEL_CONFIG_STATIC2
typedef enum
{
	ACCEL_AAF_DIS = Bit0,
} accel_config_static2_bit;


// FIFO_DATA layout when FIFO_CONFIG1 has FIFO_GYRO_EN and FIFO_ACCEL_EN set

// Packet 4
typedef struct FIFO_DATA {
	uint8_t FIFO_Header;
	uint8_t ACCEL_DATA_X1; // Accel X [19:12]
	uint8_t ACCEL_DATA_X0; // Accel X [11:4]
	uint8_t ACCEL_DATA_Y1; // Accel Y [19:12]
	uint8_t ACCEL_DATA_Y0; // Accel Y [11:4]
	uint8_t ACCEL_DATA_Z1; // Accel Z [19:12]
	uint8_t ACCEL_DATA_Z0; // Accel Z [11:4]
	uint8_t GYRO_DATA_X1;  // Gyro X [19:12]
	uint8_t GYRO_DATA_X0;  // Gyro X [11:4]
	uint8_t GYRO_DATA_Y1;  // Gyro Y [19:12]
	uint8_t GYRO_DATA_Y0;  // Gyro Y [11:4]
	uint8_t GYRO_DATA_Z1;  // Gyro Z [19:12]
	uint8_t GYRO_DATA_Z0;  // Gyro Z [11:4]
	uint8_t TEMP_DATA1;    // Temperature[15:8]
	uint8_t TEMP_DATA0;    // Temperature[7:0]
	uint8_t TimeStamp_h;   // TimeStamp[15:8]
	uint8_t TimeStamp_l;   // TimeStamp[7:0]
	uint8_t Ext_Accel_X_Gyro_X; // Accel X [3:0] Gyro X [3:0]
	uint8_t Ext_Accel_Y_Gyro_Y; // Accel Y [3:0] Gyro Y [3:0]
	uint8_t Ext_Accel_Z_Gyro_Z; // Accel Z [3:0] Gyro Z [3:0]
} fifo_data;

// With FIFO_ACCEL_EN and FIFO_GYRO_EN header should be 8’b_0110_10xx
typedef enum
{
	HEADER_MSG             = Bit7, // 1: FIFO is empty
	HEADER_ACCEL           = Bit6, // 1: Packet is sized so that accel data have location in the packet, FIFO_ACCEL_EN must be 1
	HEADER_GYRO            = Bit5, // 1: Packet is sized so that gyro data have location in the packet, FIFO_GYRO_EN must be1
	HEADER_20              = Bit4, // 1: Packet has a new and valid sample of extended 20-bit data for gyro and/or accel
	HEADER_TIMESTAMP_FSYNC = Bit3 | Bit2,
	HEADER_ODR_ACCEL       = Bit1, // 1: The ODR for accel is different for this accel data packet compared to the previous accel packet
	HEADER_ODR_GYRO        = Bit0, // 1: The ODR for gyro is different for this gyro data packet compared to the previous gyro packet
} fifo_header_bit;

typedef struct register_bank0_config_t {
	user_bank_0_register reg;
	uint8_t set_bits;
	uint8_t clear_bits;
} register_bank0_config;

typedef struct register_bank1_config_t {
	user_bank_1_register reg;
	uint8_t set_bits;
	uint8_t clear_bits;
} register_bank1_config;

typedef struct register_bank2_config_t {
	user_bank_2_register reg;
	uint8_t set_bits : 1;
	uint8_t clear_bits : 1;
} register_bank2_config;

#define size_register_bank0_cfg 14
static register_bank0_config icm42688_register_bank0_cfg[size_register_bank0_cfg] = {
	// Register                              | Set bits, Clear bits
	{ INT_CONFIG,			INT1_MODE | INT1_DRIVE_CIRCUIT,	INT1_POLARITY },
	{ FIFO_CONFIG,          FIFO_MODE_STOP_ON_FULL, 0 },
	{ PWR_MGMT0,            GYRO_MODE_LOW_NOISE | ACCEL_MODE_LOW_NOISE, 0 },
	{ GYRO_CONFIG0,         GYRO_ODR_100HZ_SET, GYRO_ODR_100HZ_CLEAR },
	{ ACCEL_CONFIG0,        ACCEL_ODR_100HZ_SET, ACCEL_ODR_100HZ_CLEAR },
	{ GYRO_CONFIG1,         0, GYRO_UI_FILT_ORD },
	{ GYRO_ACCEL_CONFIG0,   0, ACCEL_UI_FILT_BW | GYRO_UI_FILT_BW },
	{ ACCEL_CONFIG1,        0, ACCEL_UI_FILT_ORD },
	{ TMST_CONFIG,        	TMST_TO_REGS_EN | TMST_EN , TMST_RES | TMST_DELTA_EN | TMST_FSYNC_EN },
	{ FIFO_CONFIG1,         FIFO_GYRO_EN | FIFO_ACCEL_EN, 0 }, // FIFO_WM_GT_TH | FIFO_HIRES_EN | FIFO_TEMP_EN | FIFO_GYRO_EN | FIFO_ACCEL_EN
	{ FIFO_CONFIG2,         0, 0 }, // FIFO_WM[7:0] set at runtime
	{ FIFO_CONFIG3,         0, 0 }, // FIFO_WM[11:8] set at runtime
	{ INT_CONFIG0,          CLEAR_ON_FIFO_READ, 0 },
	{ INT_SOURCE0,          FIFO_THS_INT1_EN, 0 },
};

#define size_register_bank1_cfg 1
static register_bank1_config icm42688_register_bank1_cfg[size_register_bank1_cfg] = {
	// Register                              | Set bits, Clear bits
	{ GYRO_CONFIG_STATIC2,  GYRO_AAF_DIS | GYRO_NF_DIS, 0 },
};

#define size_register_bank2_cfg 1
static register_bank2_config icm42688_register_bank2_cfg[size_register_bank2_cfg] = {
	// Register                              | Set bits, Clear bits
	{ ACCEL_CONFIG_STATIC2, ACCEL_AAF_DIS, 0 },
};

typedef enum GyroRange
{
  GYRO_RANGE_15_625DPS,
  GYRO_RANGE_31_25DPS,
  GYRO_RANGE_62_5DPS,
  GYRO_RANGE_125DPS,
  GYRO_RANGE_250DPS,
  GYRO_RANGE_500DPS,
  GYRO_RANGE_1000DPS,
  GYRO_RANGE_2000DPS
} sensor_gyro_range;

typedef enum AccelRange
{
  ACCEL_RANGE_2G,
  ACCEL_RANGE_4G,
  ACCEL_RANGE_8G,
  ACCEL_RANGE_16G
} sensor_accel_range;



/* Main Functions */

// sensor init function.
// if sensor id is wrong, it is stuck in while.
extern uint8_t icm42688_init(void);

extern bool icm42688_configure();

// 16 bits ADC value. raw data.
extern void icm42688_gyro_read(axises* data);
extern void icm42688_accel_read(axises* data);


extern void icm42688_gyro_read_raw(uint8_t* data);
extern void icm42688_accel_read_raw(uint8_t* data);
extern bool ak09918_mag_read_raw(uint8_t* data);

// Convert 16 bits ADC value to their unit.
extern void icm42688_gyro_read_dps(axises* data);
extern void icm42688_accel_read_g(axises* data);
extern void icm42688_enable_data_ready_interrupt(void);


/* Sub Functions */
extern bool icm42688_who_am_i();


extern void icm42688_device_reset();
extern bool icm42688_device_reset_done();


extern void icm42688_wakeup();
extern void icm42688_sleep();

extern void icm42688_clock_source(uint8_t source);

extern void icm42688_gyro_full_scale_select(gyro_full_scale full_scale);
extern void icm42688_gyro_odr_select(odr odr);
extern void icm42688_accel_full_scale_select(accel_full_scale full_scale);
extern void icm42688_accel_odr_select(odr odr);

extern bool icm42688_register_check(userbank ub, uint8_t reg, uint8_t setbits, uint8_t clearbits);
extern void icm42688_register_set_and_clear_bits(userbank ub, uint8_t reg, uint8_t setbits, uint8_t clearbits);

extern bool process_sensor(void);
extern void calculate_IMU_error(void);

// Using static keyword force the compiler to inline this function, which makes the program compile successfully.
static inline double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_ICM42688P_H_ */
