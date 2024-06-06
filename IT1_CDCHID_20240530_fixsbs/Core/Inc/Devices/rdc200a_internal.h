/******************************************************************************
* (c) COPYRIGHT 2019 RAONTECH, Inc. ALL RIGHTS RESERVED.                      *
*                                                                             *
* This software is the property of RAONTECH and is furnished under license    *
* by RAONTECH.                                                                *
* This software may be used only in accordance with the terms of said license.*
* This copyright noitce may not be removed, modified or obliterated without   *
* the prior written permission of RAONTECH, Inc.                              *
*                                                                             *
* This software may not be copied, transmitted, provided to or otherwise      *
* made available to any other person, company, corporation or other entity    *
* except as specified in the terms of said license.                           *
*                                                                             *
* No right, title, ownership or other interest in the software is hereby      *
* granted or transferred.                                                     *
*                                                                             *
* The information contained herein is subject to change without notice        *
* and should not be construed as a commitment by RAONTECH, Inc.               *
******************************************************************************/

#ifndef __RDC200A_INTERNAL_H__
#define __RDC200A_INTERNAL_H__

#include "main.h"
#include <math.h>
#include "rti_vc_rdc.h"
#include "rti_vc_panel.h"
#include "rdc200a_flash_partitions.h"

#ifdef __cplusplus
extern "C"{
#endif

#define RDC200A_REG_BURST_SET(buf, size)\
	rtiVC_WriteBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0,\
						vc_rdc_i2c_slave_addr, buf, size)


#define RDC200A_REG_BURST_GET(w_buf, w_size, r_buf, r_size)\
	rtiVC_ReadBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr,\
							w_buf, w_size, r_buf, r_size)

#define VC_LED_DUTY_DIVIDER		1000

#define VC_LED_DUTY_TO_PERCENTAGE(duty)\
	((double)(duty) / (double)VC_LED_DUTY_DIVIDER)

#define VC_FRAME_RATE_DIVIDER		1000

#define VC_FRAME_RATE_TO_HZ(frame_rate)\
	((double)(frame_rate) / (double)VC_FRAME_RATE_DIVIDER)

/*
 * Defines the step of frame rate in KHz.
 */
#define _CFG_FRAME_RATE_CTRL_STEP		1

/*
 * Defines the step of LED duty.
 */
#define _CFG_LED_DUTY_CTRL_STEP		0.5


#define RDC200A_XTAL_FREQ_MHz	32 //32000000

#define RDC200A_STEP_DOWN_LED_DUTY_VALUE(led_duty)\
	((led_duty) - (_CFG_LED_DUTY_CTRL_STEP * VC_LED_DUTY_DIVIDER))

#define RDC200A_STEP_UP_LED_DUTY_VALUE(led_duty)\
	((led_duty) + (_CFG_LED_DUTY_CTRL_STEP * VC_LED_DUTY_DIVIDER))


#define RDC200A_STEP_DOWN_FRAME_RATE_VALUE(frame_rate)\
	((frame_rate) - (_CFG_FRAME_RATE_CTRL_STEP * VC_FRAME_RATE_DIVIDER))

#define RDC200A_STEP_UP_FRAME_RATE_VALUE(frame_rate)\
	((frame_rate) + (_CFG_FRAME_RATE_CTRL_STEP * VC_FRAME_RATE_DIVIDER))


// RDC register 0x0015
#define TX1_PANEL_EN		0x20
#define TX0_PANEL_EN		0x10
#define PDRV_TX1_AUTO_ON	0x08
#define PDRV_TX0_AUTO_ON	0x04
#define PDRV_TX1_AUTO_EN	0x02
#define PDRV_TX0_AUTO_EN	0x01

#define RDC_PANEL_EN_AUTO_EN_MASK\
	(PDRV_TX0_AUTO_EN|PDRV_TX1_AUTO_EN|TX0_PANEL_EN|TX1_PANEL_EN)

typedef struct {
	U8_T RED_POL_H;
	U8_T GREEN_POL_H;
	U8_T BLUE_POL_H;
	U8_T RED_POL_L;
	U8_T GREEN_POL_L;
	U8_T BLUE_POL_L;	
} RDC200A_GAMMA_LUT_ID_INFO_T;

typedef struct {
	U32_T h_size;
	U32_T v_size;
} RDC200A_OSD_MAG_SIZE_INFO_T;

#define RDC200A_MAX_NUM_FONT_CHAR_H	32
#define RDC200A_MAX_NUM_FONT_CHAR_V	8
#define RDC200A_MAX_NUM_FONT_CHAR_PER_BOX\
		(RDC200A_MAX_NUM_FONT_CHAR_H*RDC200A_MAX_NUM_FONT_CHAR_V)
		
#define RDC200A_FONT_BOX_WITDH	256
#define RDC200A_FONT_BOX_HEGIHT	128

#define RDC200A_BMP_RES_WITDH	160
#define RDC200A_BMP_RES_HEGIHT	96

/*
	The first 2d font pos_x is 0x0358. FONT0_POS_X<11:8>
	Increment offset: drawer_id * 4
*/
#define RDC200A_3D_FONT_POS_Y_MSB_REG_ADDR(drawer_id)\
	(0x036A + ((drawer_id) << 2))

