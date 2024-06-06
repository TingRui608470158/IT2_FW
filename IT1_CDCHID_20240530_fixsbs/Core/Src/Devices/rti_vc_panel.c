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

/**
 * @file rti_vc_panel.c
 * @author Kevin Ko
 * @brief Implements the panel driver.
 */
 
#include "rti_vc_panel.h"

int RDP501H_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP360H_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP550F_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP700Q_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP551F_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP502H_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP370F_PanelProbe(VC_PANEL_DEV_INFO_T *dev);
int RDP250H_PanelProbe(VC_PANEL_DEV_INFO_T *dev);

int rdp550f_f_attached = -1; /* 1-: default, 0: dettached, 1: attached */
int rdc200_poweron_panel(BOOL_T on); /* For RDP550F */

static VC_PANEL_CB_T vc_panel_cb;

static int curligntVal;


#define DISACRD_RAW_CODE_DIFF	3

S32_T rdp_compensate_temperature(S32_T raw_code[], U8_T num_raw_code)
{
	U8_T i, j, k, median_idx;
	S32_T tmp, median, diff;
	static S32_T temp_Y[NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0];
	static S32_T valid_X[NUM_RDP_TEMP_SENSOR_READ_RAW_CODE_TYPE0];
	S32_T sum_X = 0;
	S32_T comp_raw_code = 0;

	/* Sort to get the median. */
	for (i = 0; i < (num_raw_code-1) ; i++) {
		for (j = 0; j < (num_raw_code-i-1); j++) {
			if (raw_code[j+1] < raw_code[j]) { 
				tmp = raw_code[j];
				raw_code[j] = raw_code[j+1];
				raw_code[j+1] = tmp;
			}
		}
	}

	median_idx = num_raw_code >> 1; // Divied by 2.
	median = raw_code[median_idx]; // The number of element is odd number.

	return median;
}

