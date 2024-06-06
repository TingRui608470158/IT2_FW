#ifndef CM32183E_H_
#define CM32183E_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <stdint.h>
#include "main.h"

// CM32183E slave address can be 0x20 or 0x90, determined by pin ADDR configuration
#define CM32183E_ADDR_ALS  0x52  // 7-bit: 0x29
#define CM32183E_ADDR_7bit_ALS  0x29

// CM32183E registers
#define ALS_CONF  0x00
#define ALS_THDH  0x01
#define ALS_THDL  0x02
#define ALS_PSM   0x03
#define ALS_DATA  0x04
#define ALS_STATUS  0x06

// CM32183E command code 00 register bits
#define CM32183E_ALS_CONF_SD      0x0001
#define CM32183E_ALS_CONF_INT_EN    0x0002
#define CM32183E_ALS_CONF_PERS_MASK    0x0030
#define CM32183E_ALS_CONF_PERS_1    0x0000
#define CM32183E_ALS_CONF_PERS_2    0x0010
#define CM32183E_ALS_CONF_PERS_4    0x0020
#define CM32183E_ALS_CONF_PERS_8    0x0030
#define CM32183E_ALS_CONF_IT_MASK    0x00C0
#define CM32183E_ALS_CONF_IT_100MS    0x0000
#define CM32183E_ALS_CONF_IT_200MS    0x0040
#define CM32183E_ALS_CONF_IT_400MS    0x0080
#define CM32183E_ALS_CONF_IT_800MS    0x00C0
#define CM32183E_ALS_CONF_SM_MASK    0x1800
#define CM32183E_ALS_CONF_SM_x1      0x0000
#define CM32183E_ALS_CONF_SM_x2      0x0800
#define CM32183E_ALS_CONF_SM_x1_8    0x1000
#define CM32183E_ALS_CONF_SM_x1_4    0x1800
// JOSEPH: add gain @20220531
#define CM32183E_ALS_CONF_GAIN_x1      0x0000
#define CM32183E_ALS_CONF_GAIN_x2      0x0400

#define CM32183E_ALS_CONF_DEFAULT    0x0000


//#define CM32183E_I2C1        I2C1_BASE

extern bool write_ambient_light_sensor_complete_flag;
extern uint16_t als_code;
extern void initialize_CM32183E(uint16_t als_code);
extern uint16_t read_als_data(void);
// JOSEPH: add threshold_high and threshold_low to global @20220531
extern uint32_t threshold_high, threshold_low;
extern void set_als_int_threshold(uint16_t als_code);
extern void clear_interrupt(void);
extern void enable_sensor(void);
extern void disable_sensor(void);
extern void enable_interrupt(void);
extern void disable_interrupt(void);
extern uint16_t CM32183E_read_word(uint8_t command);
extern void CM32183E_write_word(uint8_t command, uint16_t val);
extern float read_CM32183E(void);

#ifdef __cplusplus
}
#endif

#endif /* CM32183E_H_ */
