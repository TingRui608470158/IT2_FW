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

#include <math.h>
#include "rti_vc_panel.h"

#if defined(CFG_PANEL_RDP551F)

#define RDP551F_OTP_INDICATOR_TSENSOR	0x40 // Temperature sensor
#define RDP551F_OTP_INDICATOR_LVDS		0x20 // LVDS res
#define RDP551F_OTP_INDICATOR_TEMP_V2	0x01 // Temperature sensor V2 flag

#define RDP551F_OTP_INDICATOR_GAMMA		0x20 // Gamma
#define RDP551F_OTP_INDICATOR_VCOM		0x10
#define RDP551F_OTP_INDICATOR_LED		0x08 // LED driver current
#define RDP551F_OTP_INDICATOR_LDO		0x04 // LED driver POL1
#define RDP551F_OTP_INDICATOR_OSC		0x02 // Oscillator
#define RDP551F_OTP_INDICATOR_BIAS		0x01


#define MAX_RDP551F_LED_CURRENT_VALUE	0x3F // Used only [7:2] to elminate unmoved step.
//#define MAX_RDP551F_LED_CURRENT_VALUE	0xFF // Used only [7:2] to elminate unmoved step.
#define MIN_RDP551F_LED_CURRENT_VALUE	0x00

typedef struct {
	U8_T Temp_V2;

	BOOL_T led_apply;
	char LED_CON; // POL0. Only R

	BOOL_T tsensor_apply;
	char  tsensor_otp;
} RDP551F_OTP_INFO_T;

static RDP551F_OTP_INFO_T rdp551f_otp[MAX_NUM_VC_PANEL_PORT];

/*
 * This function measures the temperature with the temperature sensor
 * on the panel and returns the calibrated code and the temperature in degrees celsius.
 *
 * code: temperature sensor code
 * temperature: temperature value in degree celsius.
 *
 * NOTE: The value of celsius temperature is calculated by multiplying by 1000
 *       to avoid floating point operations.
*/
static int rdp551f_get_temperature(int panel_port, int *code, S32_T *temperature)
{
	S32_T raw_code[NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0]; /* Raw temperature sensor code */
	S32_T comp_raw_code;
	S32_T raw_temp; /* Raw temperature in degree celsius */
	S32_T diff_code; /* Difference between raw code and otp code */
	S32_T caled_code; /* Calibrated code */
	S32_T caled_temp; /* Calibrated temperature */
	S32_T calibration_val = 0; /* CP calibration value */
	RDP551F_OTP_INFO_T *otp = &rdp551f_otp[panel_port];
	UINT_T i;

	for (i = 0; i < NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0; i++) {
		raw_code[i] = (S32_T)(RDP_REG_GET(panel_port, 0x0175) & 0x3) << 8; // TEMPERATURE<9:8>
		raw_code[i] |= (S32_T)RDP_REG_GET(panel_port, 0x0176); // TEMPERATURE<7:0>

		//DMSG("0x%04X\n", raw_code[i]);
	}

	comp_raw_code = rdp_compensate_temperature(raw_code,
								NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0);

	if (otp->tsensor_apply == TRUE)
		calibration_val = otp->tsensor_otp;

	caled_code = comp_raw_code - calibration_val;
	diff_code = caled_code - comp_raw_code;

	raw_temp = 30*1000 - (S32_T)(0.415*1000) * (comp_raw_code - 320);
	caled_temp = raw_temp - (S32_T)(1000*0.415) * diff_code;

	if (otp->Temp_V2 == 0)
		caled_temp += (S32_T)(4.15*1000);

	*code = caled_code;
	*temperature = caled_temp;

//	DMSG("P%d: tsensor_apply(%d) [raw_code(%d) => caled_code(%d)] [raw_temp(%d) => caled_temp(%d)]\n",
//			panel_port, otp->tsensor_apply, raw_code, caled_code, raw_temp, caled_temp);

	return 0;
}

