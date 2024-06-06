
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
 * @brief Implements the RDC driver.
 */

#include "rti_vc_rdc.h"
#include "rti_vc_dload.h"

int RDC200_Probe(VC_RDC_DEV_INFO_T *dev);
int RDC200A_Probe(VC_RDC_DEV_INFO_T *dev);


VC_RDC_CB_T vc_rdc_cb;

int rtiVC_DisableLaserSparklingPwm(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->change_laser_sparkling_pwm) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		dev->disable_laser_sparkling();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

int rtiVC_ChangeLaserSparklingPwm(U16_T on_time_period, U16_T off_time_period)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->change_laser_sparkling_pwm) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->change_laser_sparkling_pwm(on_time_period, off_time_period);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}
/**
 * @brief Get the status of text OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] enabled Flag for enabled or not.
 * @param[in] showed Flag for showed or not.
 * @param[in] pos_x X coordinate of starting position on the display.
 * @param[in] pos_y Y coordinate of starting position on the display.
 * @param[in] font_addr FLASH address of font LUT stored in SPI FLASH.
 * @param[in] palette_addr FLASH address of palette LUT stored in SPI FLASH.
 * @param[in] mag Magnification.
 * @param[in] font_alpha Font alpha blending.
 * @param[in] back_alpha Background alpha blending.
 * @param[in] font_color_idx Index of font color in palette LUT file.
 * @param[in] back_color_idx Index of background color in palette LUT file.
 * @param[in] d Dimension.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetStatusTextOSD(int handle, BOOL_T *enabled, BOOL_T *showed,
								U32_T *pos_x, U32_T *pos_y,
								U32_T *font_addr,  U32_T *palette_addr,
								E_VC_OSD_TEXT_MAG_T *mag,
								E_VC_OSD_ALPHA_T *font_alpha,
								E_VC_OSD_ALPHA_T *back_alpha,
								U8_T *font_color_idx, U8_T *back_color_idx,
								E_VC_OSD_DIMENSTION_T *d)
{
	VC_TEXT_DRAWER_CB *drawer_cb;

	if ((handle < 1) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

	*enabled = drawer_cb->enabled;
	*showed = drawer_cb->showed;
	*pos_x = drawer_cb->pos_x;
	*pos_y = drawer_cb->pos_y;
	*font_addr = drawer_cb->font_addr;
	*palette_addr = drawer_cb->palette_addr;
	*mag = drawer_cb->mag;
	*font_alpha = drawer_cb->font_alpha;
	*back_alpha = drawer_cb->back_alpha;
	*font_color_idx = drawer_cb->font_color_idx;
	*back_color_idx = drawer_cb->back_color_idx;
	*d = drawer_cb->d;

	return 0;
}

/**
 * @brief Changes the dimension of text OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] d Dimension.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangeDimensionTextOSD(int handle, E_VC_OSD_DIMENSTION_T d)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text drawer handle\n");
		return -1;
	}

	if (dev->change_dimension_text_osd) {	
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (d != drawer_cb->d) {
				ret = dev->change_dimension_text_osd(drawer_cb, d);
				if (ret == 0)
					drawer_cb->d = d;
			}
		}
		else {
			EMSG("Text drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Changes the property of text OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] mag Magnification.
 * @param[in] font_alpha Font alpha blending.
 * @param[in] back_alpha Background alpha blending.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangePropertyTextOSD(int handle, E_VC_OSD_TEXT_MAG_T mag,
								E_VC_OSD_ALPHA_T font_alpha,
								E_VC_OSD_ALPHA_T back_alpha)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	if (dev->change_property_text_osd) { 	
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			ret = dev->change_property_text_osd(drawer_cb, mag,
												font_alpha,	back_alpha);
			if (ret == 0) {
				drawer_cb->mag = mag;
				drawer_cb->font_alpha = font_alpha;
				drawer_cb->back_alpha = back_alpha;
			}
		}
		else {
			EMSG("Text drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Hide the text OSD.
 *
 * @param[in] handle Handle of drawer.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_HideTextOSD(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	if (dev->hide_text_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (drawer_cb->showed == TRUE) {
				ret = dev->hide_text_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = FALSE;
			}
		}
		else {
			EMSG("Text drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Show the text OSD after being hidden.
 *
 * @param[in] handle Handle of drawer.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ShowTextOSD(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	if (dev->show_text_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (drawer_cb->showed == FALSE) {
				ret = dev->show_text_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = TRUE;
			}
		}
		else {
			EMSG("Text drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Move the horizontal and vertical poistion of text OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] mov_x X coordinate of starting position to be moved.
 * @param[in] mov_y Y coordinate of starting position to be moved.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_MoveTextOSD(int handle, U32_T mov_x, U32_T mov_y)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	if (dev->move_text_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			ret = dev->move_text_osd(drawer_cb, mov_x, mov_y);
			if (ret == 0) {
				/* Update the position */
				drawer_cb->pos_x = mov_x;
				drawer_cb->pos_y = mov_y;
			}
		}
		else {
			EMSG("Text drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Draws a new text OSD with the specified parameters.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] pos_x X coordinate of starting position to be draw.
 * @param[in] pos_y Y coordinate of starting position to be draw.
 * @param[in] font_addr FLASH address of font LUT stored in SPI FLASH.
 *            If this is 0xFFFFFFFF, the default palette built in RDC is used.
 * @param[in] palette_addr FLASH address of font LUT stored in SPI FLASH.
 *            If this is 0xFFFFFFFF, the default palette built in RDC is used.
 * @param[in] mag Magnification
 * @param[in] font_alpha Font alpha blending
 * @param[in] back_alpha Background blpha blending
 * @param[in] font_color_idx Index of font color in palette LUT file.
 * @param[in] back_color_idx Index of background color in palette LUT file.
 * @param[in] d Dimension.
 * @param[in] string Text string to be displayed.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_DrawTextOSD(int handle, U32_T pos_x, U32_T pos_y,
						U32_T font_addr, U32_T palette_addr,
						E_VC_OSD_TEXT_MAG_T mag,
						E_VC_OSD_ALPHA_T font_alpha,
						E_VC_OSD_ALPHA_T back_alpha,
						U8_T font_color_idx, U8_T back_color_idx,
						E_VC_OSD_DIMENSTION_T d,
						const char *string)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid text handle\n");
		return -1;
	}

	if (dev->draw_text_osd) {		
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

		ret = dev->draw_text_osd(drawer_cb, pos_x, pos_y,
								font_addr, palette_addr,
								mag, font_alpha, back_alpha,
								font_color_idx, back_color_idx,
								d, string);
		if (ret == 0) {
			drawer_cb->pos_x = pos_x;
			drawer_cb->pos_y = pos_y;
			drawer_cb->font_addr = font_addr;
			drawer_cb->palette_addr = palette_addr;
			drawer_cb->font_color_idx = font_color_idx;
			drawer_cb->back_color_idx = back_color_idx;
			drawer_cb->enabled = TRUE;
			drawer_cb->showed = TRUE;
			drawer_cb->mag = mag;
			drawer_cb->font_alpha = font_alpha;
			drawer_cb->back_alpha = back_alpha;
			drawer_cb->d = d;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Destroys a bitmap text drawer created by rtiVC_CreateTextOsdDrawer().
 *
 * @param[in] handle Handle of drawer to be destroyed.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_DestoryTextOsdDrawer(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_TEXT_DRAWER_CB *drawer_cb;
#if defined(CFG_MDC_RDC200)
	int min_handle = 1;
#elif defined(CFG_MDC_RDC200A)
	int min_handle = 0;
#else
	#error "Code not present"
#endif

	if ((handle < min_handle) || (handle >= NUM_VC_TEXT_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	if (vc_rdc_cb.text_drawer_id_bits & (1<<handle)) {
		vc_rdc_cb.text_drawer_id_bits &= ~(1 << handle);

		if (dev->hide_bitmap_osd) {
			drawer_cb = &vc_rdc_cb.text_drawer_cb[handle];

			if (drawer_cb->showed == TRUE) {
				ret = dev->hide_text_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = FALSE;
			}
		
			if (drawer_cb->enabled == TRUE) {
				ret = dev->disable_text_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->enabled = FALSE;
			}
		}
		else {
			EMSG("Interface function is NULL\n");
			ret = -2;
		}
	}

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return ret;
}

/**
 * @brief Creates a text OSD drawer.
 *
 * @return Positive number(handle) on success,
 *         otherwise returns a negative number.
 *
 */
int rtiVC_CreateTextOsdDrawer(void)
{
	int i, handle = -1;

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	for (i = 0; i < NUM_RDC_TEXT_DRAWER; i++) {
		if ((vc_rdc_cb.text_drawer_id_bits & (1<<i)) == 0) {
			vc_rdc_cb.text_drawer_id_bits |= (1 << i);
			handle = i;

			vc_rdc_cb.text_drawer_cb[handle].enabled = FALSE;
			vc_rdc_cb.text_drawer_cb[handle].showed = FALSE;
			break;
		}
	}

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return handle;
}


/*
 *
 *  Bitmap OSD APIs
 */

/**
 * @brief Get the status of bitmap OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] enabled Flag for enabled or not.
 * @param[in] showed Flag for showed or not.
 * @param[in] pos_x X coordinate of starting position on the display.
 * @param[in] pos_y Y coordinate of starting position on the display.
 * @param[in] bmp_addr FLASH address of bitmap LUT stored in SPI FLASH.
 * @param[in] mag Magnification.
 * @param[in] alpha Alpha blending.
 * @param[in] d Dimension.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetStatusBitmapOSD(int handle, BOOL_T *enabled, BOOL_T *showed,
								U32_T *pos_x, U32_T *pos_y,
								U32_T *bmp_addr, E_VC_OSD_BITMAP_MAG_T *mag,
								E_VC_OSD_ALPHA_T *alpha,
								E_VC_OSD_DIMENSTION_T *d)
{
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

	*enabled = drawer_cb->enabled;
	*showed = drawer_cb->showed;
	*pos_x = drawer_cb->pos_x;
	*pos_y = drawer_cb->pos_y;
	*bmp_addr = drawer_cb->bmp_addr;
	*mag = drawer_cb->mag;
	*alpha = drawer_cb->alpha;
	*d = drawer_cb->d;

	return 0;
}

/**
 * @brief Changes the dimension of bitmap OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] d Dimension.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangeDimensionBitmapOSD(int handle, E_VC_OSD_DIMENSTION_T d)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->change_dimension_bitmap_osd) {	
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (d != drawer_cb->d) {
				ret = dev->change_dimension_bitmap_osd(drawer_cb, d);
				if (ret == 0)
					drawer_cb->d = d;
			}
		}
		else {
			EMSG("Bitmap drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Changes the property of bitmap OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] mag Magnification.
 * @param[in] alpha Alpha blending.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangePropertyBitmapOSD(int handle, E_VC_OSD_BITMAP_MAG_T mag,
									E_VC_OSD_ALPHA_T alpha)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->change_property_bitmap_osd) { 	
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			ret = dev->change_property_bitmap_osd(drawer_cb, mag, alpha);
			if (ret == 0) {
				drawer_cb->mag = mag;
				drawer_cb->alpha = alpha;
			}
		}
		else {
			EMSG("Bitmap drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Hide the bitmap OSD.
 *
 * @param[in] handle Handle of drawer.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_HideBitmapOSD(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->hide_bitmap_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (drawer_cb->showed == TRUE) {
				ret = dev->hide_bitmap_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = FALSE;
			}
		}
		else {
			EMSG("Bitmap drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Show the bitmap OSD after being hidden.
 *
 * @param[in] handle Handle of drawer.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ShowBitmapOSD(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->show_bitmap_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			if (drawer_cb->showed == FALSE) {
				ret = dev->show_bitmap_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = TRUE;
			}
		}
		else {
			EMSG("Bitmap drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Move the horizontal and vertical poistion of bitmap OSD on the display.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] mov_x X coordinate of starting position to be moved.
 * @param[in] mov_y Y coordinate of starting position to be moved.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_MoveBitmapOSD(int handle, U32_T mov_x, U32_T mov_y)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->move_bitmap_osd) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		if (drawer_cb->enabled == TRUE) {
			ret = dev->move_bitmap_osd(drawer_cb, mov_x, mov_y);
			if (ret == 0) {
				/* Update the position */
				drawer_cb->pos_x = mov_x;
				drawer_cb->pos_y = mov_y;
			}
		}
		else {
			EMSG("Bitmap drawer was not enabled\n");
			ret = -3;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Draws a new bitmap OSD with the specified parameters.
 *
 * @param[in] handle Handle of drawer.
 * @param[in] pos_x X coordinate of starting position to be draw.
 * @param[in] pos_y Y coordinate of starting position to be draw.
 * @param[in] bmp_addr FLASH address of bitmap LUT stored in SPI FLASH.
 * @param[in] mag Magnification
 * @param[in] alpha Alpha blending
 * @param[in] d Dimension.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_DrawBitmapOSD(int handle, U32_T pos_x, U32_T pos_y,
						U32_T bmp_addr, E_VC_OSD_BITMAP_MAG_T mag,
						E_VC_OSD_ALPHA_T alpha,
						E_VC_OSD_DIMENSTION_T d)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	if (dev->draw_bitmap_osd) {		
		VC_MUTEX_LOCK(&vc_dev_access_lock);
		drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

		ret = dev->draw_bitmap_osd(drawer_cb, pos_x, pos_y,
									bmp_addr, mag, alpha, d);
		if (ret == 0) {
			drawer_cb->pos_x = pos_x;
			drawer_cb->pos_y = pos_y;
			drawer_cb->bmp_addr = bmp_addr;
			drawer_cb->enabled = TRUE;
			drawer_cb->showed = TRUE;
			drawer_cb->mag = mag;
			drawer_cb->alpha = alpha;
			drawer_cb->d = d;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Destroys a bitmap OSD drawer created by rtiVC_CreateBitmapOsdDrawer().
 *
 * @param[in] handle Handle of drawer to be destroyed.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_DestoryBitmapOsdDrawer(int handle)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
	VC_BITMAP_DRAWER_CB *drawer_cb;

	if ((handle < 0) || (handle >= NUM_VC_BMP_DRAWER)) {
		EMSG("Invalid bitmap handle\n");
		return -1;
	}

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	if (vc_rdc_cb.bmp_drawer_id_bits & (1<<handle)) {
		vc_rdc_cb.bmp_drawer_id_bits &= ~(1 << handle);

		if (dev->hide_bitmap_osd) {
			drawer_cb = &vc_rdc_cb.bmp_drawer_cb[handle];

			if (drawer_cb->showed == TRUE) {
				ret = dev->hide_bitmap_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->showed = FALSE;
			}
		
			if (drawer_cb->enabled == TRUE) {
				ret = dev->disable_bitmap_osd(drawer_cb);
				if (ret == 0)
					drawer_cb->enabled = FALSE;
			}
		}
		else {
			EMSG("Interface function is NULL\n");
			ret = -2;
		}
	}

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return ret;
}

/**
 * @brief Creates a bitmap OSD drawer.
 *
 * @return Positive number(handle) on success,
 *         otherwise returns a negative number.
 *
 */
int rtiVC_CreateBitmapOsdDrawer(void)
{
	int i, handle = -1;

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	for (i = 0; i < NUM_VC_BMP_DRAWER; i++) {
		if ((vc_rdc_cb.bmp_drawer_id_bits & (1<<i)) == 0) {
			vc_rdc_cb.bmp_drawer_id_bits |= (1 << i);
			handle = i;

			vc_rdc_cb.bmp_drawer_cb[handle].enabled = FALSE;
			vc_rdc_cb.bmp_drawer_cb[handle].showed = FALSE;
			break;
		}
	}

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return handle;
}

int rtiVC_DisableVideoSyncInterrupt(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->disable_video_sync_interrupt) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->disable_video_sync_interrupt();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

int rtiVC_EnableVideoSyncInterrupt(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->enable_video_sync_interrupt) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->enable_video_sync_interrupt();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

int rtiVC_RequestAtwShiftPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
				RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->request_atw_shift_pixel) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->request_atw_shift_pixel(ctrl_port, info);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Requests the generation of CoKoA LUT to frmware.
 *
 * @param[in] param Information of CoKOA parameters.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_RequestCoKOAGeneration(VC_COKOA_PARAM_T *param)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if ((param->lt_x < -256) || (param->lt_x > 255)) {
		EMSG("keystone lt_x: Invalid value (-256 ~ 255)\n");
		return -1;
	}

	if ((param->rt_x < -256) || (param->rt_x > 255)) {
		EMSG("keystone rt_x: Invalid value (-256 ~ 255)\n");
		return -2;
	}

	if ((param->lb_x < -256) || (param->lb_x > 255)) {
		EMSG("lb_x: Invalid value (-256 ~ 255)\n");
		return -3;
	}

	if ((param->rb_x < -256) || (param->rb_x > 255)) {
		EMSG("rb_x: Invalid value (-256 ~ 255)\n");
		return -4;
	}

	if ((param->lt_y < -64) || (param->lt_y > 63)) {
		EMSG("lt_y: Invalid value (-64 ~ 63)\n");
		return -5;
	}

	if ((param->rt_y < -64) || (param->rt_y > 63)) {
		EMSG("rt_y: Invalid value (-64 ~ 63)\n");
		return -6;
	}	

	if ((param->lb_y < -64) || (param->lb_y > 63)) {
		EMSG("lb_y: Invalid value (-64 ~ 63)\n");
		return -7;
	}

	if ((param->rb_y < -64) || (param->rb_y > 63)) {
		EMSG("keystone rb_y: Invalid value (-64 ~ 63)\n");
		return -8;
	}

	if ((param->k1 < -0.500) || (param->k1 > 0.500)) {
		EMSG("k1: Invalid range (-0.500 ~ 0.500)\n");
		return -9;
	}

	if ((param->k2 < -0.500) || (param->k2 > 0.500)) {
		EMSG("k2: Invalid range (-0.500 ~ 0.500)\n");
		return -10;
	}

	if (param->chrm_r > 100) {
		EMSG("chrm_r: Invalid range (0 ~ 100)%%\n");
		return -12;
	}

	if (param->chrm_g > 100) {
		EMSG("chrm_g: Invalid range (0 ~ 100)%%\n");
		return -13;
	}

	if (param->chrm_b > 100) {
		EMSG("chrm_b: Invalid range (0 ~ 100)%%\n");
		return -14;
	}

	if ((param->interp < 0.500) || (param->interp > 1.500)) {
		EMSG("interp: Invalid range (0.500 ~ 1.500)\n");
		return -15;
	}
	
	if (dev->request_cokoa_generation) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->request_cokoa_generation(param);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;

}

/**
 * @brief Apply LUT data in FLASH memory.
 *
 * @param[in] addr The base addres of LUT in FLASH.
 * @param[in] size Size in byte to be applied.
 * @param[in] id LUT ID.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_UpdateLUT(U32_T addr, U32_T size, U8_T id)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->update_lut) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->update_lut(addr, size, id);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Enable/disable distortion correction.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in] dc_en Flag for DC enable/disable.
 * @param[in] lut_offset LUT offset(index) to be updated.
 * @param[in] ca_en Flag for chromatic aberration enable/disable.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_EnableDistortionCorrection(E_VC_PANEL_CTRL_PORT_T ctrl_port, BOOL_T dc_en,
						UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT], BOOL_T ca_en)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->enable_distortion_correction) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->enable_distortion_correction(ctrl_port, dc_en, lut_offset, ca_en);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Enable/disable gamma correction.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in] enable Flag for enable/disable.
 * @param[in] lut_offset Offset for Gamma LUT in FLASH memory.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_EnableGammaCorrection(E_VC_PANEL_CTRL_PORT_T ctrl_port,
								BOOL_T enable,
								UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT])
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->enable_gamma_correction) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->enable_gamma_correction(ctrl_port, enable, lut_offset);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Get the output resolution.
 *
 * @param[out] width Horizontal resolution.
 * @param[out] height Veritcal resolution.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetOutputResolution(U32_T *width, U32_T *height)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->get_output_resolution) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_output_resolution(width, height);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Set the format of the image to be displayed on the panel.
 *
 * @param[in] fmt Format of the image to be displayed on the panel.
 * @param[in] swap_image_3d_mode Left, Right image swap at 3D mode. 
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_SetImageDisplayFormat(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
								BOOL_T swap_image_3d_mode)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->set_image_display_format) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->set_image_display_format(fmt, swap_image_3d_mode);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

static int step_led_duty(struct _VC_RDC_DEV_INFO_T *dev,
						E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T applied_duty[MAX_NUM_VC_PANEL_PORT],
						E_VC_LED_CTRL_COLOR_T color, BOOL_T up)
{
	int ret;
	UINT_T port;
	VC_LED_DUTY_INFO_T new_duty[MAX_NUM_VC_PANEL_PORT] = {
		{-1, -1, -1},
		{-1, -1, -1}
	};

	if (!dev->fill_step_value) {
		EMSG("RDC: fill_step_value Interface function is NULL\n");
		return -1;
	}

	if (!dev->set_led_duty) {
		EMSG("RDC: set_led_duty Interface function is NULL\n");
		return -2;
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		/* Default return value. */
		applied_duty[0] = dev->led_duty_info[0];
		dev->fill_step_value(&new_duty[0], color, up, &dev->led_duty_info[0]);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		/* Default return value. */
		applied_duty[1] = dev->led_duty_info[1];
		dev->fill_step_value(&new_duty[1], color, up, &dev->led_duty_info[1]);
	}

	ret = dev->set_led_duty(ctrl_port, color, new_duty);
	if (ret == 0) {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
			port = 0;

			if (new_duty[port].R != -1) {
				dev->led_duty_info[port].R = new_duty[port].R; /* Update */
				applied_duty[port].R = new_duty[port].R;
			}

			if (new_duty[port].G != -1) {
				dev->led_duty_info[port].G = new_duty[port].G; /* Update */
				applied_duty[port].G = new_duty[port].G;
			}

			if (new_duty[port].B != -1) {
				dev->led_duty_info[port].B = new_duty[port].B; /* Update */
				applied_duty[port].B = new_duty[port].B;
			}
		}

		if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
			port = 1;

			if (new_duty[port].R != -1) {
				dev->led_duty_info[port].R = new_duty[port].R; /* Update */
				applied_duty[port].R = new_duty[port].R;
			}

			if (new_duty[port].G != -1) {
				dev->led_duty_info[port].G = new_duty[port].G; /* Update */
				applied_duty[port].G = new_duty[port].G;
			}

			if (new_duty[port].B != -1) {
				dev->led_duty_info[port].B = new_duty[port].B; /* Update */
				applied_duty[port].B = new_duty[port].B;
			}
		}		
	}

	return ret;
}

