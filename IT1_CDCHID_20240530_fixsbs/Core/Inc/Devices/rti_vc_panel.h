/******************************************************************************
* (c) COPYRIGHT 2017 RAONTECH, Inc. ALL RIGHTS RESERVED.                      *
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

// Panel 

#ifndef __RTI_VC_PANEL_H__
#define __RTI_VC_PANEL_H__

#include "rti_vc_common.h"

#ifdef __cplusplus
extern "C"{
#endif

// for RDP551F, RDP502H and RDP370F
// Must be odd number.
#define NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0			9

#if ((NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0 & 0x1) == 0)
	#error "Must be an odd number of raw codes!"
#endif

typedef struct
{
	/* Current User requested brightness (0 - max_brightness) */
	int curr_brightness;

	/* Maximal value for brightness (read-only) */
	int max_brightness;
} VC_BACKLIGHT_INFO_T;

typedef struct {
	BOOL_T	otp_applied;

	int		last_tsensor_code; /* The last code of temperature sensor */
	S32_T	last_temperature;
} VC_TEMPERATURE_STATUS_T;

typedef struct _VC_PANEL_DEV_INFO_T
{
	VC_TEMPERATURE_STATUS_T temperature_status[MAX_NUM_VC_PANEL_PORT];

	VC_LED_CURRENT_INFO_T led_current_info[MAX_NUM_VC_PANEL_PORT];

	int (*prepare_panel)(struct _VC_PANEL_DEV_INFO_T *dev);

	int (*get_temperature)(int panel_port, int *code, S32_T *temperature);

	int (*get_otp_summary)(int port, char *summary_buf);
	
	int (*set_led_current)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
					E_VC_LED_CTRL_COLOR_T color,
					VC_LED_CURRENT_INFO_T new_current[MAX_NUM_VC_PANEL_PORT]);

	int (*get_led_current)(struct _VC_PANEL_DEV_INFO_T *dev,
							E_VC_PANEL_CTRL_PORT_T ctrl_port,
							VC_LED_CURRENT_INFO_T *info);

	int (*get_led_level_range)(U8_T *min, U8_T *max);

	int (*get_device_id)(E_VC_PANEL_PORT_T port, U16_T *chip_id, U16_T *rev_num);

	int (*set_horizontal_pixel_shift)(int panel_port, int shift_val);
	int (*set_vertical_pixel_shift)(int panel_port, int shift_val);

	int (*get_horizontal_pixel_shift)(int panel_port, int *shift_val);
	int (*get_vertical_pixel_shift)(int panel_port, int *shift_val);

	VC_BACKLIGHT_INFO_T bl;

	// Function interfaces to mapping the native-driver functions.
	void (*set_brightness)(int brightness);
	int (*get_brightness)(void);

	int (*increase_brightness)(void);
	int (*decrease_brightness)(void);

} VC_PANEL_DEV_INFO_T;

typedef struct
{
	VC_PANEL_DEV_INFO_T dev;

} VC_PANEL_CB_T;

extern int rdp550f_f_attached;

S32_T rdp_compensate_temperature(S32_T raw_code[], U8_T num_raw_code);

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_PANEL_H__ */