/*
 Calculate 8-bit RDP LED_CON regisgter value.
 
 level: 8-bit UI level
*/
static U8_T rdp551f_calculate_led_otp(char otp_LED_CON, U8_T level)
{
	U32_T result;
	U8_T ret;
	U8_T b;
	
	// Multiply 10 to eliminate floating math.
	result = (U32_T)((1270 + (U32_T)(otp_LED_CON*10)) * (U32_T)(level*10)) / 2550;

	ret = result / 10;
	b = result % 10;
	if (b >= 5)
		ret++;

	return ret;
}

/*
 Calculate 6-bit UI level.

 panel_LED_CON: 8-bit LED_CON in panel register

 [I2C Input CODE] = (127 + [OTP]) / 255 x [UI CODE internal]

*/
static U8_T rdp551f_calculate_ui_level(char otp_LED_CON, U8_T reg_panel_LED)
{
	U8_T level = 0; // UI level (6-bit)
	U32_T temp;
	U8_T b;

	//if (otp_LED_CON > -127) {
		temp = ((U32_T)reg_panel_LED*10 * ( U32_T)2550) / (1270 + (S32_T)(otp_LED_CON*10));
		temp >>= 2; // Divide by 4. (for 6-bit UI level)

		level = temp / 10;
		b = temp % 10;
		if (b >= 5)
			level++;
	//}

	return level;
}

static void rdp551f_read_led_current_from_reg(E_VC_PANEL_PORT_T port,										
										VC_LED_CURRENT_INFO_T *info)
{
	U8_T R, G, B;
	
	R = RDP_REG_GET(port, 0x10E);
	G = RDP_REG_GET(port, 0x10F);
	B = RDP_REG_GET(port, 0x110);

#if 0
	DMSG("P%d: 0x10E(0x%02X)\n", port, R);
	DMSG("P%d: 0x10F(0x%02X)\n", port, G);
	DMSG("P%d: 0x110(0x%02X)\n", port, B);
#endif

	if (rdp551f_otp[port].led_apply == TRUE) {
		info->R = rdp551f_calculate_ui_level(rdp551f_otp[port].LED_CON, R);
		info->G = rdp551f_calculate_ui_level(rdp551f_otp[port].LED_CON, G);
		info->B = rdp551f_calculate_ui_level(rdp551f_otp[port].LED_CON, B);
	}
	else {
		info->R = R >> 2; // Used only [7:2]. 4 step
		info->G = G >> 2; // Used only [7:2] 
		info->B = B >> 2; // Used only [7:2]
	}
}

static int rdp551f_get_led_level_range(U8_T *min, U8_T *max)
{
	*min = MIN_RDP551F_LED_CURRENT_VALUE;
	*max = MAX_RDP551F_LED_CURRENT_VALUE;

	return 0;
}

