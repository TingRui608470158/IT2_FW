#ifndef RDC200A_H_
#define RDC200A_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <stdint.h>
#include "rti_vc_types.h"

#define RDC200A_ADDR			0x94  // 7-bit: 0x4A
#define RDC200A_7bit_ADDR		0x4A
/*
 * I2C slave address for RDP. (7 bit)
 */
//#define _CFG_RDP_SLAVE_ADDR	    0x38 // RDP360H
//#define _CFG_RDP_SLAVE_ADDR	    0x39 // RDP550F-e
#define _CFG_RDP_SLAVE_ADDR	    0x18 // RDP700Q / RDP550F-f / RDP551F / RDP502H / RDP370F

typedef enum {
    RDC_VIDEO_COORD_INPUT_LVDS = 0,
    RDC_VIDEO_COORD_INPUT_MIPI = 1
} E_RDC_VIDEO_COORD_INPUT_T;

#if 1 // betta added
extern void set_rdc200a_output_60Hz(void);
extern void set_rdc200a_output_90Hz(void);
extern void set_rdc200a_output_120Hz(void);
#endif
extern void rdc200a_sbs_on(void);
extern void rdc200a_sbs_off(void);
extern int rdc200a_power_on(void);
extern int rdc200a_power_off(void);
extern bool rdc200a_setupOutputpin(void);
extern int display_resume(unsigned char poweroff, E_RDC_VIDEO_COORD_INPUT_T iface);
extern int display_suspend(unsigned char poweroff);
extern uint8_t RDC_REG_GET2(uint16_t reg);
extern void RDC_REG_SET2(uint16_t reg, uint8_t val);
extern uint32_t RDC_REG_GET16(uint16_t reg);
extern uint32_t RDC_REG_GET32(uint16_t reg);
//extern int rdc200a_set_image_display_format(void);

extern int rdc200a_poweron_panel(BOOL_T on);

#ifdef __cplusplus
}
#endif

#endif /* RDC200A_H_ */
