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

#ifndef __RTI_VC_RDC_H__
#define __RTI_VC_RDC_H__

#include "rti_vc_api.h"
#include "rti_vc_common.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
* Number of horizontal scan line.
*/
#if defined(CFG_PANEL_RDP501H)
	#define NUM_H_LINE_PIXEL		1280
	#define NUM_V_LINE_PIXEL		720
	#define NUM_LVDS_CHANNEL		0 // dummy

#elif defined(CFG_PANEL_RDP360H)
	#define NUM_H_LINE_PIXEL		1280
	#define NUM_V_LINE_PIXEL		720
	#define NUM_LVDS_CHANNEL		4

#elif defined(CFG_PANEL_RDP550F)
	#define NUM_H_LINE_PIXEL		1920
	#define NUM_V_LINE_PIXEL		1080
	#define NUM_LVDS_CHANNEL		8

#elif defined(CFG_PANEL_RDP700Q)
	#define NUM_H_LINE_PIXEL		1920
	#define NUM_V_LINE_PIXEL		1080
	#define NUM_LVDS_CHANNEL		8

#elif defined(CFG_PANEL_RDP551F)
	#define NUM_H_LINE_PIXEL		1920
	#define NUM_V_LINE_PIXEL		1080
	#define NUM_LVDS_CHANNEL		8

#elif defined(CFG_PANEL_RDP502H)
	#define NUM_H_LINE_PIXEL		1280
	#define NUM_V_LINE_PIXEL		720
	#define NUM_LVDS_CHANNEL		4

#elif defined(CFG_PANEL_RDP370F)
	#define NUM_H_LINE_PIXEL		1920
	#define NUM_V_LINE_PIXEL		1080
	#define NUM_LVDS_CHANNEL		8

#elif defined(CFG_PANEL_RDP250H)
	#define NUM_H_LINE_PIXEL		1280
	#define NUM_V_LINE_PIXEL		720
	#define NUM_LVDS_CHANNEL		4

#else
	#error "Code not present"
#endif


#define MAX_VC_FLASH_SECTOR_SIZE		0x1000

#define ALIGN_FLASH_SECTOR(addr, secotr_size)\
					((addr) & ~((secotr_size) - 1))

#if defined(CFG_MDC_RDC200) || defined(CFG_MDC_RDC200A)

#define NUM_RDC_TEXT_DRAWER		4 // number of actual drawer

typedef struct {
	U32_T pos_x; /* Previous x position*/
	U32_T pos_y;
	U32_T font_addr;
	U32_T palette_addr;
	BOOL_T enabled;
	BOOL_T showed;
	E_VC_OSD_TEXT_MAG_T mag;
	E_VC_OSD_ALPHA_T font_alpha;
	E_VC_OSD_ALPHA_T back_alpha;
	U8_T font_color_idx;
	U8_T back_color_idx;
	E_VC_OSD_DIMENSTION_T d; // dimension
	U8_T drawer_id;
} VC_TEXT_DRAWER_CB;

typedef struct {
	U32_T pos_x; /* Previous x position*/
	U32_T pos_y;
	U32_T bmp_addr;
	BOOL_T enabled;
	BOOL_T showed;
	E_VC_OSD_BITMAP_MAG_T mag;
	E_VC_OSD_ALPHA_T alpha;	
	E_VC_OSD_DIMENSTION_T d; // dimension
	U8_T drawer_id;
} VC_BITMAP_DRAWER_CB;
#endif

typedef struct _VC_RDC_DEV_INFO_T
{
	U8_T min_led_duty;
	U8_T max_led_duty;

	U8_T min_frame_rate;
	U8_T max_frame_rate;

	U8_T led_duty;
	U8_T frame_rate_hz;
	U32_T lvds_rate_bps;

	int (*power_on)(void);
	int (*power_off)(void);
	int (*suspend)(void);
	int (*resume)(void);

	int (*get_device_id)(U16_T *chip_id, U16_T *rev_num);

	int (*get_fw_version)(U8_T *major_ver, U8_T *minor_ver, U8_T *rel_ver);
	int (*poweron_panel)(BOOL_T on);
	int (*prepare_controller)(struct _VC_RDC_DEV_INFO_T *dev);
	
	int (*shift_horizontal_pixel)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
									E_VC_SHIFT_HORIZONTAL_T h_direction);	
	int (*shift_vertical_pixel)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
								E_VC_SHIFT_VERTICAL_T v_direction);

	int (*request_led_brightness_mode)(E_VC_RDC_INDEX_T rdc_idx,
										E_VC_PANEL_CTRL_PORT_T ctrl_port,
										U8_T brightness);
	int (*request_led_color_mode)(E_VC_RDC_INDEX_T rdc_idx,
									E_VC_PANEL_CTRL_PORT_T ctrl_port,
									VC_LED_CTRL_INFO_T *led_ctrl_info);
	int (*confirm_led_control)(E_VC_RDC_INDEX_T rdc_idx);

	int (*enable_test_pattern)(BOOL_T en);
	int (*generate_test_pattern)(U8_T r_val, U8_T g_val, U8_T b_val);

	int (*set_image_display_format)(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
									BOOL_T swap_image_3d_mode);

	VC_LED_DUTY_INFO_T led_duty_info[MAX_NUM_VC_PANEL_PORT];

	void (*fill_step_value)(VC_LED_DUTY_INFO_T *new_duty,
							E_VC_LED_CTRL_COLOR_T color, BOOL_T up,
							VC_LED_DUTY_INFO_T *dev_duty);

	int (*get_led_duty)(struct _VC_RDC_DEV_INFO_T *dev,
						E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T info[MAX_NUM_VC_PANEL_PORT]);

	int (*set_led_duty)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						E_VC_LED_CTRL_COLOR_T color,
						VC_LED_DUTY_INFO_T new_duty[MAX_NUM_VC_PANEL_PORT]);

	S32_T frame_rate;
	S32_T (*step_frame_rate)(struct _VC_RDC_DEV_INFO_T *dev, BOOL_T up);	
	S32_T (*get_frame_rate)(struct _VC_RDC_DEV_INFO_T *dev);

	int (*get_output_resolution)(U32_T *width, U32_T *height);

	int (*enable_gamma_correction)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
									BOOL_T enable,
									UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT]);