/**
 * @brief Get the current LED duty value.
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in, out] info The current LED duty value information.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 * @note The value of duty was pre-multiplied by VC_LED_DUTY_DIVIDER.
 *       To convert the return value into floating-point number in percentage,
 *       use VC_LED_DUTY_TO_PERCENTAGE macro.
 *
 */
int rtiVC_GetLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
					VC_LED_DUTY_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
	int ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->get_led_duty) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_led_duty(dev, ctrl_port, info);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}


/**
 * @brief Decrement LED duty by _CFG_LED_DUTY_CTRL_STEP defined in rti_vc_config.h
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in, out] applied_duty The applied duty value after step down.
 * @param[in] color LED color to control.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_StepDownLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T applied_duty[MAX_NUM_VC_PANEL_PORT],
							E_VC_LED_CTRL_COLOR_T color)
{
	int ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	ret = step_led_duty(dev, ctrl_port, applied_duty, color, FALSE);

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return ret;
}

/**
 * @brief Increment LED duty by _CFG_LED_DUTY_CTRL_STEP defined in rti_vc_config.h
 *
 * @param[in] ctrl_port The panel port to control.
 * @param[in, out] The applied duty value after step up.
 * @param[in] color LED color to control.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_StepUpLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T applied_duty[MAX_NUM_VC_PANEL_PORT], 
						E_VC_LED_CTRL_COLOR_T color)
{
	int ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	ret = step_led_duty(dev, ctrl_port, applied_duty, color, TRUE);

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return ret;
}

/**
 * @brief Get the current frame rate.
 *
 * @return On success, returns the actual frame rate applied.
 *         Otherwise returns a negative number.
 *
 * @note The value of frame rate was pre-multiplied by VC_FRAME_RATE_DIVIDER.
 *       To convert the return value into floating-point number in Hz,
 *       use VC_FRAME_RATE_TO_HZ macro.
 *
 */