#define RDC200A_3D_FONT_POS_X_MSB_REG_ADDR(drawer_id)\
	(0x0368 + ((drawer_id) << 2))

#define RDC200A_2D_FONT_POS_Y_MSB_REG_ADDR(drawer_id)\
	(0x035A + ((drawer_id) << 2))

#define RDC200A_2D_FONT_POS_X_MSB_REG_ADDR(drawer_id)\
	(0x0358 + ((drawer_id) << 2))

#define RDC200A_FONT_ALPHA_REG_ADDR(drawer_id)\
	(0x0351 + ((drawer_id) << 1))
	
#define RDC200A_FONT_MAG_REG_ADDR(drawer_id)\
	(0x0350 + ((drawer_id) << 1))

/*
	The first 2d bmp pos_x is 0x0326. BMP0_POS_X<11:8>
	Increment offset: drawer_id * 4
*/
#define RDC200A_2D_BMP_POS_X_MSB_REG_ADDR(drawer_id)\
	(0x0326 + ((drawer_id) << 2))

#define RDC200A_2D_BMP_POS_Y_MSB_REG_ADDR(drawer_id)\
	(0x0328 + ((drawer_id) << 2))

#define RDC200A_3D_BMP_POS_X_MSB_REG_ADDR(drawer_id)\
	(0x0336 + ((drawer_id) << 2))

#define RDC200A_3D_BMP_POS_Y_MSB_REG_ADDR(drawer_id)\
	(0x0338 + ((drawer_id) << 2))

#define RDC200A_BMP_MAG_REG_ADDR(drawer_id)\
	(0x0320 + (drawer_id))

#define VC_BMP_MAG_N_Y_MASK		0xC0 // reg[7:6]. BMP0_MAG_N_Y<1:0>
#define VC_BMP_MAG_I_Y_MASK		0x30 // reg[5:4]. BMP0_MAG_I_Y<1:0>
#define VC_BMP_MAG_N_X_MASK		0x0C // reg[3:2]. BMP0_MAG_N_X<1:0>
#define VC_BMP_MAG_I_X_MASK		0x03 // reg[1:0]. BMP0_MAG_I_X<1:0>

// Output resolution of register
typedef struct {
	U16_T h_size;
	U16_T v_size;	
} RDC200A_RESOLUTION_INFO_T;

extern const RDC200A_RESOLUTION_INFO_T rdc200a_output_res[];
    
// RDC200A register 0x0014
#define FBF_3D_EN		0x80
#define FBF_EX_SIG		0x40
#define TAB3D_EN		0x20
#define SBS_LL_3D_EN	0x10
#define SBS3D_EN		0x08
//#define UNUSED		0x04
#define PANEL_SWAP		0x02
//DUAL_BUF_EN			0x01

#define LED_P0_ON_R_msb(port)	(0x0853 + (port)*0x100)
#define LED_P0_ON_R_lsb(port)	(0x0854 + (port)*0x100)
#define LED_P0_OFF_R_msb(port)	(0x0855 + (port)*0x100)
#define LED_P0_OFF_R_lsb(port)	(0x0856 + (port)*0x100)

#define LED_P0_ON_G_msb(port)	(0x085B + (port)*0x100)
#define LED_P0_ON_G_lsb(port)	(0x085C + (port)*0x100)
#define LED_P0_OFF_G_msb(port)	(0x085D + (port)*0x100)
#define LED_P0_OFF_G_lsb(port)	(0x085E + (port)*0x100)

#define LED_P0_ON_B_msb(port)	(0x0863 + (port)*0x100)
#define LED_P0_ON_B_lsb(port)	(0x0864 + (port)*0x100)
#define LED_P0_OFF_B_msb(port)	(0x0865 + (port)*0x100)
#define LED_P0_OFF_B_lsb(port)	(0x0866 + (port)*0x100)

#define LED_P1_ON_R_msb(port)    (0x0857 + (port)*0x100)
#define LED_P1_ON_R_lsb(port)    (0x0858 + (port)*0x100)
#define LED_P1_OFF_R_msb(port)    (0x0859 + (port)*0x100)
#define LED_P1_OFF_R_lsb(port)    (0x085A + (port)*0x100)
    
#define LED_P1_ON_G_msb(port)    (0x085F + (port)*0x100)
#define LED_P1_ON_G_lsb(port)    (0x0860 + (port)*0x100)
#define LED_P1_OFF_G_msb(port)    (0x0861 + (port)*0x100)
#define LED_P1_OFF_G_lsb(port)    (0x0862 + (port)*0x100)
    
#define LED_P1_ON_B_msb(port)    (0x0867 + (port)*0x100)
#define LED_P1_ON_B_lsb(port)    (0x0868 + (port)*0x100)
#define LED_P1_OFF_B_msb(port)    (0x0869 + (port)*0x100)
#define LED_P1_OFF_B_lsb(port)    (0x086A + (port)*0x100)
    
#define DISPLAY_HOLD_R(port)	(0x0823 + (port)*0x100)
#define DISPLAY_HOLD_G(port)	(0x0824 + (port)*0x100)
#define DISPLAY_HOLD_B(port)	(0x0825 + (port)*0x100)

#ifdef __cplusplus
}
#endif

#endif /* __RDC200A_INTERNAL_H__ */