static int rdp551f_set_led_current(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							E_VC_LED_CTRL_COLOR_T color,
							VC_LED_CURRENT_INFO_T new_current[MAX_NUM_VC_PANEL_PORT])
{
	int i;
	U8_T val[MAX_NUM_VC_PANEL_PORT][3]; // Register value to be written.
	U8_T chk_port;
	VC_LED_CURRENT_INFO_T *current_ptr = NULL;
	
	// Calculate first!
	chk_port = (U8_T)ctrl_port;
	for (i = VC_PANEL_PORT_0; i < MAX_NUM_VC_PANEL_PORT; i++) {
		if (chk_port & 0x1) {
			current_ptr = &new_current[i];

			if (color & VC_LED_CTRL_COLOR_R) {
				if (current_ptr->R < MIN_RDP551F_LED_CURRENT_VALUE) {
					EMSG("R: Can't step to the down side anymore for port %d\n", i);
					//return -1;
					current_ptr->R = MIN_RDP551F_LED_CURRENT_VALUE;
				}

				if (current_ptr->R > MAX_RDP551F_LED_CURRENT_VALUE) {
					EMSG("R: Can't step to the up side anymore for port %d\n", i);
					current_ptr->R = MAX_RDP551F_LED_CURRENT_VALUE;
				}

				/* Make 8-bit level to write it into register. */
				val[i][0] = current_ptr->R << 2;

				if (rdp551f_otp[i].led_apply == TRUE)
					val[i][0] = rdp551f_calculate_led_otp(rdp551f_otp[i].LED_CON,
															val[i][0]);
			}

			if (color & VC_LED_CTRL_COLOR_G) {
				if (current_ptr->G < MIN_RDP551F_LED_CURRENT_VALUE) {
					EMSG("G: Can't step to the down side anymore for port %d\n", i);
					current_ptr->G = MIN_RDP551F_LED_CURRENT_VALUE;
				}

				if (current_ptr->G > MAX_RDP551F_LED_CURRENT_VALUE) {
					EMSG("G: Can't step to the up side anymore for port %d\n", i);
					current_ptr->G = MAX_RDP551F_LED_CURRENT_VALUE;
				}

				val[i][1] = current_ptr->G << 2;

				if (rdp551f_otp[i].led_apply == TRUE)
					val[i][1] = rdp551f_calculate_led_otp(rdp551f_otp[i].LED_CON,
															val[i][1]);
			}

			if (color & VC_LED_CTRL_COLOR_B) {
				if (current_ptr->B < MIN_RDP551F_LED_CURRENT_VALUE) {
					EMSG("B: Can't step to the down side anymore for port %d\n", i);
					current_ptr->B = MIN_RDP551F_LED_CURRENT_VALUE;
				}

				if (current_ptr->B > MAX_RDP551F_LED_CURRENT_VALUE) {
					EMSG("B: Can't step to the up side anymore for port %d\n", i);
					current_ptr->B = MAX_RDP551F_LED_CURRENT_VALUE;
				}

				val[i][2] = current_ptr->B << 2;

				if (rdp551f_otp[i].led_apply == TRUE)
					val[i][2] = rdp551f_calculate_led_otp(rdp551f_otp[i].LED_CON,
															val[i][2]);
			}
		}		
		chk_port >>= 1;
	}

	// Write register values.	
	if (color & VC_LED_CTRL_COLOR_R) {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0)
			RDP_REG_SET(VC_DEVICE_CH_RDC_RDP0, 0x10E, val[VC_PANEL_PORT_0][0]);
		if (ctrl_port & VC_PANEL_CTRL_PORT_1)
			RDP_REG_SET(VC_DEVICE_CH_RDP1, 0x10E, val[VC_PANEL_PORT_1][0]);
	}

	if (color & VC_LED_CTRL_COLOR_G) {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0)
			RDP_REG_SET(VC_DEVICE_CH_RDC_RDP0, 0x10F, val[VC_PANEL_PORT_0][1]);

		if (ctrl_port & VC_PANEL_CTRL_PORT_1)
			RDP_REG_SET(VC_DEVICE_CH_RDP1, 0x10F, val[VC_PANEL_PORT_1][1]);
	}
	
	if (color & VC_LED_CTRL_COLOR_B) {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0)
			RDP_REG_SET(VC_DEVICE_CH_RDC_RDP0, 0x110, val[VC_PANEL_PORT_0][2]);
		
		if (ctrl_port & VC_PANEL_CTRL_PORT_1)
			RDP_REG_SET(VC_DEVICE_CH_RDP1, 0x110, val[VC_PANEL_PORT_1][2]);
	}
	
#if 0
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		DMSG("P0: 0x10E(0x%02X)\n", RDP_REG_GET(0, 0x10E));
		DMSG("P0: 0x10F(0x%02X)\n", RDP_REG_GET(0, 0x10F));
		DMSG("P0: 0x110(0x%02X)\n", RDP_REG_GET(0, 0x110));
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		DMSG("P1: 0x10E(0x%02X)\n", RDP_REG_GET(1, 0x10E));
		DMSG("P1: 0x10F(0x%02X)\n", RDP_REG_GET(1, 0x10F));
		DMSG("P1: 0x110(0x%02X)\n", RDP_REG_GET(1, 0x110));
	}
#endif

	return 0;
}