#if defined(CFG_MDC_RDC200) || defined(CFG_MDC_RDC200A)
	int (*enable_video_sync_interrupt)(void);
	int (*disable_video_sync_interrupt)(void);
	
	int (*request_atw_shift_pixel)(E_VC_PANEL_CTRL_PORT_T ctrl_port,
				RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T info[MAX_NUM_VC_PANEL_PORT]);
				
	int (*change_input_display_interface)(E_VC_INPUT_DISPLAY_IF_T dif,
											VC_INPUT_DISPLAY_IF_CFG *cfg);

	int (*enable_distortion_correction)(E_VC_PANEL_CTRL_PORT_T ctrl_port, BOOL_T dc_en,
								UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT], BOOL_T ca_en);

	int (*update_lut)(U32_T faddr, U32_T size, U8_T id);

	int (*update_gamma_lut)(U8_T port, U8_T lut_offset);

	int (*request_cokoa_generation)(VC_COKOA_PARAM_T *param);

	int (*change_input_resolution)(VC_INPUT_RES_T res, BOOL_T enable_3D,
									U32_T h_blank, U32_T v_blank,
									U32_T frame_rate);

	int (*change_input_resolution2)(VC_INPUT_RES_T res, BOOL_T enable_3D,
									U32_T pclk_hz);

	int (*draw_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb, U32_T pos_x, U32_T pos_y,
							U32_T bmp_addr, E_VC_OSD_BITMAP_MAG_T mag,
							E_VC_OSD_ALPHA_T alpha,
							E_VC_OSD_DIMENSTION_T d);

	int (*move_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb, U32_T mov_x, U32_T mov_y);

	int (*show_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb);

	int (*hide_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb);

	int (*change_property_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb,
										E_VC_OSD_BITMAP_MAG_T mag,
										E_VC_OSD_ALPHA_T alpha);

	int (*change_dimension_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb,
										E_VC_OSD_DIMENSTION_T d);

	int (*disable_bitmap_osd)(VC_BITMAP_DRAWER_CB *drawer_cb);

	/**
	* Text drawer interfaces
	*/

	// if font_addr is 0xFFFFFFFF, the drawer use the default font.
	int (*draw_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb, U32_T pos_x, U32_T pos_y,
							U32_T font_addr, U32_T palette_addr,
							E_VC_OSD_TEXT_MAG_T mag,
							E_VC_OSD_ALPHA_T font_alpha,
							E_VC_OSD_ALPHA_T back_alpha,
							U8_T font_color_idx, U8_T back_color_idx,
							E_VC_OSD_DIMENSTION_T d,
							const char *string);

	int (*move_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb, U32_T mov_x, U32_T mov_y);

	int (*show_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb);

	int (*hide_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb);

	int (*change_property_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb,
									E_VC_OSD_TEXT_MAG_T mag,
									E_VC_OSD_ALPHA_T font_alpha,
									E_VC_OSD_ALPHA_T back_alpha);

	int (*change_dimension_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb,
										E_VC_OSD_DIMENSTION_T d);

	int (*disable_text_osd)(VC_TEXT_DRAWER_CB *drawer_cb);

	// For projector product.
	int (*change_laser_sparkling_pwm)(U16_T on_time_period, U16_T off_time_period);
	void (*disable_laser_sparkling)(void);
#endif

	/*** FLASH interface ***/
	int sector_size_shift; /* The shift value for the size of sector. */
	int (*flash_prepare)(void);
	int (*flash_erase_sector)(U32_T sector_addr);
	int (*flash_erase_chip)(void);
	int (*flash_write)(U32_T addr, const U8_T *buf, UINT_T len);
	int (*flash_read)(U32_T addr, U8_T *buf, UINT_T len);
	int (*flash_finish)(void);
} VC_RDC_DEV_INFO_T;


typedef struct
{
	BOOL_T test_pattern_enabled;

	VC_RDC_DEV_INFO_T dev;

#if defined(CFG_MDC_RDC200)
	VC_BITMAP_DRAWER_CB bmp_drawer_cb[NUM_VC_BMP_DRAWER];
	U8_T bmp_drawer_id_bits;

	VC_TEXT_DRAWER_CB text_drawer_cb[NUM_RDC_TEXT_DRAWER];
	U8_T text_drawer_id_bits;
#endif

#if defined(CFG_MDC_RDC200A)
	VC_BITMAP_DRAWER_CB bmp_drawer_cb[NUM_VC_BMP_DRAWER];
	U8_T bmp_drawer_id_bits;

	VC_TEXT_DRAWER_CB text_drawer_cb[NUM_RDC_TEXT_DRAWER];
	U8_T text_drawer_id_bits;
#endif
} VC_RDC_CB_T;

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_RDC_H__ */