S32_T rtiVC_GetFrameRate(void)
{
	S32_T ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->get_frame_rate) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_frame_rate(dev);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Decrement frame rate by _CFG_FRAME_RATE_CTRL_STEP defined in rti_vc_config.h
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
S32_T rtiVC_StepDownFrameRate(void)
{
	S32_T ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->step_frame_rate) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->step_frame_rate(dev, FALSE);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Increment frame rate by _CFG_FRAME_RATE_CTRL_STEP defined in rti_vc_config.h
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
S32_T rtiVC_StepUpFrameRate(void)
{
	S32_T ret;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->step_frame_rate) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->step_frame_rate(dev, TRUE);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Confirm the controling of RDP brightness to RDC using the interrupt.
 *
 * @param[in] rdc_idx RDC index to be control.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ConfirmLedControl(E_VC_RDC_INDEX_T rdc_idx)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->confirm_led_control) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->confirm_led_control(rdc_idx);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Request the controling of RDP brightness to RDC using the interrupt.
 *
 * @param[in] rdc_idx RDC index to be control.
 * @param[in] ctrl_port Panel port to be control.
 * @param[out] led_ctrl_info LED control information.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ReqeustLedControl(E_VC_RDC_INDEX_T rdc_idx,
							E_VC_PANEL_CTRL_PORT_T ctrl_port,
							VC_LED_CTRL_INFO_T *led_ctrl_info)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	VC_MUTEX_LOCK(&vc_dev_access_lock);

	if (led_ctrl_info->mode == VC_LED_CTRL_MODE_BRIGHTNESS) {
		if (dev->request_led_brightness_mode) {
			ret = dev->request_led_brightness_mode(rdc_idx, ctrl_port,
												led_ctrl_info->param.brightness);
		}
		else {
			EMSG("Interface function is NULL\n");
			ret = -2;
		}
	}
	else {
		if (dev->request_led_color_mode) {
			ret = dev->request_led_color_mode(rdc_idx, ctrl_port, led_ctrl_info);
		}
		else {
			EMSG("Interface function is NULL\n");
			ret = -3;
		}	
	}

	VC_MUTEX_FREE(&vc_dev_access_lock);

	return ret;
}