static int rdp551f_get_led_current(struct _VC_PANEL_DEV_INFO_T *dev,
							E_VC_PANEL_CTRL_PORT_T ctrl_port,
							VC_LED_CURRENT_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
#if 1
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		rdp551f_read_led_current_from_reg(VC_PANEL_PORT_0, &info[VC_PANEL_PORT_0]);
		dev->led_current_info[VC_PANEL_PORT_0] = info[VC_PANEL_PORT_0];
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		rdp551f_read_led_current_from_reg(VC_PANEL_PORT_1, &info[VC_PANEL_PORT_1]);
		dev->led_current_info[VC_PANEL_PORT_1] = info[VC_PANEL_PORT_1];
	}

#else
	if (ctrl_port & VC_PANEL_CTRL_PORT_0)
		info[VC_PANEL_PORT_0] = dev->led_current_info[VC_PANEL_PORT_0];

	if (ctrl_port & VC_PANEL_CTRL_PORT_1)
		info[VC_PANEL_PORT_1] = dev->led_current_info[VC_PANEL_PORT_1];
#endif

	return 0;
}

int rdp551f_get_vertical_pixel_shift(int panel_port, int *shift_val)
{
	*shift_val = RDP_REG_GET(panel_port, 0x001B);

	return 0;
}

int rdp551f_get_horizontal_pixel_shift(int panel_port, int *shift_val)
{
	*shift_val = RDP_REG_GET(panel_port, 0x001C);

	return 0;
}

int rdp551f_set_vertical_pixel_shift(int panel_port, int shift_val)
{
    if (shift_val > 32)
        DMSG("Vertical pixel cropped\n");
		
	RDP_REG_SET(panel_port, 0x000A, 0x08);  // PIXEL RESET
	RDP_REG_SET(panel_port, 0x001B, shift_val);
	RDP_REG_SET(panel_port, 0x001D, 0x11); // BPM_STROBE, BPM_MODE
	VC_M_DELAY(1);
	RDP_REG_SET(panel_port, 0x000A, 0x00);  // PIXEL RESET

	return 0;
}

int rdp551f_set_horizontal_pixel_shift(int panel_port, int shift_val)
{
    if (shift_val > 144)
        DMSG("Horizotal pixel cropped\n");
		
	RDP_REG_SET(panel_port, 0x000A, 0x08);  // PIXEL RESET
	RDP_REG_SET(panel_port, 0x001C, shift_val);
	RDP_REG_SET(panel_port, 0x001D, 0x11); // BPM_STROBE, BPM_MODE
	VC_M_DELAY(1);
	RDP_REG_SET(panel_port, 0x000A, 0x00);  // PIXEL RESET

	return 0;
}

int rdp551f_get_device_id(E_VC_PANEL_PORT_T port, U16_T *chip_id, U16_T *rev_num)
{
	U8_T reg0x00, reg0x01;

	reg0x00 = RDP_REG_GET(port, 0x0000);
	reg0x01 = RDP_REG_GET(port, 0x0001);

	*chip_id = ((U16_T)reg0x00<<4) | (reg0x01>>4);
	*rev_num = reg0x01 & 0x0F;

	return 0;
}

static int rdp551f_prepare_otp(U8_T panel_port)
{
	U8_T otp_indicator, otp_data;
	RDP551F_OTP_INFO_T *otp = &rdp551f_otp[panel_port];

	/* Read otp-data from the mirrored regisgter in RDP551F*/
	otp_indicator = RDP_REG_GET(panel_port, 0x0200);
	if (otp_indicator & RDP551F_OTP_INDICATOR_LED) {
		otp->led_apply = TRUE;

		/* Read otp-data from the mirrored regisgter in RDP551F */
		otp_data = RDP_REG_GET(panel_port, 0x025D); // LED_CON_R_POL0<7:0>
		otp->LED_CON = (char)otp_data;
	}

	otp_indicator = RDP_REG_GET(panel_port, 0x0201);

	if (otp_indicator & RDP551F_OTP_INDICATOR_TEMP_V2)
		otp->Temp_V2 = 1;
	
	if (otp_indicator & RDP551F_OTP_INDICATOR_TSENSOR) {
		otp->tsensor_apply = TRUE;

		/* Read otp-data from the mirrored regisgter in RDP551F */
		otp_data = RDP_REG_GET(panel_port, 0x0274); // TEMPERATURE<7:0>
		otp->tsensor_otp = (char)otp_data;
	}

	return 0;
}