int rtiVC_DecreaseBrightness(void)
{
	int ret = -1;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (dev->decrease_brightness) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		
		ret = dev->decrease_brightness();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

int rtiVC_IncreaseBrightness(void)
{
	int ret = -1;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (dev->increase_brightness) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
	
		ret = dev->increase_brightness();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

void rtiVC_SetBrightness(int brightness)
{
	//VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (brightness > curligntVal) {
		//if (dev)
	//		dev->set_brightnessUp();
	}
	else if(brightness < curligntVal) {
	//	if (dev)
	//		dev->set_brightnessDown();
	}
	//else curligntVal = brightness;
	
}

int rtiVC_GetPanelOtpSummary(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						char summary_buf[MAX_NUM_VC_PANEL_PORT][48])
{
	int ret = 0;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	do {
		if (chk_port & 0x1) {
			if (dev->get_otp_summary) {
				VC_MUTEX_LOCK(&vc_dev_access_lock);
				ret = dev->get_otp_summary(port, summary_buf[port]);
				VC_MUTEX_FREE(&vc_dev_access_lock);
			}
			else {
				EMSG("Interface function is NULL\n");
				ret = -2;
			}

			if (ret)
				break;
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

	return ret;
}

/**
 * @brief Measures the temperature of panel.
 * This function measures the temperature with the temperature sensor
 * on the panel and returns the code and the temperature in degrees celsius.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[out] tinfo Buffer of temperature inforamtion to read.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 * @note The value of celsius temperature is calculated by multiplying by 1000
 *       to avoid floating point operations.
 *
 */
int rtiVC_GetTemperature(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_PANEL_TEMPERATURE_INFO_T tinfo[MAX_NUM_VC_PANEL_PORT])
{
	int ret;
	VC_PANEL_TEMPERATURE_INFO_T *t;
	int i = 0;
	U8_T chk_port = (U8_T)ctrl_port;
	VC_TEMPERATURE_STATUS_T *temp_status;	
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	if (dev->get_temperature) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		do {
			if (chk_port & 0x1) {
				t = &tinfo[i];
				temp_status = &dev->temperature_status[i];

				t->otp_applied = temp_status->otp_applied;

				/* Set the last code and temperature from device structure */
				t->prev_tsensor_code = temp_status->last_tsensor_code;
				t->prev_temperature = temp_status->last_temperature;

				ret = dev->get_temperature(i, &t->tsensor_code, &t->temperature);
				if (ret)
					break;

				/* Update the last code and temperature in device structure */
				temp_status->last_tsensor_code = t->tsensor_code;
				temp_status->last_temperature = t->temperature;			
			}

			i++;
			chk_port >>= 1;
		} while (chk_port);	

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("[rtiVC_GetTemperature] Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Gets the vertical shift value from the panel, in pixel unit.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[out] shift_val Buffer of vertical shift value to read.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetVerticalPixelShift(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							int shift_val[MAX_NUM_VC_PANEL_PORT])
{
	int ret = 0;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	do {
		if (chk_port & 0x1) {
			if (dev->get_vertical_pixel_shift) {
				VC_MUTEX_LOCK(&vc_dev_access_lock);
				ret = dev->get_vertical_pixel_shift(port, &shift_val[port]);
				VC_MUTEX_FREE(&vc_dev_access_lock);
			}
			else {
				EMSG("Interface function is NULL\n");
				ret = -2;
			}

			if (ret)
				break;
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

	return ret;
}

/**
 * @brief Gets the horizontal shift value from the panel, in pixel unit.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[out] shift_val Buffer of horizontal shift value to read.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetHorizontalPixelShift(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							int shift_val[MAX_NUM_VC_PANEL_PORT])
{
	int ret = 0;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;	
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	do {
		if (chk_port & 0x1) {
			if (dev->get_horizontal_pixel_shift) {
				VC_MUTEX_LOCK(&vc_dev_access_lock);
				ret = dev->get_horizontal_pixel_shift(port, &shift_val[port]);
				VC_MUTEX_FREE(&vc_dev_access_lock);
			}
			else {
				EMSG("Interface function is NULL\n");
				ret = -2;
			}

			if (ret)
				break;			
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

	return ret;
}

/**
 * @brief Set the pixel-shift value of the panel in the vertical direction.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in] shift_val The value of vertical shift from top to bottom.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 * @note shift_val ranges are,
 * RDP550F-F: -8 ~ 8
 * RDP551F, RDP502H, RDP370F: 0 ~ 200 (16 center, cropped more than 32)
 *
 */
int rtiVC_SetVerticalPixelShift(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							int shift_val)
{
	int ret = 0;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

#if defined(CFG_MDC_RDC200)
	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	if (rdp550f_f_attached == 1) {
		U8_T reg_0x75;
		U8_T msb, lsb;
		int center = shift_val + 8;

		if ((shift_val < -8) || (shift_val > 8)) {
			EMSG("Vertical values is out of range! Range(from -8 to 8)\n");
			return -2;
		}

		reg_0x75 = RDC_REG_GET(0x75); /* [7]: CoKOA_en_boot */
		if (reg_0x75 & 0x80) { // FHD
			do {
				if (chk_port & 0x1) {
					if (dev->set_vertical_pixel_shift) {
						VC_MUTEX_LOCK(&vc_dev_access_lock);
						rdc200_poweron_panel(FALSE);
						ret = dev->set_vertical_pixel_shift(port, center);
						rdc200_poweron_panel(TRUE);					
						VC_MUTEX_FREE(&vc_dev_access_lock);
					}
					else {
						EMSG("Interface function is NULL\n");
						ret = -3;
					}

					if (ret)
						goto out;
				}

				port++;
				chk_port >>= 1;
			} while (chk_port);
		}
		else { // Full size
			msb = (center >> 3) & 0x7F;
			lsb = (center & 0x07) << 5;

			RDC_REG_MASK_SET(0x45, 0x7F, msb);
			RDC_REG_MASK_SET(0x46, 0xE0, lsb);
		}
	}
	else {
		if ((shift_val < 0) || (shift_val > 200)) {
			EMSG("Vertical values is out of range! Range(from 0 to 200)\n");
			return -4;
		}

		do {
			if (chk_port & 0x1) {
				if (dev->set_vertical_pixel_shift) {
					VC_MUTEX_LOCK(&vc_dev_access_lock);
					ret = dev->set_vertical_pixel_shift(port, shift_val);
					VC_MUTEX_FREE(&vc_dev_access_lock);
				}
				else {
					EMSG("Interface function is NULL\n");
					ret = -5;
				}

				if (ret)
					goto out;
			}

			port++;
			chk_port >>= 1;
		} while (chk_port);
	}

#elif defined(CFG_MDC_RDC200A)
	if ((shift_val < 0) || (shift_val > 200)) {
		EMSG("Vertical values is out of range! Range(from 0 to 200)\n");
		return -4;
	}

	do {
		if (chk_port & 0x1) {
			if (dev->set_vertical_pixel_shift) {
				VC_MUTEX_LOCK(&vc_dev_access_lock);
				ret = dev->set_vertical_pixel_shift(port, shift_val);
				VC_MUTEX_FREE(&vc_dev_access_lock);
			}
			else {
				EMSG("Interface function is NULL\n");
				ret = -5;
			}

			if (ret)
				goto out;
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);
#else
	#error "Code not present"
#endif

out:
	return ret;
}

/**
 * @brief Set the pixel-shift value of the panel in the horizontal direction.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in] shift_val The value of horizontal shift from left to right.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_SetHorizontalPixelShift(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							int shift_val)
{
	int ret = 0;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

#if defined(CFG_MDC_RDC200)
	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	if (rdp550f_f_attached == 1) {
		U8_T reg_0x40, reg_0x75;
		U16_T step, reg_val;
		U8_T msb, lsb;
		int center = shift_val + 72;
	
		if ((shift_val < -32) || (shift_val > 31)) {
			EMSG("Horizontal values is out of range! Range(from -32 to 31)\n");
			return -2;
		}
	
		reg_0x75 = RDC_REG_GET(0x75); /* [7]: CoKOA_en_boot */
		if (reg_0x75 & 0x80) { // FHD
			do {
				if (chk_port & 0x1) {
					if (dev->set_horizontal_pixel_shift) {
						VC_MUTEX_LOCK(&vc_dev_access_lock);
						rdc200_poweron_panel(FALSE);
						ret = dev->set_horizontal_pixel_shift(port, center);
						rdc200_poweron_panel(TRUE);
						VC_MUTEX_FREE(&vc_dev_access_lock);
					}
					else {
						EMSG("Interface function is NULL\n");
						ret = -3;
					}

					if (ret)
						goto out;
				}

				port++;
				chk_port >>= 1;
			} while (chk_port);
		}
		else { // Full size
			reg_0x40 = RDC_REG_GET(0x40); // config_panel_drv
			step = reg_0x40 >> 6;
			if (step >= 3)
				step = 0;

			step = 4 << step; // step: 0 ~ 2
			reg_val = center / step;
			// reg_val = center >> (2 + step);

			msb = (reg_val >> 1) & 0xFF;
			lsb = (reg_val & 0x01) << 7;
			
			RDC_REG_SET(0x44, msb);
			RDC_REG_MASK_SET(0x45, 0x80, lsb);

			do {
				if (chk_port & 0x1) {
					if (dev->set_horizontal_pixel_shift) {
						VC_MUTEX_LOCK(&vc_dev_access_lock);
						ret = dev->set_horizontal_pixel_shift(port, center % step);
						VC_MUTEX_FREE(&vc_dev_access_lock);
					}
					else {
						EMSG("Interface function is NULL\n");
						ret = -4;
					}

					if (ret)
						goto out;
				}

				port++;
				chk_port >>= 1;
			} while (chk_port);
		}
	}
	else {
		if ((shift_val < 0) || (shift_val > 200)) {
			EMSG("Horizontal values is out of range! Range(from 0 to 200)\n");
			return -5;
		}

		do {
			if (chk_port & 0x1) {
				if (dev->set_horizontal_pixel_shift) {
					VC_MUTEX_LOCK(&vc_dev_access_lock);
					ret = dev->set_horizontal_pixel_shift(port, shift_val);
					VC_MUTEX_FREE(&vc_dev_access_lock);
				}
				else {
					EMSG("Interface function is NULL\n");
					ret = -6;
				}

				if (ret)
					goto out;
			}

			port++;
			chk_port >>= 1;
		} while (chk_port);
	}

#elif defined(CFG_MDC_RDC200A)
	if ((shift_val < 0) || (shift_val > 200)) {
		EMSG("Horizontal values is out of range! Range(from 0 to 200)\n");
		return -5;
	}

	do {
		if (chk_port & 0x1) {
			if (dev->set_horizontal_pixel_shift) {
				VC_MUTEX_LOCK(&vc_dev_access_lock);
				ret = dev->set_horizontal_pixel_shift(port, shift_val);
				VC_MUTEX_FREE(&vc_dev_access_lock);
			}
			else {
				EMSG("Interface function is NULL\n");
				ret = -6;
			}

			if (ret)
				goto out;
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

#else
	#error "Code not present"
#endif

out:
	return ret;
}

/**
 * @brief Get the device ID and revision number for RDP.
 *
 * @param[in] port Panel port to control.
 * @param[out] chip_id Chip identification.
 * @param[out] rev_num Revision Number.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetPanelDeviceID(E_VC_PANEL_PORT_T port, U16_T *chip_id, U16_T *rev_num)
{
	int ret;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (dev->get_device_id) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_device_id(port, chip_id, rev_num);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Get the current LED current level from panel.
 *
 * @param[in] ctrl_port Panel port to control.
 * @param[out] info The information of LED current level to read.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetLedCurrent(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_CURRENT_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
	int ret;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	if (dev->get_led_current) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_led_current(dev, ctrl_port, info);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Set LED current with the specified level.
 *
 * @param[in] ctrl_port Panel port to control.
 * @param[out] level The LED current level to be set.
 * @param[out] color LED color to control.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_SetLedCurrent(E_VC_PANEL_CTRL_PORT_T ctrl_port,
					VC_LED_CURRENT_INFO_T level[MAX_NUM_VC_PANEL_PORT],
					E_VC_LED_CTRL_COLOR_T color)
{
	int ret = 0;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (ctrl_port < VC_PANEL_CTRL_PORT_0) {
		EMSG("Invalid control panel port\n");
		return -1;
	}

	if (dev->set_led_current) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->set_led_current(ctrl_port, color, level);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Get the minimum and maximum level of panel's led current.
 *
 * @param[out] min Minimum level.
 * @param[out] max maximum level.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetLedLevelRange(U8_T *min, U8_T *max)
{
	int ret = 0;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

	if (dev->get_led_level_range) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_led_level_range(min, max);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

#if defined(CFG_MDC_RDC200A)
#define TX1_PANEL_ACCESS_EN		0x08
#define TX0_PANEL_ACCESS_EN		0x04
#define TX1_SLV_I2C_BYPASS		0x02
#define TX0_SLV_I2C_BYPASS		0x01

static void enable_backdoor_path(void)
{
	U8_T path_cfg = 0;

#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	path_cfg |= (TX0_PANEL_ACCESS_EN | TX0_SLV_I2C_BYPASS); 
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	path_cfg |= (TX1_PANEL_ACCESS_EN | TX1_SLV_I2C_BYPASS); 
#endif

	RDC_REG_SET(0x0004, 0x76);
	RDC_REG_SET(0x0005, _CFG_RDP_SLAVE_ADDR);
	RDC_REG_SET(0x0006, path_cfg);
}
#endif

/**
 * @brief Prepare panel at driver open time.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_prepare_panel(void)
{
	int ret = 0;
	VC_PANEL_DEV_INFO_T *dev = &vc_panel_cb.dev;

#if 0//defined(CFG_MDC_RDC200A)
	enable_backdoor_path();
#endif

	if (dev->prepare_panel) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->prepare_panel(dev);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Denitializes RDP driver.
 */
void rtiVC_DeinitPanel(void)
{
}

/**
 * @brief Initializes RDP panel.
 *
 * The function probes RDP driver and initializes the it's context.
 *
 */
void rtiVC_InitPanel(void)
{
	memset(&vc_panel_cb, 0, sizeof(vc_panel_cb));

//	curligntVal = getCurrentLightVal();

#if defined(CFG_PANEL_RDP551F)
	RDP551F_PanelProbe(&vc_panel_cb.dev);

#else
	#error "Code not present"
#endif

	rdp550f_f_attached = -1;
}