/**
 * @brief Shift the pixel of the panel in the horizontal direction.
 *
 * @param[in] ctrl_port The panel port to control. 
 * @param[in] h_direction Horizontal direction to shift.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ShiftHorizontalPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							E_VC_SHIFT_HORIZONTAL_T h_direction)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->shift_horizontal_pixel) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->shift_horizontal_pixel(ctrl_port, h_direction);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Shift the pixel of the panel in the vertical direction.
 *
 * @param[in] ctrl_port The panel port to control. 
 * @param[in] v_direction Vertical direction to shift.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ShiftVerticalPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						E_VC_SHIFT_VERTICAL_T v_direction)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->shift_vertical_pixel) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->shift_vertical_pixel(ctrl_port, v_direction);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Generates the test-pattern with the specified RGB values.
 *
 * @param[in] R 8-bit Red value.
 * @param[in] G 8-bit Green value.
 * @param[in] B 8-bit Blue value.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GenerateTestPattern(U8_T R, U8_T G, U8_T B)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->generate_test_pattern) {
		if (vc_rdc_cb.test_pattern_enabled == FALSE) {
			EMSG("Test pattern NOT enabled\n");
			return -1;
		}

		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->generate_test_pattern(R, G, B);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Enables or disables the Test Pattern mode.
 *
 * @param[in] on Flag. If TURE is enabling of Test pattern.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_EnableTestPattern(BOOL_T en)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->enable_test_pattern) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->enable_test_pattern(en);
		if (ret == 0)
			vc_rdc_cb.test_pattern_enabled = en;

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Power on/off the panel.
 *
 * @param[in] on Flag for power on/off panel.
 *               If TURE is power on the panel, else power off.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_PowerOnPanel(BOOL_T on)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->poweron_panel) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->poweron_panel(on);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Change the input resolution.
 *
 * @param[in] res The new input resolution type.
 * @param[in] enable_3D Flag of 3D demension.
 * @param[in] pclk_hz Pixel clock, in Hz, of the new input resolution.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangeInputResolution2(VC_INPUT_RES_T res, BOOL_T enable_3D,
							U32_T pclk_hz)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->change_input_resolution2) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->change_input_resolution2(res, enable_3D, pclk_hz);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Change the input resolution.
 *
 * @param[in] res The new input resolution type.
 * @param[in] enable_3D Flag of 3D demension.
 * @param[in] h_blank Total H blank (HSW+HBP+HFP)
 * @param[in] v_blank Total V blank (VSW+VBP+VFP)
 * @param[in] frame_rate Target frame rate.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangeInputResolution(VC_INPUT_RES_T res, BOOL_T enable_3D,
							U32_T h_blank, U32_T v_blank,
							U32_T frame_rate)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->change_input_resolution) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->change_input_resolution(res, enable_3D, h_blank,
											v_blank, frame_rate);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Change the input display interface between MIPI and LVDS.
 *
 * @param[in] dif Display interface to change.
 * @param[out] cfg Configuration of display interface to change.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ChangeInputDisplayInterface(E_VC_INPUT_DISPLAY_IF_T dif,
										VC_INPUT_DISPLAY_IF_CFG *cfg)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->change_input_display_interface) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->change_input_display_interface(dif, cfg);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Prepare RDC controller at driver open time.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_prepare_controller(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->prepare_controller) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->prepare_controller(dev);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Get the RDC firmware version.
 *
 * @param[out] major_ver Major version number.
 * @param[out] minor_ver Minor version number.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetFirmwareVersion(U8_T *major_ver, U8_T *minor_ver, U8_T *rel_ver)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->get_fw_version) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_fw_version(major_ver, minor_ver, rel_ver);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Read the data from FLASH memory with the specified length.
 *
 * @param[in] addr Starting address of FLASH to read.
 * @param[out] buf Pointer to buffer to read.
 * @param[in] len Number of bytes to be read.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ReadFLASH(U32_T addr, U8_T *buf, UINT_T len)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev;

	dev = &vc_rdc_cb.dev;

	if (dev->flash_read) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->flash_read(addr, buf, len);
		if (ret < 0) {
			EMSG("[VC_DLOAD] Can't read the data of FLASH to dump.\n");
			ret = -1;
		}

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Program the data in buffer into FLASH memory with the specified length.
 *
 * @param[in] addr Starting address of FLASH to be written.
 * @param[in] buf Pointer to data buffer to be written.
 * @param[in] len Data length in byte be written.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ProgramFLASH(U32_T addr, const U8_T *buf, UINT_T len)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev;

	dev = &vc_rdc_cb.dev;

	if (dev->flash_write) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

        if (dev->flash_prepare)
            dev->flash_prepare();
        
		ret = dev->flash_write(addr, buf, len);
		if (ret < 0) {
			EMSG("[VC_DLOAD] Can't program FLASH memory.\n");
			ret = -1;
		}

        if (dev->flash_finish)
            dev->flash_finish();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}
    
	return ret;
}

/**
 * @brief Erase a sector of FLASH attached to RDC.
 *
 * @param[in] sector_addr The base address of sector to be erased.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_EraseSectorFLASH(U32_T sector_addr)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev;

	dev = &vc_rdc_cb.dev;

	if (dev->flash_erase_sector) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

        if (dev->flash_prepare)
            dev->flash_prepare();
        
		ret = dev->flash_erase_sector(sector_addr);
		if (ret < 0) {
			EMSG("[VC_DLOAD] Can't erase FLASH memory.\n");
			ret = -1;
		}

        if (dev->flash_finish)
            dev->flash_finish();
        
		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -2;
	}

	return ret;
}

/**
 * @brief Erase the entire data of FLASH attached to RDC.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_EraseFlashChip(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;
    
	if (dev->flash_erase_chip) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

        if (dev->flash_prepare)
            dev->flash_prepare();
        
		ret = dev->flash_erase_chip();

        if (dev->flash_finish)
            dev->flash_finish();
        
		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Get the device ID and revision number from RDC device.
 *
 * @param[out] chip_id 8-bit chip identification of RDC.
 * @param[out] rev_num The revision number of RDC.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_GetRdcDeviceID(U16_T *chip_id, U16_T *rev_num)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->get_device_id) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->get_device_id(chip_id, rev_num);

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Resume RDC device.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ResumeController(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->resume) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->resume();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Suspend RDC device.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_SuspendController(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->suspend) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->suspend();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Power off RDC device.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_PowerOffController(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->power_off) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->power_off();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Power on RDC device.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_PowerOnController(void)
{
	int ret = 0;
	VC_RDC_DEV_INFO_T *dev = &vc_rdc_cb.dev;

	if (dev->power_on) {
		VC_MUTEX_LOCK(&vc_dev_access_lock);

		ret = dev->power_on();

		VC_MUTEX_FREE(&vc_dev_access_lock);
	}
	else {
		EMSG("Interface function is NULL\n");
		ret = -1;
	}

	return ret;
}

/**
 * @brief Denitializes RDC controller driver.
 */
