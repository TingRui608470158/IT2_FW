#ifndef INC_AK09918_H_
#define INC_AK09918_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "imu.h"

typedef enum
{
  AK09918_REG_WIA1 = 0x00,
  AK09918_REG_WIA2,
  AK09918_REG_RSV1,
  AK09918_REG_RSV2, // Reserved register.
  // discontinuity
  AK09918_REG_ST1 = 0x10,	// DataStatus 1
  AK09918_REG_HXL,			// X-axis data
  AK09918_REG_HXH,
  AK09918_REG_HYL,			// Y-axis data
  AK09918_REG_HYH,
  AK09918_REG_HZL,			// Z-axis data
  AK09918_REG_HZH,
  // discontinuity
  AK09918_REG_ST2 = 0x18,	// DataStatus 2
  // discontinuity
  AK09918_REG_CNTL2 = 0x31,	// Control settings
  AK09918_REG_CNTL3,		// Control settings
} AK09918_Reg_Addr_e;

typedef struct
{
  uint8_t WIA1;
} AK09918_WIA1_Reg_t;

typedef struct
{
  uint8_t WIA2;
} AK09918_WIA2_Reg_t;

typedef struct
{
  uint8_t DRDY : 1;			// Data Ready
  uint8_t DOR : 1;			// Data Over Run
  uint8_t reserved_0 : 6;
} AK09918_ST1_Reg_t;

typedef struct
{
  uint8_t reserved_0 : 3;
  uint8_t HOFL : 1;			// Sensor Over Flow
  uint8_t reserved_1 : 4;
} AK09918_ST2_Reg_t;

typedef struct
{
  uint8_t MODE : 5;
  uint8_t reserved_0 : 3;
} AK09918_CNTL2_Reg_t;

typedef struct
{
  uint8_t SRST : 1;			// Soft Reset
  uint8_t reserved_0 : 7;
} AK09918_CNTL3_Reg_t;

/*
 * AK09918 has following seven operation modes:
 * (1) Power-down mode: AK09918 doesn't measure
 * (2) Single measurement mode
 * (3) Continuous measurement mode 1: 10Hz, measure 10 times per second,
 * (4) Continuous measurement mode 2: 20Hz, measure 20 times per second,
 * (5) Continuous measurement mode 3: 50Hz, measure 50 times per second,
 * (6) Continuous measurement mode 4: 100Hz, measure 100 times per second,
 * (7) Self-test mode
 */
typedef enum
{
  AK09918_mode_power_down = 0x00,
  AK09918_mode_single = (0x01 << 0),
  AK09918_mode_cont_10hz = (0x01 << 1),
  AK09918_mode_cont_20hz = (0x02 << 1),
  AK09918_mode_cont_50hz = (0x03 << 1),
  AK09918_mode_cont_100hz = (0x04 << 1),
  AK09918_mode_self_test = (0x01 << 4),
} AK09918_mode_e;

/* AK09918 Registers */
#define AK09918C_ID						0x0C
#define MAG_SLAVE_ADDR 					0x0C	// I2C address (Can't be changed)
#define AK09918C_W	0
#define AK09918C_R	1

extern uint8_t ak09918_init();
extern bool ak09918_mag_read(axises* data);
extern bool ak09918_mag_read_uT(axises* data);
extern bool ak09918_who_am_i();
extern void ak09918_soft_reset();
extern void ak09918_operation_mode_setting(AK09918_mode_e mode);
extern bool ak09918_register_check(uint8_t reg, uint8_t setbits, uint8_t clearbits);
extern void ak09918_register_set_and_clear_bits(uint8_t reg, uint8_t setbits, uint8_t clearbits);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_AK09918_H_ */