static int rdp551f_prepare_lot(U8_T panel_port)
{
	int i;
	U8_T w_num, rev_num;
	char w_lot_name[7];
	U16_T addr = 0x02F9;

	rev_num = RDP_REG_GET(panel_port, 0x0001) & 0xF;
	DMSG("rev_num(%d)\n", rev_num);

	if (rev_num == 3) {

	}
	else {
#if 0
		for (i = 0; i < 6; i++)
			w_lot_name[i] = (char)RDP_REG_GET(panel_port, addr++);

		w_lot_name[6] = '\0';

		w_num = RDP_REG_GET(panel_port, addr);

#if 0
		if (!strcmp(w_lot_name, "Q02V99")) {
		}
		else if (!strcmp(w_lot_name, "Q02V98")) {
		}
		else {
		}
#endif
		DMSG("wf: w_lot_name(%s) num(%d)\n", w_lot_name, w_num);

#endif
	}

	return 0;
}

static int rdp551f_prepare_temperature(VC_PANEL_DEV_INFO_T *dev, U8_T panel_port)
{
	VC_TEMPERATURE_STATUS_T *temp_status = &dev->temperature_status[panel_port];
	
	/* Config and start temperature sensor. */
	RDP_REG_SET(panel_port, 0x0171, 1);
	RDP_REG_SET(panel_port, 0x0172, 0xF1);
	RDP_REG_SET(panel_port, 0x0173, 0x00);

	/* Get and store the previous data into device structure */
	rdp551f_get_temperature(panel_port,
								&temp_status->last_tsensor_code,
								&temp_status->last_temperature);

	temp_status->otp_applied = rdp551f_otp[panel_port].tsensor_apply;

	return 0;
}

static int rdp551f_prepare_panel(VC_PANEL_DEV_INFO_T *dev)
{
#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	rdp551f_prepare_lot(VC_PANEL_PORT_0);

	rdp551f_prepare_otp(VC_PANEL_PORT_0);

//	rdp551f_prepare_temperature(dev, VC_PANEL_PORT_0);
	
	rdp551f_read_led_current_from_reg(VC_PANEL_PORT_0,
									&dev->led_current_info[VC_PANEL_PORT_0]);
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	rdp551f_prepare_lot(VC_PANEL_PORT_1);

	rdp551f_prepare_otp(VC_PANEL_PORT_1);

//	rdp551f_prepare_temperature(dev, VC_PANEL_PORT_1);
	
	rdp551f_read_led_current_from_reg(VC_PANEL_PORT_1,
									&dev->led_current_info[VC_PANEL_PORT_1]);
#endif

	return 0;
}

int RDP551F_PanelProbe(VC_PANEL_DEV_INFO_T *dev)
{
	memset(rdp551f_otp, 0x0, sizeof(rdp551f_otp));

	dev->prepare_panel = rdp551f_prepare_panel;
	dev->get_device_id = rdp551f_get_device_id;

	dev->get_led_current = rdp551f_get_led_current;
	dev->set_led_current = rdp551f_set_led_current;
	dev->get_led_level_range = rdp551f_get_led_level_range;

	dev->set_horizontal_pixel_shift = rdp551f_set_horizontal_pixel_shift;
	dev->set_vertical_pixel_shift = rdp551f_set_vertical_pixel_shift;
	dev->get_horizontal_pixel_shift = rdp551f_get_horizontal_pixel_shift;
	dev->get_vertical_pixel_shift = rdp551f_get_vertical_pixel_shift;

	dev->get_temperature = rdp551f_get_temperature;
	    
	return 0;
}

#endif // #if defined(CFG_PANEL_RDP551F)