void rtiVC_DeinitRDC(void)
{
	rtiVC_DeinitDownload();
}

/**
 * @brief Initializes RDC controller.
 *
 * The function probes RDC driver and initializes the it's context.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_InitRDC(void)
{
	VC_RDC_DEV_INFO_T *dev;
	int ret = 0;

	memset(&vc_rdc_cb, 0, sizeof(vc_rdc_cb));
	
#if defined(CFG_MDC_RDC200)
	ret = RDC200_Probe(&vc_rdc_cb.dev);

	vc_rdc_cb.text_drawer_id_bits = 0x01; // Except for drawer 0

	{
	int i;
	for (i = 0; i < NUM_VC_BMP_DRAWER; i++)
		vc_rdc_cb.bmp_drawer_cb[i].drawer_id = i;

	for (i = 0; i < NUM_RDC_TEXT_DRAWER; i++)
		vc_rdc_cb.text_drawer_cb[i].drawer_id = i;
	}
	
#elif defined(CFG_MDC_RDC200A)
	ret = RDC200A_Probe(&vc_rdc_cb.dev);

	{
	int i;
	for (i = 0; i < NUM_VC_BMP_DRAWER; i++)
		vc_rdc_cb.bmp_drawer_cb[i].drawer_id = i;

	for (i = 0; i < NUM_RDC_TEXT_DRAWER; i++)
		vc_rdc_cb.text_drawer_cb[i].drawer_id = i;
	}

#else
	#error "Code not present"
#endif

	if (ret != 0)
		return ret;

	dev = &vc_rdc_cb.dev;

	if ((1<<dev->sector_size_shift) > MAX_VC_FLASH_SECTOR_SIZE) {
		EMSG("The sector size of FLASH is too big!\n");
		return -1;		
	}

	dev->led_duty_info[0].R = -1;
	dev->led_duty_info[0].G = -1;
	dev->led_duty_info[0].B = -1;

	dev->led_duty_info[1].R = -1;
	dev->led_duty_info[1].G = -1;
	dev->led_duty_info[1].B = -1;

	rtiVC_InitDownload();

	return 0;
}

