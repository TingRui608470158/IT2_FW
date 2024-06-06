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

#include "main.h"
#include "i2c.h"
#include "stm32l4xx_hal.h"
#include "rdc200a.h"

#include <math.h>

#if defined(CFG_MDC_RDC200A)
#include "rdc200a_internal.h"

int RDC200A_ProbeFLASH(VC_RDC_DEV_INFO_T *dev);

typedef struct
{
//    Gpio(GPIO_TypeDef* port, uint16_t pin) : mPort(port), mPin(pin)
    GPIO_TypeDef* mPort;
    uint16_t mPin;
}Gpio;

Gpio GpioRDC200A = {RDC200A_RSTB_H_GPIO_Port, RDC200A_RSTB_H_Pin};
static U8_T rdc200a_mrx_config;
static U8_T rdc200a_pgen_config;

static const U8_T rdc200a_palette_drawer_sram_id[NUM_RDC_TEXT_DRAWER] =
	{TSID_FONT_BG_COLOR_0, TSID_FONT_BG_COLOR_1, TSID_FONT_BG_COLOR_2, TSID_FONT_BG_COLOR_3};

static const U8_T rdc200a_font_drawer_sram_id[NUM_RDC_TEXT_DRAWER] =
	{TSID_FONT_BMP_0, TSID_FONT_BMP_1, TSID_FONT_BMP_2, TSID_FONT_BMP_3};

static const U8_T rdc200a_bmp_drawer_sram_id[NUM_VC_BMP_DRAWER] =
	{TSID_BMP_DRAWER_0, TSID_BMP_DRAWER_1, TSID_BMP_DRAWER_2};

static const RDC200A_OSD_MAG_SIZE_INFO_T
			rdc200a_font_osd_mag_size[MAX_NUM_VC_TEXT_OSD_MAG] = {
	{RDC200A_FONT_BOX_WITDH,   RDC200A_FONT_BOX_HEGIHT},
	{RDC200A_FONT_BOX_WITDH*2, RDC200A_FONT_BOX_HEGIHT*2},
	{RDC200A_FONT_BOX_WITDH*3, RDC200A_FONT_BOX_HEGIHT*3},
	{RDC200A_FONT_BOX_WITDH*4, RDC200A_FONT_BOX_HEGIHT*4},
	{RDC200A_FONT_BOX_WITDH*5, RDC200A_FONT_BOX_HEGIHT*5},
	{RDC200A_FONT_BOX_WITDH*6, RDC200A_FONT_BOX_HEGIHT*6},
	{RDC200A_FONT_BOX_WITDH*7, RDC200A_FONT_BOX_HEGIHT*7}
};

static const RDC200A_OSD_MAG_SIZE_INFO_T
			rdc200a_bmp_osd_mag_size[MAX_NUM_VC_BITMAP_OSD_MAG] = {
	{RDC200A_BMP_RES_WITDH,   RDC200A_BMP_RES_HEGIHT},
	{RDC200A_BMP_RES_WITDH*2, RDC200A_BMP_RES_HEGIHT*2},
	{RDC200A_BMP_RES_WITDH*3, RDC200A_BMP_RES_HEGIHT*3},
	{RDC200A_BMP_RES_WITDH*4, RDC200A_BMP_RES_HEGIHT*4}
};

static const RDC200A_GAMMA_LUT_ID_INFO_T rdc200a_gamma_lut_id[MAX_NUM_VC_PANEL_PORT] = {
	{
		TSID_TX0_DIG_GAMMA_RED_POL_H,
		TSID_TX0_DIG_GAMMA_GREEN_POL_H,
		TSID_TX0_DIG_GAMMA_BLUE_POL_H,
		TSID_TX0_DIG_GAMMA_RED_POL_L,
		TSID_TX0_DIG_GAMMA_GREEN_POL_L,
		TSID_TX0_DIG_GAMMA_BLUE_POL_L,
	},
	{
		TSID_TX1_DIG_GAMMA_RED_POL_H,
		TSID_TX1_DIG_GAMMA_GREEN_POL_H,
		TSID_TX1_DIG_GAMMA_BLUE_POL_H,
		TSID_TX1_DIG_GAMMA_RED_POL_L,
		TSID_TX1_DIG_GAMMA_GREEN_POL_L,
		TSID_TX1_DIG_GAMMA_BLUE_POL_L,
	}
};

const RDC200A_RESOLUTION_INFO_T rdc200a_output_res[] =
{
	{ 640 ,  480},// 0x00 : VGA          ( 640 x  480),
	{ 864 ,  480},// 0x01 : WVGA         ( 864 x  480),
	{ 960 ,  544},// 0x02 : QFHD         ( 960 x  544),
	{1024 ,  600},// 0x03 : WSVGA        (1024 x  600),
	{1024 ,  768},// 0x04 : XGA          (1024 x  768),
	{1280 ,  720},// 0x05 : HD           (1280 x  720),
	{1280 ,  768},// 0x06 : WXGA_S       (1280 x  768),
	{1280 ,  800},// 0x07 : WXGA         (1280 x  800),
	{1024 , 1024},// 0x08 : 1K1K         (1024 x 1024),
	{1088 , 1080},// 0x09 : 1080R        (1088 x 1080),
	{1280 ,  960},// 0x0a : MLO          (1280 x  960),
	{1280 , 1024},// 0x0b : SXGA         (1280 x 1024),
	{1600 ,  904},// 0x0c : 1600X900     (1600 x  904),
	{1440 , 1080},// 0x0d : 1440X1080    (1440 x 1080),
	{1280 , 1280},// 0x0e : 1280R        (1280 x 1280),
	{2560 ,  720},// 0x0f : DUALHD       (2560 x  720),
	{1600 , 1200},// 0x10 : UXGA_S       (1600 x 1200),
	{2560 ,  768},// 0x11 : DUALWXGA_S   (2560 x  768),
	{1600 , 1280},// 0x12 : UXGA         (1600 x 1280),
	{2560 ,  800},// 0x13 : DUALWXGA     (2560 x  800),
	{1440 , 1440},// 0x14 : 1440R        (1440 x 1440),
	{1920 , 1080},// 0x15 : FHD          (1920 x 1080),
	{2048 , 1024},// 0x16 : 2K1K         (2048 x 1024),
	{2560 , 1440},// 0x17 : WQHD         (2560 x 1440),
	{2560 , 1600},// 0x18 : WQXGA        (2560 x 1600),
	{1088 , 3840},// 0x19 : RFU0         (1088 x 3840),
	{2048 , 2048},// 0x1a : 2K2K         (2048 x 2048),
	{1280 ,  728},// 0x1b : 1280X728     (1280 x  728),
	{1024 , 4096},// 0x1c : RFU2         (1024 x 4096),
	{1024 , 4096},// 0x1d : RFU3         (1024 x 4096),
	{1024 , 4096},// 0x1e : RFU4         (1024 x 4096),
	{1024 , 4096},// 0x1f : RFU5         (1024 x 4096),
	{ 480 ,  640},// 0x20 : PVGA         ( 480 x  640)
	{ 480 ,  856},// 0x21 : PWVGA        ( 480 x  856)
	{ 544 ,  960},// 0x22 : PQFHD        ( 544 x  960)
	{ 608 , 1024},// 0x23 : PWSVGA       ( 608 x 1024)
	{ 768 , 1024},// 0x24 : PXGA         ( 768 x 1024)
	{ 720 , 1280},// 0x25 : PHD          ( 720 x 1280)
	{ 768 , 1280},// 0x26 : PWXGA_S      ( 768 x 1280)
	{ 800 , 1280},// 0x27 : PWXGA        ( 800 x 1280)
	{1024 , 1024},// 0x28 : RFU6         (1024 x 1024)
	{1088 , 1080},// 0x29 : RFU7         (1088 x 1080)
	{ 960 , 1280},// 0x2a : PMLO         ( 960 x 1280)
	{1024 , 1280},// 0x2b : PSXGA        (1024 x 1280)
	{ 912 , 1600},// 0x2c : 900X1600    ( 912 x 1600)
	{1088 , 1440},// 0x2d : 1080X1440   (1088 x 1440)
	{1280 , 1280},// 0x2e : RFU8         (1280 x 1280)
	{ 720 , 2560},// 0x2f : PDUALHD      ( 720 x 2560)
	{1200 , 1600},// 0x30 : PUXGA_S      (1200 x 1600)
	{ 768 , 2560},// 0x31 : PDUALWXGA_S  ( 768 x 2560)
	{1200 , 1600},// 0x32 : PUXGA        (1200 x 1600)
	{ 800 , 2560},// 0x33 : PDUALWXGA    ( 800 x 2560)
	{1440 , 1440},// 0x34 : RFU9         (1440 x 1440)
	{1088 , 1920},// 0x35 : PFHD         (1088 x 1920)
	{1024 , 2048},// 0x36 : P2K1K        (1024 x 2048)
	{1440 , 2560},// 0x37 : PWQHD        (1440 x 2560)
	{1600 , 2560},// 0x38 : PWQXGA       (1600 x 2560)
	{1088 , 3840},// 0x39 : PDUALFHD     (1088 x 3840)
	{2048 , 2048},// 0x3a : RFUA         (2048 x 2048)
	{1024 , 4096},// 0x3b : RFUB         (1024 x 4096)
	{1024 , 4096},// 0x3c : RFUC         (1024 x 4096)
	{1024 , 4096},// 0x3d : RFUD         (1024 x 4096)
	{1024 , 4096},// 0x3e : RFUE         (1024 x 4096)
	{0, 0}        // 0x3f : ETC (TX_OUTPUT_H_SIZE_IDX)
};

static const RDC200A_RESOLUTION_INFO_T rdc200a_input_res[] = {
	{ 640 ,  480},// 0x00 : VGA          ( 640 x  480),
	{ 854 ,  480},// 0x01 : WVGA         ( 854 x  480),
	{ 960 ,  540},// 0x02 : QFHD         ( 960 x  540),
	{1024 ,  600},// 0x03 : WSVGA        (1024 x  600),
	{1024 ,  768},// 0x04 : XGA          (1024 x  768),
	{1280 ,  720},// 0x05 : HD           (1280 x  720),
	{1280 ,  768},// 0x06 : WXGA_S       (1280 x  768),
	{1280 ,  800},// 0x07 : WXGA         (1280 x  800),
	{1024 , 1024},// 0x08 : 1K1K         (1024 x 1024),
	{1080 , 1080},// 0x09 : 1080R        (1080 x 1080),
	{1280 ,  960},// 0x0a : MLO          (1280 x  960),
	{1280 , 1024},// 0x0b : SXGA         (1280 x 1024),
	{1600 ,  900},// 0x0c : 1600X900     (1600 x  900),
	{1440 , 1080},// 0x0d : 1440X1080    (1440 x 1080),
	{1280 , 1280},// 0x0e : 1280R        (1280 x 1280),
	{2560 ,  720},// 0x0f : DUALHD       (2560 x  720),
	{1600 , 1200},// 0x10 : UXGA_S       (1600 x 1200),
	{2560 ,  768},// 0x11 : DUALWXGA_S   (2560 x  768),
	{1600 , 1280},// 0x12 : UXGA         (1600 x 1280),
	{2560 ,  800},// 0x13 : DUALWXGA     (2560 x  800),
	{1440 , 1440},// 0x14 : 1440R        (1440 x 1440),
	{1920 , 1080},// 0x15 : FHD          (1920 x 1080),
	{2048 , 1024},// 0x16 : 2K1K         (2048 x 1024),
	{2560 , 1440},// 0x17 : WQHD         (2560 x 1440),
	{2560 , 1600},// 0x18 : WQXGA        (2560 x 1600),
	{1080 , 3840},// 0x19 : RFU0         (1080 x 3840),
	{2048 , 2048},// 0x1a : 2K2K         (2048 x 2048),
	{1280 ,  728},// 0x1b : 1280X728     (1280 x  728),
	{1024 , 4096},// 0x1c : RFU2         (1024 x 4096),
	{1024 , 4096},// 0x1d : RFU3         (1024 x 4096),
	{1024 , 4096},// 0x1e : RFU4         (1024 x 4096),
	{1024 , 4096},// 0x1f : RFU5         (1024 x 4096),
	{ 480 ,  640},// 0x20 : PVGA         ( 480 x  640)
	{ 480 ,  854},// 0x21 : PWVGA        ( 480 x  854)
	{ 540 ,  960},// 0x22 : PQFHD        ( 540 x  960)
	{ 600 , 1024},// 0x23 : PWSVGA       ( 600 x 1024)
	{ 768 , 1024},// 0x24 : PXGA         ( 768 x 1024)
	{ 720 , 1280},// 0x25 : PHD          ( 720 x 1280)
	{ 768 , 1280},// 0x26 : PWXGA_S      ( 768 x 1280)
	{ 800 , 1280},// 0x27 : PWXGA        ( 800 x 1280)
	{1024 , 1024},// 0x28 : RFU6         (1024 x 1024)
	{1080 , 1080},// 0x29 : RFU7         (1080 x 1080)
	{ 960 , 1280},// 0x2a : PMLO         ( 960 x 1280)
	{1024 , 1280},// 0x2b : PSXGA        (1024 x 1280)
	{ 900 , 1600},// 0x2c : 900X1600    ( 900 x 1600)
	{1080 , 1440},// 0x2d : 1080X1440   (1080 x 1440)
	{1280 , 1280},// 0x2e : RFU8         (1280 x 1280)
	{ 720 , 2560},// 0x2f : PDUALHD      ( 720 x 2560)
	{1200 , 1600},// 0x30 : PUXGA_S      (1200 x 1600)
	{ 768 , 2560},// 0x31 : PDUALWXGA_S  ( 768 x 2560)
	{1200 , 1600},// 0x32 : PUXGA        (1200 x 1600)
	{ 800 , 2560},// 0x33 : PDUALWXGA    ( 800 x 2560)
	{1440 , 1440},// 0x34 : RFU9         (1440 x 1440)
	{1080 , 1920},// 0x35 : PFHD         (1080 x 1920)
	{1024 , 2048},// 0x36 : P2K1K        (1024 x 2048)
	{1440 , 2560},// 0x37 : PWQHD        (1440 x 2560)
	{1600 , 2560},// 0x38 : PWQXGA       (1600 x 2560)
	{1080 , 3840},// 0x39 : PDUALFHD     (1080 x 3840)
	{2048 , 2048},// 0x3a : RFUA         (2048 x 2048)
	{1024 , 4096},// 0x3b : RFUB         (1024 x 4096)
	{1024 , 4096},// 0x3c : RFUC         (1024 x 4096)
	{1024 , 4096},// 0x3d : RFUD         (1024 x 4096)
	{1024 , 4096},// 0x3e : RFUE         (1024 x 4096)
	{0, 0}        // 0x3f : ETC (INPUT_H_SIZE, INPUT_V_SIZE)
};

static U16_T rdc200a_output_h_size_idx_val[0x15] = {
	0, 544, 608, 640, 720, 0, 768, 800, 864, 912,
	960, 1024, 0, 1088, 1200, 1280, 1440, 1600, 1856, 1920,
	2048
};

static U16_T rdc200a_output_v_size_idx_val[0x17] = {
	480, 544, 600, 640, 720, 728, 768, 800, 854, 904,
	960, 1024, 1048, 1080, 1200, 1280, 1440, 1600, 0, 1920,
	2048, 2560, 3840
};

static BOOL_T rdc200a_check_pll_lock(U8_T pll_idx)
{
	U8_T reg_1A;
	U8_T lock_mask = (1<<pll_idx) << 2; // <0> : PLL0 lock, <1> : PLL1 lock, <2> : PLL2 lock
	U8_T lock_timeout = 50;

//	DMSG("pll_idx(%bu), lock_mask(0x%bX)\n", (U8_T)pll_idx, (U8_T)lock_mask);

	do {
		reg_1A = RDC_REG_GET(0x001A); // monitoring_basic[4:2]
		if (reg_1A & lock_mask)
			return TRUE;

		VC_U_DELAY(1);
	} while (--lock_timeout);

	EMSG("PLL_(%d) timeout\n", pll_idx);

	return FALSE;
}

/**
 Firmware enabled TX_PANEL_EN and PDRV_TX_AUTO_EN bits for the specified port
 when boot time to use auto ON.
*/
static int rdc200a_drive_panel(BOOL_T on)
{
	U8_T rd0015;

	rd0015 = RDC_REG_GET(0x0015);

	if (on == TRUE)
		rd0015 |= ((rd0015 & 0x03) << 2);
	else
		rd0015 &= 0xF3;

	RDC_REG_SET(0x0015, rd0015);

	return 0;
}

static void rdc200a_dcdc_disable(void)
{
	U8_T reg_0x0019;

	reg_0x0019 = RDC_REG_GET(0x0019);
	if (reg_0x0019 & 0x02)
		reg_0x0019 |= 0x01; // Low active power down
	else
		reg_0x0019 &= ~0x01; // High active power down

	RDC_REG_SET(0x0019, reg_0x0019); // dcdc disable

	VC_M_DELAY(1);
}

static void rdc200a_dcdc_enable(void)
{
	U8_T reg_0x0019;

	reg_0x0019 = RDC_REG_GET(0x0019);
	if (reg_0x0019 & 0x02)
		reg_0x0019 &= ~0x01; // Low active power down
	else
		reg_0x0019 |= 0x01; // High active power down

	/*
	 <0> : DCDC power down manual control
   		'0' : DCDC active ( GPIO outputs low )
		'1' : DCDC power down ( GPIO outputs high ) <default>
	*/
	RDC_REG_SET(0x0019, reg_0x0019); // dcdc disable

	VC_M_DELAY(1);
}

int rdc200a_poweron_panel(BOOL_T on)
{
	U8_T PLL2_REFDIV = 0x0, PLL2_PLLN_lsb = 0x0;
	U8_T TX_T2P_EN[2] = {0, 0};

	if (on == TRUE) {
		PLL2_REFDIV = RDC_REG_GET(0x47);
		PLL2_PLLN_lsb = RDC_REG_GET(0x49);
		RDC_REG_MASK_SET(0x47, 0x03, 0x01);
		RDC_REG_SET(0x49, 0x6E);
		rdc200a_check_pll_lock(2);

		TX_T2P_EN[0] = RDC_REG_GET(0x0822) & 0x08;
		TX_T2P_EN[1] = RDC_REG_GET(0x0922) & 0x08;
		if (TX_T2P_EN[0] || TX_T2P_EN[1]) {
			RDC_REG_MASK_SET(0x0013, 0x02, 0x00);
			RDC_REG_MASK_SET(0x0013, 0x02, 0x02);
		}

		rdc200a_drive_panel(TRUE);

		RDC_REG_SET(0x47, PLL2_REFDIV);
		RDC_REG_SET(0x49, PLL2_PLLN_lsb);
		rdc200a_check_pll_lock(2);

		rdc200a_dcdc_enable();
	}
	else {
		rdc200a_dcdc_disable();
		rdc200a_drive_panel(FALSE);
	}

	return 0;
}

/**
 faddr: FLASH address
 */
int rdc200a_update_lut_from_flash(U32_T faddr, U32_T size, U8_T id)
{
	int ret = 0;
	U32_T timeout = 100000; // Max 100ms
	U8_T read_data = 0x00;

	if (!size) {
		//EMSG("Invalid LUT size\n");
		ret = -1;
		goto up_lut_err;
	}

	//size--;

	RDC_REG_SET(0x0051, 0x01); // Flash to LUT enable
	RDC_REG_SET(0x0059, 0x00); // Flash to LUT model state machine initialize
	RDC_REG_SET(0x0052, id);

	RDC_REG_SET(0x0056, (size>>16) & 0xFF);
	RDC_REG_SET(0x0057, (size>>8) & 0xFF);
	RDC_REG_SET(0x0058, size & 0xFF);

	RDC_REG_SET(0x0053, (faddr>>16) & 0xFF);
	RDC_REG_SET(0x0054, (faddr>>8) & 0xFF);
	RDC_REG_SET(0x0055, faddr & 0xFF);

	RDC_REG_SET(0x0059, 0x01); // Start state machine

	/* Wait update LUT */
	while (1) {
		read_data = RDC_REG_GET(0x0092);
		if (read_data & 0x10)
			break;

		if (--timeout == 0) {
			//EMSG("F_LUT update timeout error!\n");
			ret = -2;
			goto up_lut_err;
		}

		VC_U_DELAY(1);
	}

    return 0;

up_lut_err:
	EMSG("F_LUT update err(%d)\n", ret);

	return ret;
}

int rdc200a_update_gamma_lut(U8_T port, U8_T lut_offset)
{
	U32_T base_addr, lut_addr;
	const RDC200A_GAMMA_LUT_ID_INFO_T *lut_id_ptr;

	lut_id_ptr = &rdc200a_gamma_lut_id[port];

	base_addr = FLASH_PART_OFFSET_PANEL_GAMMA_BASE;

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_HIGH_RED(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
							lut_id_ptr->RED_POL_H);

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_HIGH_GREEN(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
							lut_id_ptr->GREEN_POL_H);

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_HIGH_BLUE(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
							lut_id_ptr->BLUE_POL_H);

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_LOW_RED(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
							lut_id_ptr->RED_POL_L);

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_LOW_GREEN(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
							lut_id_ptr->GREEN_POL_L);

	lut_addr = base_addr + FLASH_ITEM_OFFSET_GAMMA_LOW_BLUE(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_SUBITEM_SIZE_PANNEL_GAMMA,
						lut_id_ptr->BLUE_POL_L);

	return 0;
}

static int rdc200a_enable_gamma_correction(E_VC_PANEL_CTRL_PORT_T ctrl_port,
										BOOL_T enable,
										UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT])
{
	U8_T gamma_en_val = 0x0;
	UINT_T gamma_en_state[2];
	UINT_T panel_offon_mask = 0x0;

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		gamma_en_state[0] = RDC_REG_GET16(0x0873) & 0x1;

		if (enable == TRUE) {
			if (gamma_en_state[0] == 0) // Is gamma disabled for the port 0?
				panel_offon_mask = VC_PANEL_CTRL_PORT_0;
		}
		else {
			if (gamma_en_state[0] == 1) // Is gamma enabled for the port 1?
				panel_offon_mask = VC_PANEL_CTRL_PORT_0;
		}
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		gamma_en_state[1] = RDC_REG_GET16(0x0973) & 0x1;

		if (enable == TRUE) {
			if (gamma_en_state[1] == 0) // Is gamma disabled for the port 1?
				panel_offon_mask |= VC_PANEL_CTRL_PORT_1;
		}
		else {
			if (gamma_en_state[1] == 1) // Is gamma enabled for the port 1?
				panel_offon_mask |= VC_PANEL_CTRL_PORT_1;
		}
	}

	if (enable == TRUE) {
		/*
		 Load the LUT first to reduce the time difference between port 0 and 1.
		*/
		if (ctrl_port & VC_PANEL_CTRL_PORT_0)
			rdc200a_update_gamma_lut(0, lut_offset[0]);

		if (ctrl_port & VC_PANEL_CTRL_PORT_1)
			rdc200a_update_gamma_lut(1, lut_offset[1]);

		gamma_en_val = 0x1;
	}

	if (panel_offon_mask)
		rdc200a_drive_panel(FALSE);

	if (ctrl_port & VC_PANEL_CTRL_PORT_0)
		RDC_REG_MASK_SET(0x0873, 0x01, gamma_en_val);

	if (ctrl_port & VC_PANEL_CTRL_PORT_1)
		RDC_REG_MASK_SET(0x0973, 0x01, gamma_en_val);

	if (panel_offon_mask)
		rdc200a_drive_panel(TRUE);

	return 0;
}

void rdc200a_update_cokoa_lut(U8_T port, U8_T lut_offset, U8_T ca_en)
{
	U32_T base_addr, lut_addr;
	U8_T lut_id[MAX_NUM_VC_PANEL_PORT][3] = { // RGB for each port.
		{TSID_TX0_CoKOA_RED, TSID_TX0_CoKOA_GREEN, TSID_TX0_CoKOA_BLUE},
		{TSID_TX1_CoKOA_RED, TSID_TX1_CoKOA_GREEN, TSID_TX1_CoKOA_BLUE}
	};

	base_addr = FLASH_PART_OFFSET_DC;

	lut_addr = base_addr + FLASH_ITEM_OFFSET_DC_RED(lut_offset);
	rdc200a_update_lut_from_flash(lut_addr, FLASH_ITEM_SIZE_DC, lut_id[port][0]);

	if (ca_en == TRUE) {
		lut_addr = base_addr + FLASH_ITEM_OFFSET_DC_GREEN(lut_offset);
		rdc200a_update_lut_from_flash(lut_addr, FLASH_ITEM_SIZE_DC, lut_id[port][1]);

		lut_addr = base_addr + FLASH_ITEM_OFFSET_DC_BLUE(lut_offset);
		rdc200a_update_lut_from_flash(lut_addr, FLASH_ITEM_SIZE_DC, lut_id[port][2]);
	}
}

static int rdc200a_enable_cokoa(E_VC_PANEL_CTRL_PORT_T ctrl_port, BOOL_T dc_en,
						UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT], BOOL_T ca_en)
{
	UINT_T cokoa_en_state[2];
	UINT_T panel_offon_mask = 0x0;

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		cokoa_en_state[0] = RDC_REG_GET16(0x0871) & 0x1;

		if (dc_en == TRUE) {
			if (cokoa_en_state[0] == 0) // Is cokoa disabled for the specified port?
				panel_offon_mask = VC_PANEL_CTRL_PORT_0;
		}
		else {
			if (cokoa_en_state[0] == 1) // Is cokoa enabled for the specified port?
				panel_offon_mask = VC_PANEL_CTRL_PORT_0;
		}
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		cokoa_en_state[1] = RDC_REG_GET16(0x0971) & 0x1;

		if (dc_en == TRUE) {
			if (cokoa_en_state[1] == 0) // Is cokoa disabled for the specified port?
				panel_offon_mask |= VC_PANEL_CTRL_PORT_1;
		}
		else {
			if (cokoa_en_state[1] == 1) // Is cokoa enabled for the specified port?
				panel_offon_mask |= VC_PANEL_CTRL_PORT_1;
		}
	}

	if (dc_en == TRUE) {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
			RDC_REG_MASK_SET(0x0870, 0x80, 0x80);
			RDC_REG_MASK_SET(0x0870, 0x40, 0x00);
			RDC_REG_SET(0x0871, 0x1 | (ca_en<<3));

			rdc200a_update_cokoa_lut(0, lut_offset[0], ca_en);
		}

		if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
			RDC_REG_MASK_SET(0x0970, 0x80, 0x80);
			RDC_REG_MASK_SET(0x0970, 0x40, 0x00);
			RDC_REG_SET(0x0971, 0x1 | (ca_en<<3));

			rdc200a_update_cokoa_lut(1, lut_offset[1], ca_en);
		}
	}
	else {
		if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
			if (cokoa_en_state[0] == 1) {
				RDC_REG_MASK_SET(0x0871, 0x01, 0x00); // TX0_COKOA_EN
				RDC_REG_MASK_SET(0x0870, 0x40, 0x40); // TX0_COKOA_ISO_CTRL
				RDC_REG_MASK_SET(0x0870, 0x80, 0x00); // TX0_COKOA_PSW_CTRL
			}
		}

		if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
			if (cokoa_en_state[1] == 1) {
				RDC_REG_MASK_SET(0x0971, 0x01, 0x00); // TX1_COKOA_EN
				RDC_REG_MASK_SET(0x0970, 0x40, 0x40); // TX1_COKOA_ISO_CTRL
				RDC_REG_MASK_SET(0x0970, 0x80, 0x00); // TX1_COKOA_PSW_CTRL
			}
		}
	}

	if (vc_panel_attached_port_mask == VC_PANEL_CTRL_PORT_1) {
		U16_T addr, val, add = 0;
		U8_T led_lsb, led_msb;

		if (cokoa_en_state[1] == 1)
			add = 64;

		for (addr = 0x0853; addr <= 0x086A; addr += 2) {
			led_msb = RDC_REG_GET(addr + 0x100); // Get the led from port 1.
			led_lsb = RDC_REG_GET((addr + 0x100)+1);
			val = ((U16_T)led_msb<<8) | led_lsb;
			val += add;

			RDC_REG_SET(addr, (U8_T)(val >> 8));
			RDC_REG_SET(addr+1, val & 0xFF);
		}
	}

	if (panel_offon_mask) {
		rdc200a_drive_panel(FALSE);
		rdc200a_drive_panel(TRUE);
	}

	return 0;
}

static int rdc200a_get_output_resolution(U32_T *h_size, U32_T *v_size)
{
	UINT_T tx_port;
	U8_T output_size_idx;
	U8_T output_h_size_idx, output_v_size_idx;
	U16_T h_temp_size = 0, v_temp_size = 0;
	U16_T output_size_idx_reg[2] = {0x0827, 0x0927};
	U16_T output_size_h_idx_reg[2] = {0x0875, 0x0975};
	U16_T output_size_v_idx_reg[2] = {0x0876, 0x0976};
	U16_T output_h_size_reg[2] = {0x828, 0x928};
	U16_T output_v_size_msb_reg[2] = {0x829, 0x929};
	U16_T output_v_size_lsb_reg[2] = {0x82A, 0x92A};

	if (vc_panel_attached_port_mask & VC_PANEL_CTRL_PORT_0)
		tx_port = 0;
	else
		tx_port = 1;

	output_size_idx = RDC_REG_GET(output_size_idx_reg[tx_port]) >> 2; // <7:2>

	if (output_size_idx != 0x3F) {
		*h_size = rdc200a_output_res[output_size_idx].h_size;
		*v_size = rdc200a_output_res[output_size_idx].v_size;
	}
	else { // Manual
		output_h_size_idx = RDC_REG_GET(output_size_h_idx_reg[tx_port]) & 0x1F;
		if (output_h_size_idx < 0x15)
			h_temp_size = rdc200a_output_h_size_idx_val[output_h_size_idx];

		if (h_temp_size == 0) /* unit : 16 pixels */
			h_temp_size = (U16_T)RDC_REG_GET(output_h_size_reg[tx_port]) << 4;

		output_v_size_idx = RDC_REG_GET(output_size_v_idx_reg[tx_port]) & 0x1F;
		if (output_v_size_idx < 0x17)
			v_temp_size = rdc200a_output_v_size_idx_val[output_v_size_idx];

		if (v_temp_size == 0) {
			v_temp_size = (U16_T)(RDC_REG_GET(output_v_size_msb_reg[tx_port]) & 0x3) << 8;
			v_temp_size |= (U16_T)RDC_REG_GET(output_v_size_lsb_reg[tx_port]);
			v_temp_size <<= 3; /* unit : 8 lines */
		}

		*h_size = h_temp_size;
		*v_size = v_temp_size;
	}

	return 0;
}

static BOOL_T rdc200a_check_text_position(U32_T *h_output_res,
										U32_T *v_output_res,
										U32_T x, U32_T y,
										E_VC_OSD_TEXT_MAG_T mag,
										E_VC_OSD_DIMENSTION_T d)
{
	U8_T dimension_reg_val;
	U32_T output_h_size, output_v_size;
	U32_T font_h_size, font_v_size;

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	font_h_size = rdc200a_font_osd_mag_size[mag].h_size;
	font_v_size = rdc200a_font_osd_mag_size[mag].v_size;

	if (d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) {
			output_h_size >>= 1; // H Divide by 2 (Side By Side)
		}
		else if (dimension_reg_val & TAB3D_EN) {
			output_v_size >>= 1; // V Divide by 2 (Top and Bottom)
		}
		else {
		}
	}

	*h_output_res = output_h_size;
	*v_output_res = output_v_size;

	if (((x+font_h_size) < output_h_size) && ((y+font_v_size) < output_v_size))
		return TRUE;
	else
		return FALSE;

	return 0;
}

static int rdc200a_disable_text_osd(VC_TEXT_DRAWER_CB *cb)
{
	U8_T ftdrw_en_mask, ftdrw_en_3d_mask;
	U8_T drawer_id = cb->drawer_id;

	ftdrw_en_mask = 1 << drawer_id;
	ftdrw_en_3d_mask = (1 << drawer_id) << 4;

	RDC_REG_MASK_SET(0x378, ftdrw_en_mask | ftdrw_en_3d_mask, 0x0);

	return 0;
}

static int rdc200a_hide_text_osd(VC_TEXT_DRAWER_CB *cb)
{
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, (U8_T)VC_OSD_ALPHA_0);

	return 0;
}

static int rdc200a_show_text_osd(VC_TEXT_DRAWER_CB *cb)
{
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, ((U8_T)cb->back_alpha<<4)|(U8_T)cb->font_alpha);

	return 0;
}

static int rdc200a_change_dimension_text_osd(VC_TEXT_DRAWER_CB *cb,
											E_VC_OSD_DIMENSTION_T d)
{
	U8_T ftdrw_en_mask, ftdrw_en_3d_mask, dimension_reg_val, en_reg_val;
	U32_T font_pos_x, font_pos_y, font_3d_pos_x, font_3d_pos_y;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;

	ftdrw_en_mask = 1 << drawer_id;
	ftdrw_en_3d_mask = (1 << drawer_id) << 4;

	en_reg_val = ftdrw_en_mask; /* Make the value of 2D enable in default */

	font_pos_x = cb->pos_x; // 1 pixel unit
	font_pos_y = cb->pos_y; // 1 pixel unit

	if (d == VC_OSD_DIMENSTION_3D) {
		if (!rdc200a_check_text_position(&output_h_size, &output_v_size,
										cb->pos_x, cb->pos_y, cb->mag, d)) {
			EMSG("Invalid dimension to change.\n");
			return -1;
		}

		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) { // Side By Side
			font_3d_pos_x = font_pos_x + output_h_size; // 1 pixel unit
		}
		else if (dimension_reg_val & TAB3D_EN) {
			font_3d_pos_y = font_pos_y + output_v_size; // 1 pixel unit
		}
		else {
		}

		reg_addr = RDC200A_3D_FONT_POS_X_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_x >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_x & 0xFF);

		reg_addr = RDC200A_3D_FONT_POS_Y_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_y >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_y & 0xFF);

		en_reg_val |= ftdrw_en_3d_mask;
	}

	/* Update drawer */
	RDC_REG_MASK_SET(0x0378, ftdrw_en_mask|ftdrw_en_3d_mask, en_reg_val);

	return 0;
}

static int rdc200a_change_property_text_osd(VC_TEXT_DRAWER_CB *cb,
										E_VC_OSD_TEXT_MAG_T mag,
										E_VC_OSD_ALPHA_T font_alpha,
										E_VC_OSD_ALPHA_T back_alpha)
{
	U32_T output_h_size, output_v_size;
	U8_T mag_x, mag_y, dimension_reg_val;
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;

	if (!rdc200a_check_text_position(&output_h_size, &output_v_size,
									cb->pos_x, cb->pos_y, mag, cb->d)) {
		EMSG("Invalid text size to drawing\n");
		return -1;
	}

	mag_x = (U8_T)mag;
	mag_y = (U8_T)mag;

	if (cb->d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) // Side By Side
			mag_y = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of X
		else if (dimension_reg_val & TAB3D_EN)
			mag_x = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of Y
		else {
		}
	}

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, ((U8_T)back_alpha<<4)|(U8_T)font_alpha);

	reg_addr = RDC200A_FONT_MAG_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, (mag_y<<4)|mag_x);

	return 0;
}

static int rdc200a_move_text_osd(VC_TEXT_DRAWER_CB *cb, U32_T mov_x, U32_T mov_y)
{
	U32_T font_pos_x, font_pos_y, font_3d_pos_x, font_3d_pos_y;
	U8_T dimension_reg_val;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;

	if (!rdc200a_check_text_position(&output_h_size, &output_v_size,
									mov_x, mov_y, cb->mag, cb->d)) {
		EMSG("Invalid text size to drawing\n");
		return -1;
	}

	font_pos_x = mov_x;
	font_pos_y = mov_y;

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, 0x0); // alpha 0%

	/* Frist, Move 2D text. */
	reg_addr = RDC200A_2D_FONT_POS_X_MSB_REG_ADDR(drawer_id); // MSB reg addr
	RDC_REG_SET(reg_addr, (U8_T)(font_pos_x >> 8));
	RDC_REG_SET(reg_addr+1, font_pos_x & 0xFF);

	reg_addr = RDC200A_2D_FONT_POS_Y_MSB_REG_ADDR(drawer_id); // MSB reg addr
	RDC_REG_SET(reg_addr, (U8_T)(font_pos_y >> 8));
	RDC_REG_SET(reg_addr+1, font_pos_y & 0xFF);

	if (cb->d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) { // Side By Side
			font_3d_pos_x = font_pos_x + output_h_size; // 1 pixel unit
		}
		else if (dimension_reg_val & TAB3D_EN) {
			font_3d_pos_y = font_pos_y + output_v_size; // 1 pixel unit
		}
		else {
		}

		reg_addr = RDC200A_3D_FONT_POS_X_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_x >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_x & 0xFF);

		reg_addr = RDC200A_3D_FONT_POS_Y_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_y >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_y & 0xFF);
	}

	/* Restore alpha */
	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, ((U8_T)cb->back_alpha<<4)|(U8_T)cb->font_alpha);

	return 0;
}

static int rdc200a_draw_text_osd(VC_TEXT_DRAWER_CB *cb, U32_T pos_x, U32_T pos_y,
									U32_T font_addr, U32_T palette_addr,
									E_VC_OSD_TEXT_MAG_T mag,
									E_VC_OSD_ALPHA_T font_alpha,
									E_VC_OSD_ALPHA_T back_alpha,
									U8_T font_color_idx, U8_T back_color_idx,
									E_VC_OSD_DIMENSTION_T d,
									const char *string)
{
	U8_T ftdrw_en_mask, ftdrw_en_3d_mask, dimension_reg_val, en_reg_val;
	U32_T font_pos_x, font_pos_y, font_3d_pos_x, font_3d_pos_y;
	U8_T mag_x, mag_y;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr;
	U8_T drawer_id = cb->drawer_id;
	size_t i, num_char = strlen(string);

	if (num_char > RDC200A_MAX_NUM_FONT_CHAR_PER_BOX) {
		EMSG("Must be less than %d characters\n", RDC200A_MAX_NUM_FONT_CHAR_PER_BOX);
		return -1;
	}

	if (!rdc200a_check_text_position(&output_h_size, &output_v_size,
										pos_x, pos_y, mag, d)) {
		EMSG("Invalid text size to drawing\n");
		return -2;
	}

	if (font_color_idx > 15) {
		EMSG("Font color must be from 0 to 15\n");
		return -3;
	}

	if (back_color_idx > 15) {
		EMSG("Background color must be from 0 to 15\n");
		return -4;
	}

	if (d == VC_OSD_DIMENSTION_3D) {
		if (mag == VC_TEXT_OSD_MAG_x8) {
			EMSG("Invaild magnification to draw 3D text\n");
			return -5;
		}
	}

	ftdrw_en_mask = 1 << drawer_id;
	ftdrw_en_3d_mask = (1 << drawer_id) << 4;

	en_reg_val = ftdrw_en_mask; /* Make the value of 2D enable in default */

	font_pos_x = pos_x; // 1 pixel unit
	font_pos_y = pos_y; // 1 pixel unit

	mag_x = (U8_T)mag;
	mag_y = (U8_T)mag;

	if (font_addr != 0xFFFFFFFF)
		rdc200a_update_lut_from_flash(font_addr, FLASH_ITEM_SIZE_FONT,
							rdc200a_font_drawer_sram_id[drawer_id]);

	if (palette_addr != 0xFFFFFFFF)
		rdc200a_update_lut_from_flash(palette_addr, 96,
							rdc200a_palette_drawer_sram_id[drawer_id]);

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, ((U8_T)back_alpha<<4)|(U8_T)font_alpha);

	reg_addr = RDC200A_FONT_MAG_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, (mag_y<<4)|mag_x);

	/* 2D position. */
	reg_addr = RDC200A_2D_FONT_POS_X_MSB_REG_ADDR(drawer_id); // MSB reg addr
	RDC_REG_SET(reg_addr, (U8_T)(font_pos_x >> 8));
	RDC_REG_SET(reg_addr+1, font_pos_x & 0xFF);

	reg_addr = RDC200A_2D_FONT_POS_Y_MSB_REG_ADDR(drawer_id); // MSB reg addr
	RDC_REG_SET(reg_addr, (U8_T)(font_pos_y >> 8));
	RDC_REG_SET(reg_addr+1, font_pos_y & 0xFF);

	if (d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) { // Side By Side
			font_3d_pos_x = font_pos_x + output_h_size; // 1 pixel unit

			mag_y = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of X
		}
		else if (dimension_reg_val & TAB3D_EN) {
			font_3d_pos_y = font_pos_y + output_v_size; // 1 pixel unit

			mag_x = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of Y
		}
		else {
		}

		reg_addr = RDC200A_3D_FONT_POS_X_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_x >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_x & 0xFF);

		reg_addr = RDC200A_3D_FONT_POS_Y_MSB_REG_ADDR(drawer_id); // MSB reg addr
		RDC_REG_SET(reg_addr, (U8_T)(font_3d_pos_y >> 8));
		RDC_REG_SET(reg_addr+1, font_3d_pos_y & 0xFF);

		en_reg_val |= ftdrw_en_3d_mask;
	}

	/* Enable drawer */
	RDC_REG_MASK_SET(0x0378, ftdrw_en_mask|ftdrw_en_3d_mask, en_reg_val);

	/* Select drawer. */
	RDC_REG_SET(0x0379, (U8_T)(1 << drawer_id));

	RDC_REG_SET(0x037A, (back_color_idx<<4)|font_color_idx); // color data
	RDC_REG_SET(0x037B, 0x18); // font_fsm command : INST_CHAR_COLOR
	RDC_REG_SET(0x037C, 0x01); // font_fsm command : idle

	/* Clear text box. */
	RDC_REG_SET(0x037B, 0x11); // font_fsm command : clear
	RDC_REG_SET(0x037C, 0x01); // font_fsm command : idle

	/* Set string. */
	for (i = 0; i < num_char; i++) {
		RDC_REG_SET(0x037A, string[i]);
		RDC_REG_SET(0x037B, 0x14); // font_fsm command : put_char
		RDC_REG_SET(0x037C, 0x01); // font_fsm command : idle
	}

	return 0;
}

static BOOL_T rdc200a_check_bitmap_position(U32_T *h_output_res,
										U32_T *v_output_res,
										U32_T x, U32_T y,
										E_VC_OSD_BITMAP_MAG_T mag,
										E_VC_OSD_DIMENSTION_T d)
{
	U32_T output_h_size, output_v_size;
	U32_T bmp_h_size, bmp_v_size;
	U8_T dimension_reg_val;

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	bmp_h_size = rdc200a_bmp_osd_mag_size[mag].h_size;
	bmp_v_size = rdc200a_bmp_osd_mag_size[mag].v_size;

	if (d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) {
			output_h_size >>= 1; // H Divide by 2 (Side By Side)
		}
		else if (dimension_reg_val & TAB3D_EN) {
			output_v_size >>= 1; // V Divide by 2 (Top and Bottom)
		}
		else {
		}
	}

	*h_output_res = output_h_size;
	*v_output_res = output_v_size;

	if (((x+bmp_h_size) < output_h_size) && ((y+bmp_v_size) < output_v_size))
		return TRUE;
	else
		return FALSE;
}

static int rdc200a_disable_bitmap_osd(VC_BITMAP_DRAWER_CB *cb)
{
	U8_T bmpdr_en, bmpdr_en_3d;
	U8_T drawer_id = cb->drawer_id;

	bmpdr_en = 1 << drawer_id;
	bmpdr_en_3d = (1 << drawer_id) << 4;

	RDC_REG_MASK_SET(0x346, bmpdr_en | bmpdr_en_3d, 0x0);

	return 0;
}

static int rdc200a_change_dimension_bitmap_osd(VC_BITMAP_DRAWER_CB *cb,
												E_VC_OSD_DIMENSTION_T d)
{
	U8_T bmpdr_en, bmpdr_en_3d;
	U32_T bmp_pos_x, bmp_pos_y;
	U8_T dimension_reg_val;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr_msb;

	bmpdr_en = 1 << cb->drawer_id;
	bmpdr_en_3d = (1 << cb->drawer_id) << 4;

	if (d == VC_OSD_DIMENSTION_3D) {
		if (!rdc200a_check_bitmap_position(&output_h_size, &output_v_size,
											cb->pos_x, cb->pos_y, cb->mag, d)) {
			EMSG("Invalid bitmap size to drawing\n");
			return -1;
		}

		bmp_pos_x = cb->pos_x; // 1 pixel unit
		bmp_pos_y = cb->pos_y; // 1 pixel unit

		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) {
			bmp_pos_x += output_h_size; // 1 pixel unit
		}
		else if (dimension_reg_val & TAB3D_EN) {
			bmp_pos_y += output_v_size; // 1 pixel unit
		}
		else {
		}

		reg_addr_msb = RDC200A_3D_BMP_POS_X_MSB_REG_ADDR(cb->drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_x >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_x & 0xFF);

		reg_addr_msb = RDC200A_3D_BMP_POS_Y_MSB_REG_ADDR(cb->drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_y >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_y & 0xFF);

		RDC_REG_MASK_SET(0x346, bmpdr_en_3d, bmpdr_en_3d);
	}
	else
		RDC_REG_MASK_SET(0x346, bmpdr_en_3d, 0x0);

	return 0;
}

static int rdc200a_config_bitmap_osd_alpha_blending(UINT_T drawer_id,
												E_VC_OSD_ALPHA_T alpha)
{
	switch (drawer_id) {
	case 0:
		RDC_REG_MASK_SET(0x0324, 0x0F, (U8_T)alpha);
		break;
	case 1:
		RDC_REG_MASK_SET(0x0324, 0xF0, (U8_T)alpha<<4);
		break;
	case 2:
		RDC_REG_SET(0x0325, (U8_T)alpha);
		break;
	default:
		break;
	}

	return 0;
}

static int rdc200a_hide_bitmap_osd(VC_BITMAP_DRAWER_CB *cb)
{
	rdc200a_config_bitmap_osd_alpha_blending(cb->drawer_id, VC_OSD_ALPHA_0);

	return 0;
}

static int rdc200a_show_bitmap_osd(VC_BITMAP_DRAWER_CB *cb)
{
	rdc200a_config_bitmap_osd_alpha_blending(cb->drawer_id, cb->alpha);

	return 0;
}

static int rdc200a_change_property_bitmap_osd(VC_BITMAP_DRAWER_CB *cb,
												E_VC_OSD_BITMAP_MAG_T mag,
												E_VC_OSD_ALPHA_T alpha)
{
	U32_T output_h_size, output_v_size, reg_addr;

	if (!rdc200a_check_bitmap_position(&output_h_size, &output_v_size,
										cb->pos_x, cb->pos_x, mag, cb->d)) {
		EMSG("Invalid bitmap size to drawing\n");
		return -1;
	}

	reg_addr = RDC200A_BMP_MAG_REG_ADDR(cb->drawer_id);
	RDC_REG_SET(reg_addr, (mag << 2) | (mag << 6));

	rdc200a_config_bitmap_osd_alpha_blending(cb->drawer_id, alpha);

	return 0;
}

static int rdc200a_move_bitmap_osd(VC_BITMAP_DRAWER_CB *cb, U32_T mov_x, U32_T mov_y)
{
	U8_T bmpdr_en, bmpdr_en_3d, dimension_reg_val;
	U32_T bmp_pos_x, bmp_pos_y;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr_msb;
	U8_T drawer_id = cb->drawer_id;

	if (!rdc200a_check_bitmap_position(&output_h_size, &output_v_size,
										mov_x, mov_y, cb->mag, cb->d)) {
		EMSG("Invalid bitmap size to moving\n");
		return -1;
	}

	bmp_pos_x = mov_x; // 1 pixel unit
	bmp_pos_y = mov_y;

	bmpdr_en = 1 << drawer_id;
	bmpdr_en_3d = (1 << drawer_id) << 4;

	/* Hide bitmap. */
	rdc200a_config_bitmap_osd_alpha_blending(drawer_id, VC_OSD_ALPHA_0);

	/* Frist, move 2D bitmap. */
	reg_addr_msb = RDC200A_2D_BMP_POS_X_MSB_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_x >> 8));
	RDC_REG_SET(reg_addr_msb+1, bmp_pos_x & 0xFF);

	reg_addr_msb = RDC200A_2D_BMP_POS_Y_MSB_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_y >> 8));
	RDC_REG_SET(reg_addr_msb+1, bmp_pos_y & 0xFF);

	/* Move 3D bitmap if enabled. */
	if (cb->d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) {
			bmp_pos_x += output_h_size; // 1 pixel unit
		}
		else if (dimension_reg_val & TAB3D_EN) {
			bmp_pos_y += output_v_size; // 1 pixel unit
		}
		else {
		}

		reg_addr_msb = RDC200A_3D_BMP_POS_X_MSB_REG_ADDR(drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_x >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_x & 0xFF);

		reg_addr_msb = RDC200A_3D_BMP_POS_Y_MSB_REG_ADDR(drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_y >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_y & 0xFF);
	}

	/* Restore alpha. */
	rdc200a_config_bitmap_osd_alpha_blending(drawer_id, cb->alpha);

	return 0;
}

static int rdc200a_draw_bitmap_osd(VC_BITMAP_DRAWER_CB *cb, U32_T pos_x, U32_T pos_y,
									U32_T bmp_addr, E_VC_OSD_BITMAP_MAG_T mag,
									E_VC_OSD_ALPHA_T alpha,
									E_VC_OSD_DIMENSTION_T d)
{
	U8_T bmpdr_en, bmpdr_en_3d;
	U32_T bmp_pos_x, bmp_pos_y;
	U32_T output_h_size, output_v_size;
	U16_T reg_addr_msb;
	U8_T dimension_reg_val;
	U8_T drawer_id = cb->drawer_id;

	if (!rdc200a_check_bitmap_position(&output_h_size, &output_v_size,
										pos_x, pos_y, mag, d)) {
		EMSG("Invalid bitmap size to drawing\n");
		return -1;
	}

	bmpdr_en = 1 << drawer_id;
	bmpdr_en_3d = (1 << drawer_id) << 4;

	bmp_pos_x = pos_x; // 1 pixel unit
	bmp_pos_y = pos_y; // 1 pixel unit

	/* Disable 2D and 3D drawer. */
	RDC_REG_MASK_SET(0x346, bmpdr_en|bmpdr_en_3d, 0);

	rdc200a_update_lut_from_flash(bmp_addr, FLASH_ITEM_ACTUAL_SIZE_BMP,
								rdc200a_bmp_drawer_sram_id[drawer_id]);

	/* Frist, draw 2D bitmap. */
	reg_addr_msb = RDC200A_2D_BMP_POS_X_MSB_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_x >> 8));
	RDC_REG_SET(reg_addr_msb+1, bmp_pos_x & 0xFF);

	reg_addr_msb = RDC200A_2D_BMP_POS_Y_MSB_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_y >> 8));
	RDC_REG_SET(reg_addr_msb+1, bmp_pos_y & 0xFF);

	// mag ===> nearest
	RDC_REG_SET(RDC200A_BMP_MAG_REG_ADDR(drawer_id), (mag << 2) | (mag << 6));

#if 0

	mag_x = (U8_T)mag;
	mag_y = (U8_T)mag;

	if (cb->d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) // Side By Side
			mag_y = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of X
		else if (dimension_reg_val & TAB3D_EN)
			mag_x = (U8_T)(mag + (E_VC_OSD_TEXT_MAG_T)1); // Twice of Y
		else {
		}
	}

	reg_addr = RDC200A_FONT_ALPHA_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, ((U8_T)back_alpha<<4)|(U8_T)font_alpha);

	reg_addr = RDC200A_FONT_MAG_REG_ADDR(drawer_id);
	RDC_REG_SET(reg_addr, (mag_y<<4)|mag_x);
#endif

	// alpha
	rdc200a_config_bitmap_osd_alpha_blending(drawer_id, alpha);

	// Enable 2D
	RDC_REG_MASK_SET(0x346, bmpdr_en, bmpdr_en);

	/* Draw 3D bitmap if enabled. */
	if (d == VC_OSD_DIMENSTION_3D) {
		dimension_reg_val = RDC_REG_GET(0x0014);

		if (dimension_reg_val & (SBS3D_EN|SBS_LL_3D_EN)) {
			bmp_pos_x += output_h_size; // 1 pixel unit
		}
		else if (dimension_reg_val & TAB3D_EN) {
			bmp_pos_y += output_v_size; // 1 pixel unit
		}
		else {
		}

		reg_addr_msb = RDC200A_3D_BMP_POS_X_MSB_REG_ADDR(drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_x >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_x & 0xFF);

		reg_addr_msb = RDC200A_3D_BMP_POS_Y_MSB_REG_ADDR(drawer_id);
		RDC_REG_SET(reg_addr_msb, (U8_T)(bmp_pos_y >> 8));
		RDC_REG_SET(reg_addr_msb+1, bmp_pos_y & 0xFF);

		RDC_REG_MASK_SET(0x346, bmpdr_en_3d, bmpdr_en_3d);
	}
	else
		RDC_REG_MASK_SET(0x346, bmpdr_en_3d, 0);

	return 0;
}

static void rdc200a_disable_laser_sparkling(void)
{
	RDC_REG_SET(0x0572, 0x00);
	RDC_REG_SET(0x05A4, 0x10);
}

static int rdc200a_change_laser_sparkling_pwm(U16_T on_time_period, U16_T off_time_period)
{
	U8_T param[7];

	/* Start RDC address to burst write. (Addr: 0x0572) */
	param[0] = 0x05;
	param[1] = 0x72;
	param[2] = 0x01;
	param[3] = on_time_period >> 8;
	param[4] = on_time_period & 0xFF;
	param[5] = off_time_period >> 8;
	param[6] = off_time_period & 0xFF;
	RDC200A_REG_BURST_SET(param, 7);

	// set FW interrupt
	RDC_REG_SET(0x05A4, 0x10);

	return 0;
}

static int rdc200a_disable_video_sync_interrupt(void)
{
	// set FW interrupt
	RDC_REG_SET(0x05A4, 0x08); // INTR_SET<31:24> reg, [27]

	return 0;
}

int rdc200a_enable_video_sync_interrupt(void)
{
	// set FW interrupt
	RDC_REG_SET(0x05A4, 0x04); // INTR_SET<31:24> reg, [26]

	return 0;
}

static int rdc200a_request_atw_shift_pixel(
				E_VC_PANEL_CTRL_PORT_T ctrl_port,
				RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
	U8_T H_SHIFT_SIGN[2] = {0x0, 0x0}, V_SHIFT_SIGN[2] = {0x0, 0x0};
	U8_T shift_param[12];
	UINT_T i = 0, chk_port = ctrl_port;

	do {
		if (chk_port & 0x1) {
			if (info[i].shift_x >= 0)
				H_SHIFT_SIGN[i] = 0x80; // right direction from image
			else {
				H_SHIFT_SIGN[i] = 0x00; // left direction from image
				info[i].shift_x = -info[i].shift_x; /* Make a positive value */
			}

			if (info[i].shift_x > 0xFFF) {
				EMSG("P%u: Invalid horizontal shift value!\n", i);
				return -1;
			}

			if (info[i].shift_y >= 0)
				V_SHIFT_SIGN[i] = 0x00; // up direction from image
			else {
				V_SHIFT_SIGN[i] = 0x80; // down direction from image
				info[i].shift_y = -info[i].shift_y; /* Make a positive value */
			}

			if (info[i].shift_y > 0x1FFF) {
				EMSG("P%u: Invalid vertical shift value!\n", i);
				return -2;
			}
		}

		i++;
		chk_port >>= 1;
	} while (chk_port);

	if (ctrl_port == VC_PANEL_CTRL_PORT_0) { // Port 0 only attached
		/* Start RDC address to burst write. (Addr: 0x059A) */
		shift_param[0] = 0x05;
		shift_param[1] = 0x9A;

		shift_param[2] = (info[VC_PANEL_PORT_0].shift_x >> 8) | H_SHIFT_SIGN[0];
		shift_param[3] = info[VC_PANEL_PORT_0].shift_x & 0xFF;
		shift_param[4] = (info[VC_PANEL_PORT_0].shift_y >> 8) | V_SHIFT_SIGN[0];
		shift_param[5] = info[VC_PANEL_PORT_0].shift_y & 0xFF;
		shift_param[6] = 0x01; // TX0_ATW_SET
		RDC200A_REG_BURST_SET(shift_param, 7);
	}
	else if (ctrl_port == VC_PANEL_CTRL_PORT_1) { // Port 1 only attached
		/* Start RDC address to burst write. (Addr: 0x059F) */
		shift_param[0] = 0x05;
		shift_param[1] = 0x9F;

		shift_param[2] = (info[VC_PANEL_PORT_1].shift_x >> 8) | H_SHIFT_SIGN[1];
		shift_param[3] = info[VC_PANEL_PORT_1].shift_x & 0xFF;
		shift_param[4] = (info[VC_PANEL_PORT_1].shift_y >> 8) | V_SHIFT_SIGN[1];
		shift_param[5] = info[VC_PANEL_PORT_1].shift_y & 0xFF;
		shift_param[6] = 0x01; // TX1_ATW_SET
		RDC200A_REG_BURST_SET(shift_param, 7);
	}
	else { // Port 0 and Port 1
		/* Start RDC address to burst write. (Addr: 0x059A) */
		shift_param[0] = 0x05;
		shift_param[1] = 0x9A;

		shift_param[2] = (info[VC_PANEL_PORT_0].shift_x >> 8) | H_SHIFT_SIGN[0];
		shift_param[3] = info[VC_PANEL_PORT_0].shift_x & 0xFF;
		shift_param[4] = (info[VC_PANEL_PORT_0].shift_y >> 8) | V_SHIFT_SIGN[0];
		shift_param[5] = info[VC_PANEL_PORT_0].shift_y & 0xFF;
		shift_param[6] = 0x01; // TX0_ATW_SET

		shift_param[7] = (info[VC_PANEL_PORT_1].shift_x >> 8) | H_SHIFT_SIGN[1];
		shift_param[8] = info[VC_PANEL_PORT_1].shift_x & 0xFF;
		shift_param[9] = (info[VC_PANEL_PORT_1].shift_y >> 8) | V_SHIFT_SIGN[1];
		shift_param[10] = info[VC_PANEL_PORT_1].shift_y & 0xFF;
		shift_param[11] = 0x01; // TX1_ATW_SET

		RDC200A_REG_BURST_SET(shift_param, 12);
	}

	return 0;
}

static int rdc200a_shift_vertical_pixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
									E_VC_SHIFT_VERTICAL_T v_direction)
{
	S16_T shift_val;
	U8_T msb[2], lsb[2];
	/*
	 Select vertical shift direction [7]
	 1'b0 : up direction from image
	 1'b1 : down direction from image
	*/
	U8_T V_SHIFT_SIGN = 0x00;
	U16_T v_shift_msb_reg[2] = {0x832, 0x932};
	U16_T v_shift_lsb_reg[2] = {0x833, 0x933};
	UINT_T i = 0, chk_port = (UINT_T)ctrl_port;

	do {
		if (chk_port & 0x1) {
			msb[i] = RDC_REG_GET(v_shift_msb_reg[i]);
			lsb[i] = RDC_REG_GET(v_shift_lsb_reg[i]);

			V_SHIFT_SIGN = msb[i] & 0x80; /* Get the shift sign bit. */
			msb[i] &= 0x1F;

			shift_val = (S16_T)(((U16_T)msb[i] << 8) | lsb[i]);
			//DMSG("P%u: V_shift_val(%d)\n", i, shift_val);

			if (V_SHIFT_SIGN & 0x80)
				shift_val = -shift_val; /* Make a negative value */

			if (v_direction == VC_SHIFT_VERTICAL_DOWN) {
				shift_val--;
				if (shift_val < 0) {
					V_SHIFT_SIGN = 0x80; // down direction from image
					shift_val = ABS(shift_val);
					if (shift_val > 0x1FFF) {
						EMSG("P%u: Bottom! Can't move to the down side anymore\n", i);
						return -1;
					}
				}
				else {
					V_SHIFT_SIGN = 0x00; // up direction from image
				}
			}
			else {
				shift_val++;
				if (shift_val < 0) {
					V_SHIFT_SIGN = 0x80; // down direction from image
					shift_val = ABS(shift_val);
				}
				else {
					V_SHIFT_SIGN = 0x00; // up direction from image

					if (shift_val > 0x1FFF) {
						EMSG("P%u: Top! Can't move to the up side anymore\n", i);
						return -2;
					}
				}
			}

			msb[i] = (shift_val >> 8) | V_SHIFT_SIGN;
			lsb[i] = shift_val & 0xFF;
		}

		i++;
		chk_port >>= 1;
	} while (chk_port);

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		RDC_REG_SET(v_shift_msb_reg[0], msb[0]);
		RDC_REG_SET(v_shift_lsb_reg[0], lsb[0]);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		RDC_REG_SET(v_shift_msb_reg[1], msb[1]);
		RDC_REG_SET(v_shift_lsb_reg[1], lsb[1]);
	}

	return 0;
}

static int rdc200a_shift_horizontal_pixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
									E_VC_SHIFT_HORIZONTAL_T h_direction)
{
	S16_T shift_val;
	U8_T msb[2], lsb[2];
	/*
	 Select horizontal shift direction [7]
		1'b0 : left direction from image
		1'b1 : right direction from image
	*/
	U8_T H_SHIFT_SIGN = 0x00;
	U16_T h_shift_msb_reg[2] = {0x830, 0x930};
	U16_T h_shift_lsb_reg[2] = {0x831, 0x931};
	UINT_T i = 0, chk_port = (UINT_T)ctrl_port;

	do {
		if (chk_port & 0x1) {
			msb[i] = RDC_REG_GET(h_shift_msb_reg[i]);
			lsb[i] = RDC_REG_GET(h_shift_lsb_reg[i]);

			H_SHIFT_SIGN = msb[i] & 0x80; /* Get the shift sign bit. */
			msb[i] &= 0x0F;

			shift_val = (S16_T)(((U16_T)msb[i] << 8) | lsb[i]);
			//DMSG("P%u: H_shift_val(%d)\n", i, shift_val);

			if (H_SHIFT_SIGN == 0x00)
				shift_val = -shift_val; /* Make a negative value */

			if (h_direction == VC_SHIFT_HORIZONTAL_LEFT) {
				shift_val--;
				if (shift_val < 0) {
					H_SHIFT_SIGN = 0x00; // left direction from image

					shift_val = ABS(shift_val);
					if (shift_val > 0xFFF) {
						EMSG("P%u: Leftmost! Can't move to the left side anymore\n", i);
						return -1;
					}
				}
				else {
					H_SHIFT_SIGN = 0x80; // right direction from image
				}
			}
			else {
				shift_val++;

				if (shift_val < 0) {
					H_SHIFT_SIGN = 0x00; // left direction from image
					shift_val = ABS(shift_val);
				}
				else {
					H_SHIFT_SIGN = 0x80; // right direction from image

					if (shift_val > 0xFFF) {
						EMSG("P%u: Rightmost! Can't move to the right side anymore\n", i);
						return -2;
					}
				}
			}

			msb[i] = (shift_val >> 8) | H_SHIFT_SIGN;
			lsb[i] = shift_val & 0xFF;
		}

		i++;
		chk_port >>= 1;
	} while (chk_port);

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		RDC_REG_SET(h_shift_msb_reg[0], msb[0]);
		RDC_REG_SET(h_shift_lsb_reg[0], lsb[0]);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		RDC_REG_SET(h_shift_msb_reg[1], msb[1]);
		RDC_REG_SET(h_shift_lsb_reg[1], lsb[1]);
	}

	return 0;
}

static int rdc200a_enable_test_pattern(BOOL_T en)
{
	if (en == TRUE) {
		RDC_REG_MASK_SET(0x0200, 0x07, 0x04);
		RDC_REG_SET(0x0280, 0x00);
	}
	else {
		RDC_REG_SET(0x0200, rdc200a_mrx_config);
		RDC_REG_SET(0x0280, rdc200a_pgen_config);
	}

	return 0;
}

static int rdc200a_generate_test_pattern(U8_T r_val, U8_T g_val, U8_T b_val)
{
	RDC_REG_SET(0x0281, r_val);
	RDC_REG_SET(0x0282, g_val);
	RDC_REG_SET(0x0283, b_val);

	return 0;
}

#if 1 // betta added
S32_T rdc200a_change_input_resolutionx(
	U32_T h_input_size,
	U32_T v_input_size,
	U32_T h_blank,
	U32_T v_blank,
	U32_T frame_rate)
{
	int ret = 0;
	U32_T pclk_hz;
	U8_T dif_path; /* Display Interface path */
	U8_T input_res_idx = 0x3f;
	U16_T DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL;

	dif_path = RDC_REG_GET(0x0200) & 0x3;
	if ((dif_path != 0x02/* MIPI */) && (dif_path != 0x00))
	{
		EMSG("Unsupported DIF to change input resolution!\n");
		return -2;
	}
	pclk_hz = (h_input_size + h_blank) * (v_input_size + v_blank) * frame_rate;

	if (dif_path == 0x2) /* MIPI */
		ret = rdc200a_config_mipi_rx_clock(pclk_hz);
	else /*0x0: LVDS */
		ret = rdc200a_config_lvds_rx_clock(pclk_hz);

	if (ret)
		return ret;

	/* INPUT_SIZE_IDX */
	RDC_REG_SET(0x0201, (U8_T)((U16_T)input_res_idx << 2));

	RDC_REG_SET(0x0202, (U8_T)(h_input_size/4 >> 8));
	RDC_REG_SET(0x0203, h_input_size/4 & 0xFF);
	RDC_REG_SET(0x0204, (U8_T)(v_input_size/4 >> 8));
	RDC_REG_SET(0x0205, v_input_size/4 & 0xFF);

	/* Change FIFO level. */
	DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL = (h_input_size >> 2) * 3; // h_input_size * 3/4;
	RDC_REG_SET(0x0269, (U8_T)(DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL >> 8));
	RDC_REG_SET(0x026A, DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL & 0xFF);

	return 0;
}
void set_rdc200a_output_60Hz(void) // in: 60Hz 1920x1080, out: 60Hz, 1080P
{
	RDC_REG_SET(0x49, 0x37);
	RDC_REG_SET(0x82b, 1);
	RDC_REG_SET(0x92b, 1);
	RDC_REG_SET(0x14, 9); // 3D mode                                                                                                                                    , 0x9);

	RDC_REG_SET(0x82E, 2);
	RDC_REG_SET(0x92E, 2);
}

void set_rdc200a_output_90Hz(void) // in: 90Hz 1920x1080, out: 90Hz, 1080P
{
	rdc200a_change_input_resolutionx(1920, 1080, 280, 45, 90);
	RDC_REG_SET(0x49, 0x52);
	RDC_REG_SET(0x82b, 1);
	RDC_REG_SET(0x92b, 1);
	RDC_REG_SET(0x14, 9); // 3D mode                                                                                                                                    , 0x9);

	RDC_REG_SET(0x82E, 2);
	RDC_REG_SET(0x92E, 2);
}
#if 0 // set_rdc200a_output_120Hz doesn't work
//static int rdc200a_change_input_resolution2(VC_INPUT_RES_T res, BOOL_T enable_3D, U32_T pclk_hz);
void set_rdc200a_output_120Hz(void) // in: 120Hz 2560x720, out: 120Hz
{
	rdc200a_change_input_resolutionx(2560, 720, 740, 30, 120);
	//rdc200a_change_input_resolution2(VC_INPUT_RES_DUALHD, true, 297000000);

	RDC_REG_SET(0x82b, 1);
	RDC_REG_SET(0x92b, 1);

	RDC_REG_SET(0x82E, 2);
	RDC_REG_SET(0x92E, 2);
// input resaolution
	/*
	RDC_REG_SET(0x202, 2);
	RDC_REG_SET(0x203, 0x80);
	RDC_REG_SET(0x204, 0);
	RDC_REG_SET(0x205, 0xb4);
	*/
// Upscale
	RDC_REG_SET(0x834, 0x0a);
	RDC_REG_SET(0x934, 0x0a);
// 3D mode
	RDC_REG_SET(0x14, 0x11);
}
#endif

#endif // betta added

static int rdc200a_set_image_display_format(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
										BOOL_T swap_image_3d_mode)
{
	U8_T dimension_reg_val[MAX_NUM_VC_DISPLAY_FORMAT] = {
		0/*2D*/, SBS3D_EN, TAB3D_EN,
		SBS_LL_3D_EN, FBF_3D_EN, FBF_EX_SIG
	};

	if((fmt != VC_IMAGE_DISPLAY_FORMAT_2D) && swap_image_3d_mode)
		dimension_reg_val[fmt] |= PANEL_SWAP;

	/* Save mrx_cfg and pgen_cfg used by enable/disable Test Pattern */
		rdc200a_mrx_config = RDC_REG_GET(0x0200);
		rdc200a_pgen_config = RDC_REG_GET(0x0280);

    // panel off
    rdc200a_drive_panel(FALSE);
    // reset on/off
    RDC_REG_MASK_SET(0x0013, 0x04, 0x00);
    RDC_REG_MASK_SET(0x0013, 0x04, 0x04);

    // 3d func
    RDC_REG_MASK_SET(0x0014, 0xFA, dimension_reg_val[fmt]);
    // pgen on
    RDC_REG_MASK_SET(0x0200, 0x07, 0x04);
    RDC_REG_SET(0x0280, 0x00);
    // wait more than rx 3 frame
    VC_U_DELAY(50000);
    // pgen  off
    RDC_REG_SET(0x0200, rdc200a_mrx_config);
    RDC_REG_SET(0x0280, rdc200a_pgen_config);
    // panel on
    rdc200a_drive_panel(TRUE);

	return 0;
}



///
#define TX1_PANEL_ACCESS_EN		0x08
#define TX0_PANEL_ACCESS_EN		0x04
#define TX1_SLV_I2C_BYPASS		0x02
#define TX0_SLV_I2C_BYPASS		0x01

static void init_panel_backdoor_path(void)
{
	RDC_REG_SET(0x0004, 0x76);
	RDC_REG_SET(0x0005, _CFG_RDP_SLAVE_ADDR);
	RDC_REG_SET(0x0006, 0x00); /* Disable all panel backdoor path */
}

static S32_T rdc200a_get_frame_rate(struct _VC_RDC_DEV_INFO_T *dev)
{
	return dev->frame_rate;
}

#define VC_LED_DUTY_TO_DOUBLE(duty)\
	((double)(duty) / 100./ (double)VC_LED_DUTY_DIVIDER)


static int rdc200a_set_led_duty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							E_VC_LED_CTRL_COLOR_T color,
							VC_LED_DUTY_INFO_T new_duty[MAX_NUM_VC_PANEL_PORT])
{
	U32_T output_h_size, output_v_size;
	S32_T R_P0_tuning[2] = {0, 0}, G_P0_tuning[2] = {0, 0}, B_P0_tuning[2] = {0, 0};
	S32_T R_P1_tuning[2] = {0, 0}, G_P1_tuning[2] = {0, 0}, B_P1_tuning[2] = {0, 0};
	U16_T holding_time, led_off_time;
	double percentage;
	UINT_T chk_port = (UINT_T)ctrl_port;
	UINT_T i = 0;

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	if (vc_panel_attached_port_mask == VC_PANEL_CTRL_PORT_1)
		ctrl_port = VC_PANEL_CTRL_PORT_ALL;

	/*
	 * First: Calculate the value of register to write at the same time.
	 *        And calculate the returning value.
	 */
	do {
		if (chk_port & 0x1) {
			if (color & VC_LED_CTRL_COLOR_R) {
				percentage = VC_LED_DUTY_TO_DOUBLE(new_duty[i].R);
				if (percentage <= 0.0) {
					EMSG("[P%u] R: Can't step to the down side anymore\n", i);
					return -1;
				}

				if (percentage > 1.0) {
					EMSG("[P%u] R: Can't step to the up side anymore\n", i);
					return -2;
				}

				holding_time = (U16_T)RDC_REG_GET(DISPLAY_HOLD_R(i)) << 3; /* 8 line unit */
				led_off_time = (U16_T)(RDC_REG_GET(LED_P0_OFF_R_msb(i)) & 0x1F) << 8;
				led_off_time |= (U16_T)RDC_REG_GET(LED_P0_OFF_R_lsb(i));

				R_P0_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
				if (R_P0_tuning[i] < 0) {
					EMSG("[P%u] R: MIN register value is out of range\n", i);
					return -3;
				}

				if (R_P0_tuning[i] > 0x1FFF) {
					EMSG("[P%u] R: MAX register value is out of range\n", i);
					return -4;
				}

				new_duty[i].R = (S32_T)((double)(led_off_time - R_P0_tuning[i])
							/ (double)(output_v_size + holding_time)
							* (100*VC_LED_DUTY_DIVIDER));

                led_off_time = (U16_T)(RDC_REG_GET(LED_P1_OFF_R_msb(i)) & 0x1F) << 8;
                led_off_time |= (U16_T)RDC_REG_GET(LED_P1_OFF_R_lsb(i));

                R_P1_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
                if (R_P1_tuning[i] < 0) {
                    EMSG("[P%u] R: MIN register value is out of range\n", i);
                    return -3;
                }

                if (R_P1_tuning[i] > 0x1FFF) {
                    EMSG("[P%u] R: MAX register value is out of range\n", i);
                    return -4;
                }
			}

			if (color & VC_LED_CTRL_COLOR_G) {
				percentage = VC_LED_DUTY_TO_DOUBLE(new_duty[i].G);
				if (percentage <= 0.0) {
					EMSG("[P%u] G: Can't step to the down side anymore\n", i);
					return -5;
				}

				if (percentage > 1.0) {
					EMSG("[P%u] G: Can't step to the up side anymore\n", i);
					return -6;
				}

				holding_time = (U16_T)RDC_REG_GET(DISPLAY_HOLD_G(i)) << 3; /* 8 line unit */
				led_off_time = (U16_T)(RDC_REG_GET(LED_P0_OFF_G_msb(i)) & 0x1F) << 8;
				led_off_time |= (U16_T)RDC_REG_GET(LED_P0_OFF_G_lsb(i));

				G_P0_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
				if (G_P0_tuning[i] < 0) {
					EMSG("[P%u] G: MIN register value is out of range\n", i);
					return -7;
				}

				if (G_P0_tuning[i] > 0x1FFF) {
					EMSG("[P%u] G: MAX register value is out of range\n", i);
					return -8;
				}

				new_duty[i].G = (S32_T)((double)(led_off_time - G_P0_tuning[i])
							/ (double)(output_v_size + holding_time)
							* (100*VC_LED_DUTY_DIVIDER));

                led_off_time = (U16_T)(RDC_REG_GET(LED_P1_OFF_G_msb(i)) & 0x1F) << 8;
                led_off_time |= (U16_T)RDC_REG_GET(LED_P1_OFF_G_lsb(i));

                G_P1_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
                if (G_P1_tuning[i] < 0) {
                    EMSG("[P%u] G: MIN register value is out of range\n", i);
                    return -7;
                }

                if (G_P1_tuning[i] > 0x1FFF) {
                    EMSG("[P%u] G: MAX register value is out of range\n", i);
                    return -8;
                }
			}

			if (color & VC_LED_CTRL_COLOR_B) {
				percentage = VC_LED_DUTY_TO_DOUBLE(new_duty[i].B);
				if (percentage <= 0.0) {
					EMSG("[P%u] G: Can't step to the down side anymore\n", i);
					return -9;
				}

				if (percentage > 1.0) {
					EMSG("[P%u] G: Can't step to the up side anymore\n", i);
					return -10;
				}

				holding_time = (U16_T)RDC_REG_GET(DISPLAY_HOLD_B(i)) << 3; /* 8 line unit */
				led_off_time = (U16_T)(RDC_REG_GET(LED_P0_OFF_B_msb(i)) & 0x1F) << 8;
				led_off_time |= (U16_T)RDC_REG_GET(LED_P0_OFF_B_lsb(i));

				B_P0_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
				if (B_P0_tuning[i] < 0) {
					EMSG("[P%u] B: MIN register value is out of range\n", i);
					return -11;
				}

				if (B_P0_tuning[i] > 0x1FFF) {
					EMSG("[P%u] B: MAX register value is out of range\n", i);
					return -12;
				}

				new_duty[i].B = (S32_T)((double)(led_off_time - B_P0_tuning[i])
							/ (double)(output_v_size + holding_time)
							* (100*VC_LED_DUTY_DIVIDER));

                led_off_time = (U16_T)(RDC_REG_GET(LED_P1_OFF_B_msb(i)) & 0x1F) << 8;
                led_off_time |= (U16_T)RDC_REG_GET(LED_P1_OFF_B_lsb(i));

                B_P1_tuning[i] = (S32_T)((double)(led_off_time - (output_v_size + holding_time) * percentage) + 0.5);
                if (B_P1_tuning[i] < 0) {
                    EMSG("[P%u] B: MIN register value is out of range\n", i);
                    return -11;
                }

                if (B_P1_tuning[i] > 0x1FFF) {
                    EMSG("[P%u] B: MAX register value is out of range\n", i);
                    return -12;
                }
			}
		}

		i++;
		chk_port >>= 1;
	} while (chk_port);

	/*
	 * Second: Write the calculated value into register.
	 */
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		if (color & VC_LED_CTRL_COLOR_R) {
			RDC_REG_SET(LED_P0_ON_R_msb(0), R_P0_tuning[0] >> 8); /* led_p0_on_r */
			RDC_REG_SET(LED_P0_ON_R_lsb(0), R_P0_tuning[0] & 0xFF);
			RDC_REG_SET(LED_P1_ON_R_msb(0), R_P1_tuning[0] >> 8); /* led_r1_on_r */
			RDC_REG_SET(LED_P1_ON_R_lsb(0), R_P1_tuning[0] & 0xFF);
		}

		if (color & VC_LED_CTRL_COLOR_G) {
			RDC_REG_SET(LED_P0_ON_G_msb(0), G_P0_tuning[0] >> 8); /* led_p0_on_g */
			RDC_REG_SET(LED_P0_ON_G_lsb(0), G_P0_tuning[0] & 0xFF);
			RDC_REG_SET(LED_P1_ON_G_msb(0), G_P1_tuning[0] >> 8); /* led_p1_on_g */
			RDC_REG_SET(LED_P1_ON_G_lsb(0), G_P1_tuning[0] & 0xFF);
		}

		if (color & VC_LED_CTRL_COLOR_B) {
			RDC_REG_SET(LED_P0_ON_B_msb(0), B_P0_tuning[0] >> 8); /* led_p0_on_r */
			RDC_REG_SET(LED_P0_ON_B_lsb(0), B_P0_tuning[0] & 0xFF);
			RDC_REG_SET(LED_P1_ON_B_msb(0), B_P1_tuning[0] >> 8); /* led_p1_on_r */
			RDC_REG_SET(LED_P1_ON_B_lsb(0), B_P1_tuning[0] & 0xFF);
		}
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		if (color & VC_LED_CTRL_COLOR_R) {
			RDC_REG_SET(LED_P0_ON_R_msb(1), R_P0_tuning[1] >> 8); /* led_p0_on_r */
			RDC_REG_SET(LED_P0_ON_R_lsb(1), R_P0_tuning[1] & 0xFF);
			RDC_REG_SET(LED_P1_ON_R_msb(1), R_P1_tuning[1] >> 8); /* led_p1_on_r */
			RDC_REG_SET(LED_P1_ON_R_lsb(1), R_P1_tuning[1] & 0xFF);
		}

		if (color & VC_LED_CTRL_COLOR_G) {
			RDC_REG_SET(LED_P0_ON_G_msb(1), G_P0_tuning[1] >> 8); /* led_p0_on_g */
			RDC_REG_SET(LED_P0_ON_G_lsb(1), G_P0_tuning[1] & 0xFF);
			RDC_REG_SET(LED_P1_ON_G_msb(1), G_P1_tuning[1] >> 8); /* led_p1_on_g */
			RDC_REG_SET(LED_P1_ON_G_lsb(1), G_P1_tuning[1] & 0xFF);
		}

		if (color & VC_LED_CTRL_COLOR_B) {
			RDC_REG_SET(LED_P0_ON_B_msb(1), B_P0_tuning[1] >> 8); /* led_p0_on_r */
			RDC_REG_SET(LED_P0_ON_B_lsb(1), B_P0_tuning[1] & 0xFF);
			RDC_REG_SET(LED_P1_ON_B_msb(1), B_P1_tuning[1] >> 8); /* led_p1_on_r */
			RDC_REG_SET(LED_P1_ON_B_lsb(1), B_P1_tuning[1] & 0xFF);
		}
	}

	return 0;
}

static void rdc200a_get_led_duty_from_reg(UINT_T port, E_VC_LED_CTRL_COLOR_T color,
										VC_LED_DUTY_INFO_T *info)
{
	U32_T output_h_size, output_v_size;
	S32_T delta;
	U16_T r_holditme, g_holditme, b_holditme;
	U16_T led_on_r, led_off_r;
	U16_T led_on_g, led_off_g;
	U16_T led_on_b, led_off_b;
	double duty_percentage;

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	if (color & VC_LED_CTRL_COLOR_R) {
		led_on_r = (U16_T)(RDC_REG_GET(LED_P0_ON_R_msb(port)) & 0x1F) << 8; // <12:8>
		led_on_r |= (U16_T)RDC_REG_GET(LED_P0_ON_R_lsb(port));

		led_off_r = (U16_T)(RDC_REG_GET(LED_P0_OFF_R_msb(port)) & 0x1F) << 8;
		led_off_r |= (U16_T)RDC_REG_GET(LED_P0_OFF_R_lsb(port));

		r_holditme = (U16_T)RDC_REG_GET(DISPLAY_HOLD_R(port)) << 3; /* 8 line unit */

		delta = led_off_r - led_on_r; /* 1 line unit */

		duty_percentage = (delta/(double)(output_v_size + r_holditme)) * 100.;
		info->R = (S32_T)(duty_percentage * VC_LED_DUTY_DIVIDER + 0.5);
	}

	if (color & VC_LED_CTRL_COLOR_G) {
		led_on_g = (U16_T)(RDC_REG_GET(LED_P0_ON_G_msb(port)) & 0x1F) << 8; // <12:8>
		led_on_g |= (U16_T)RDC_REG_GET(LED_P0_ON_G_lsb(port));

		led_off_g = (U16_T)(RDC_REG_GET(LED_P0_OFF_G_msb(port)) & 0x1F) << 8;
		led_off_g |= (U16_T)RDC_REG_GET(LED_P0_OFF_G_lsb(port));

		g_holditme = (U16_T)RDC_REG_GET(DISPLAY_HOLD_G(port)) << 3; /* 8 line unit */

		delta = led_off_g - led_on_g; /* 1 line unit */

		duty_percentage = (delta/(double)(output_v_size + g_holditme)) * 100.;
		info->G = (S32_T)(duty_percentage * VC_LED_DUTY_DIVIDER + 0.5);
	}

	if (color & VC_LED_CTRL_COLOR_B) {
		led_on_b = (U16_T)(RDC_REG_GET(LED_P0_ON_B_msb(port)) & 0x1F) << 8; // <12:8>
		led_on_b |= (U16_T)RDC_REG_GET(LED_P0_ON_B_lsb(port));

		led_off_b = (U16_T)(RDC_REG_GET(LED_P0_OFF_B_msb(port)) & 0x1F) << 8;
		led_off_b |= (U16_T)RDC_REG_GET(LED_P0_OFF_B_lsb(port));

		b_holditme = (U16_T)RDC_REG_GET(DISPLAY_HOLD_B(port)) << 3; /* 8 line unit */

		delta = led_off_b - led_on_b; /* 1 line unit */

		duty_percentage = (delta/(double)(output_v_size + b_holditme)) * 100.;
		info->B = (S32_T)(duty_percentage * VC_LED_DUTY_DIVIDER + 0.5);
	}
}

static S32_T rdc200a_get_led_duty(struct _VC_RDC_DEV_INFO_T *dev,
								E_VC_PANEL_CTRL_PORT_T ctrl_port,
								VC_LED_DUTY_INFO_T info[MAX_NUM_VC_PANEL_PORT])
{
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		if (dev->led_duty_info[0].R == -1)
			rdc200a_get_led_duty_from_reg(0, VC_LED_CTRL_COLOR_ALL, &dev->led_duty_info[0]);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1)
		if (dev->led_duty_info[1].R == -1)
			rdc200a_get_led_duty_from_reg(1, VC_LED_CTRL_COLOR_ALL, &dev->led_duty_info[1]);

	info[0] = dev->led_duty_info[0];
	info[1] = dev->led_duty_info[1];

	return 0;
}

#if 1
static void rdc200a_fill_step_value(VC_LED_DUTY_INFO_T *new_duty,
									E_VC_LED_CTRL_COLOR_T color, BOOL_T up,
									VC_LED_DUTY_INFO_T *dev_duty)
{
	if (color & VC_LED_CTRL_COLOR_R) {
		if (up == TRUE)
			new_duty->R = (S32_T)RDC200A_STEP_UP_LED_DUTY_VALUE(dev_duty->R);
		else
			new_duty->R = (S32_T)RDC200A_STEP_DOWN_LED_DUTY_VALUE(dev_duty->R);
	}

	if (color & VC_LED_CTRL_COLOR_G) {
		if (up == TRUE)
			new_duty->G = (S32_T)RDC200A_STEP_UP_LED_DUTY_VALUE(dev_duty->G);
		else
			new_duty->G = (S32_T)RDC200A_STEP_DOWN_LED_DUTY_VALUE(dev_duty->G);
	}

	if (color & VC_LED_CTRL_COLOR_B) {
		if (up == TRUE)
			new_duty->B = (S32_T)RDC200A_STEP_UP_LED_DUTY_VALUE(dev_duty->B);
		else
			new_duty->B = (S32_T)RDC200A_STEP_DOWN_LED_DUTY_VALUE(dev_duty->B);
	}
}

static S32_T rdc200a_set_frame_rate(VC_RDC_DEV_INFO_T *dev, S32_T new_rate)
{
	U16_T pll2_refdiv;
	U32_T output_h_size, output_v_size;
	U32_T r_holding_time, g_holding_time, b_holding_time;
	double frame_rate_hz, new_frame_rate_hz;
	int x_clock;
	int lvds_line;
	U32_T bB4_bA;
	S32_T ret;
	U32_T vertical_line_pixel, scan_time, scans_holds;
	U32_T CONFIG_PANEL_DRV, num_tx_channel_idx;
	U8_T num_tx_ch_shift[4] = {2/*4*/, 3/*8*/, 0, 1/*2*/};

	if (new_rate <= 0) {
		EMSG("Can't step to the down side anymore\n");
		return -1;
	}
	new_frame_rate_hz = (double)new_rate / (double)VC_FRAME_RATE_DIVIDER;

	pll2_refdiv = RDC_REG_GET16(0x0047) & 0x03;
	x_clock = (RDC200A_XTAL_FREQ_MHz / (pll2_refdiv + 1));

	if (vc_panel_attached_port_mask & VC_PANEL_CTRL_PORT_0) {
		r_holding_time = (U16_T)RDC_REG_GET(0x0823) << 3; /* 8 pixel unit */
		g_holding_time = (U16_T)RDC_REG_GET(0x0824) << 3;
		b_holding_time = (U16_T)RDC_REG_GET(0x0825) << 3;
		CONFIG_PANEL_DRV = RDC_REG_GET32(0x0821);
	}
	else {
		r_holding_time = (U16_T)RDC_REG_GET(0x0923) << 3; /* 8 pixel unit */
		g_holding_time = (U16_T)RDC_REG_GET(0x0924) << 3;
		b_holding_time = (U16_T)RDC_REG_GET(0x0925) << 3;
		CONFIG_PANEL_DRV = RDC_REG_GET32(0x0921);
	}

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	vertical_line_pixel = output_h_size;
	scan_time = output_v_size;

	num_tx_channel_idx = CONFIG_PANEL_DRV >> 6;
	lvds_line = 1 << num_tx_ch_shift[num_tx_channel_idx];

	scans_holds = scan_time*3 + r_holding_time+g_holding_time+b_holding_time;

	bB4_bA = (U32_T)(4*(2 + (double)vertical_line_pixel/(double)lvds_line)*new_frame_rate_hz * scans_holds * (4. /(double)(1000*1000*x_clock))+0.5);
	if (bB4_bA > 0x3FF) { /* pll2_plln<9:0> */
		EMSG("Can't step to the up side anymore\n");
		return -2;
	}

	RDC_REG_SET(0x0048, bB4_bA>>8);
	RDC_REG_SET(0x0049, bB4_bA & 0xFF);

	frame_rate_hz = 1.0/(double)(scans_holds*(4 * (2 + vertical_line_pixel/(double)lvds_line)* 4 /(double)(bB4_bA*x_clock)/1000.0/1000.0));

	ret = (S32_T)(frame_rate_hz * VC_FRAME_RATE_DIVIDER);

	return ret;
}

static S32_T rdc200a_step_frame_rate(struct _VC_RDC_DEV_INFO_T *dev, BOOL_T up)
{
	S32_T applied_frame_rate;
	S32_T new_frame_rate;

	if (up == TRUE)
		new_frame_rate = RDC200A_STEP_UP_FRAME_RATE_VALUE(dev->frame_rate);
	else
		new_frame_rate = RDC200A_STEP_DOWN_FRAME_RATE_VALUE(dev->frame_rate);

	applied_frame_rate = rdc200a_set_frame_rate(dev, new_frame_rate);
	if (applied_frame_rate > 0)
		dev->frame_rate = applied_frame_rate; /* Update new rate */

	return applied_frame_rate;
}

#if 1 // betta changed
 S32_T rdc200a_get_frame_rate_from_reg(void)
#else
	static S32_T rdc200a_get_frame_rate_from_reg(void)
#endif
{
	U16_T pll2plln;
	U16_T pll2_refdiv;
	U8_T reg_0x47, reg_0x48, reg_0x49;
	U32_T output_h_size, output_v_size, feedback_divider_ratio;
	double one_line_time;
	U16_T r_holditme, g_holditme, b_holditme;
	int vco_osc_freq, lvds_line;
	double pll_out_freq;
	double lvds_tx_clock;
	double frame_rate_hz;
	int x_clock;
	U32_T vertical_line_pixel, scan_time;
	U32_T CONFIG_PANEL_DRV, num_tx_channel_idx;
	U8_T num_tx_ch_shift[4] = {2/*4*/, 3/*8*/, 0, 1/*2*/};

	reg_0x47 = RDC_REG_GET(0x0047);
	reg_0x48 = RDC_REG_GET(0x0048);
	reg_0x49 = RDC_REG_GET(0x0049);
	pll2plln = ((U16_T)reg_0x48 << 8) | reg_0x49;
	feedback_divider_ratio = (pll2plln >> 2)*4 + (pll2plln & 0x3);

	pll2_refdiv = reg_0x47 & 0x03;
	x_clock = (RDC200A_XTAL_FREQ_MHz / (pll2_refdiv + 1));

	vco_osc_freq = feedback_divider_ratio * x_clock;
	pll_out_freq = vco_osc_freq / 2.0f;
	lvds_tx_clock = pll_out_freq / 2.0f;

	if (vc_panel_attached_port_mask & VC_PANEL_CTRL_PORT_0) {
		r_holditme = (U16_T)RDC_REG_GET(0x0823) << 3; /* 8 pixel unit */
		g_holditme = (U16_T)RDC_REG_GET(0x0824) << 3;
		b_holditme = (U16_T)RDC_REG_GET(0x0825) << 3;
		CONFIG_PANEL_DRV = RDC_REG_GET32(0x0821);
	}
	else {
		r_holditme = (U16_T)RDC_REG_GET(0x0923) << 3; /* 8 pixel unit */
		g_holditme = (U16_T)RDC_REG_GET(0x0924) << 3;
		b_holditme = (U16_T)RDC_REG_GET(0x0925) << 3;
		CONFIG_PANEL_DRV = RDC_REG_GET32(0x0921);
	}

	rdc200a_get_output_resolution(&output_h_size, &output_v_size);

	vertical_line_pixel = output_h_size;
	scan_time = output_v_size;

	num_tx_channel_idx = CONFIG_PANEL_DRV >> 6;
	lvds_line = 1 << num_tx_ch_shift[num_tx_channel_idx];

	one_line_time = 4. * (2. + (double)vertical_line_pixel/(double)lvds_line) / (double)lvds_tx_clock / 1000.0/1000.0;

	frame_rate_hz = 1.0 / ((double)(scan_time*3 + (r_holditme+g_holditme+b_holditme)) * one_line_time);

	return (S32_T)(frame_rate_hz * VC_FRAME_RATE_DIVIDER);
}

static int rdc200a_prepare_controller(VC_RDC_DEV_INFO_T *dev)
{
#if 0
	// Enable button interrupt to make sw backdoor.
	//RDC_REG_SET(0x05A8, 0x01); // INTR_CLEAR<31:24>, [24]: button 0
	//RDC_REG_MASK_SET(0x054C, 0x01, 0x01); // INTR_EN<31:24>, [24]: button 0
#else
	init_panel_backdoor_path();
#endif

	dev->frame_rate = rdc200a_get_frame_rate_from_reg();

#if 0
	U32_T lvds_rate_bps;
	U8_T reg_0x27, reg_0x28;
	U32_T pll2_plln, pll2_refdiv;

	reg_0x27 = RDC_REG_GET(0x27); // pll2_plln<9:2>
	reg_0x28 = RDC_REG_GET(0x28);

	pll2_plln = ((U16_T)reg_0x27<<2) | (reg_0x28>>6);
	pll2_refdiv = reg_0x28 & 0x3;

	lvds_rate_bps
		= (U32_T)((float)(RDC200_XTAL_FREQ_Hz * pll2_plln) / (float)(pll2_refdiv + 1));

	dev->lvds_rate_bps = lvds_rate_bps >>= 1; /* Divied by 2 */

	dev->min_frame_rate = (U8_T)((50*(lvds_rate_bps/1000000)) / 800);
	dev->max_frame_rate = (U8_T)((95*(lvds_rate_bps/1000000)) / 800);

	dev->frame_rate_hz = dev->max_frame_rate; /* Default */
#endif

	/* Save mrx_cfg and pgen_cfg used by enable/disable Test Pattern */
	rdc200a_mrx_config = RDC_REG_GET(0x0200);
	rdc200a_pgen_config = RDC_REG_GET(0x0280);

	return 0;
}

static int rdc200a_change_input_display_interface(E_VC_INPUT_DISPLAY_IF_T dif,
												VC_INPUT_DISPLAY_IF_CFG *cfg)
{
	U8_T reg_0x10 = 0x0, reg_0x11 = 0x0;
	U8_T lvds_rev_bus = 0; // Don't reverse both
	U8_T lvds_pclk = 0;
	U8_T lvds_port = 0;
	U8_T mrx = rdc200a_mrx_config;

	reg_0x10 = RDC_REG_GET(0x10) & 0x01;
	reg_0x11 = RDC_REG_GET(0x11);

	switch (dif) {
	case VC_INPUT_DISPLAY_IF_LVDS_0:
	case VC_INPUT_DISPLAY_IF_LVDS_1:
	case VC_INPUT_DISPLAY_IF_LVDS_BOTH:
		mrx = 0;

		if (dif == VC_INPUT_DISPLAY_IF_LVDS_0) {
			reg_0x11 |= 0x80; // power down LVDS port 1
			lvds_port = 0;
		}
		else if (dif == VC_INPUT_DISPLAY_IF_LVDS_1) {
			reg_0x11 |= 0x40; // power down LVDS port 0
			lvds_pclk = 1; // Use port 1 clock
			lvds_port = 1;
		}
		else {
			reg_0x11 |= 0x00; // LVDS port 0/1 power on
			if (cfg->lvds.dual_mode_type == VC_LVDS_DUAL_MODE_PORT0_EVEN)
				lvds_port = 2;
			else
				lvds_port = 3;
		}

        RDC_REG_MASK_SET(0x0290, 0x07, 0); // RX_MON_DETECT_SEL<2:0>
		// config
		RDC_REG_SET(0x0210, ((U8_T)((U16_T)(lvds_pclk&0x01) <<7))
					|((U8_T)(lvds_port&0x03) << 5)
					|((U8_T)(lvds_rev_bus&0x03) << 3)
					|(U8_T)(cfg->lvds.color_depth&0x01));

            //pll lock check
        rdc200a_check_pll_lock(0);

        // mrx
        RDC_REG_MASK_SET(0x0200, 0x7, mrx);

        // power down
        RDC_REG_MASK_SET(0x0010, 0x01, reg_0x10);
        RDC_REG_SET(0x0011, reg_0x11);
		break;

	case VC_INPUT_DISPLAY_IF_MIPI:

        //STEP 1. Disable Host MIPI out.

        //STEP 2. RDC200A MIPI config
		reg_0x11 = 0xC0; // power down LVDS port 0 and 1.
		mrx = 2;

        RDC_REG_SET(0x0210, 0x00); //Set input config to default

        RDC_REG_MASK_SET(0x0290, 0x07, 2); // RX_MON_DETECT_SEL<2:0>
		// config
		// {mipi_num_lane, color_coding}
        RDC_REG_SET(0x0241,    0x00);
        RDC_REG_SET(0x0010    ,0x00);
        RDC_REG_SET(0x0012    ,0x10);
        RDC_REG_SET(0x0013    ,0x7F);

        //pll lock check
        rdc200a_check_pll_lock(0);

        RDC_REG_SET(0x0241,    0x10);
        VC_M_DELAY(1);
        RDC_REG_SET(0x0012,    0x11);

        RDC_REG_MASK_SET(0x0240, 0x03, cfg->mipi.num_lane);
        RDC_REG_MASK_SET(0x0253, 0x03, cfg->mipi.num_lane);

		// mipi_auto_pd, mipi_cont_clk
		RDC_REG_MASK_SET(0x0244, 0xC,
						((cfg->mipi.auto_pd << 3)
						| (cfg->mipi.cont_clk << 2)));

        RDC_REG_SET(0x0012, 0x1F);
        RDC_REG_SET(0x0013, 0xFF);

        // mrx
        RDC_REG_MASK_SET(0x0200, 0x7, mrx);

        // power down
        RDC_REG_MASK_SET(0x0010, 0x01, reg_0x10);
        RDC_REG_SET(0x0011, reg_0x11);

        //STEP 3. Enable Host MIPI out.

		break;

	default:
		break;
	}

	rdc200a_mrx_config = mrx;

	return 0;
}

static void rdc200a_set_default(VC_RDC_DEV_INFO_T *dev)
{
	dev->led_duty = 15; /* Default. HMD system. Beam project: 95 */

	dev->min_led_duty = 5;
	dev->max_led_duty = 95;
}
#endif

static int rdc200a_get_fw_version(U8_T *major_ver, U8_T *minor_ver, U8_T *rel_ver)
{
	U8_T reg_0x0002;

	reg_0x0002 = RDC_REG_GET(0x0002);
	*major_ver = reg_0x0002 >> 4;
	*rel_ver = reg_0x0002 & 0x0F;

	*minor_ver = RDC_REG_GET(0x0003);

	return 0;
}

static int rdc200a_get_device_id(U16_T *chip_id, U16_T *rev_num)
{
	U8_T chip_id_msb, chip_id_lsb;

	chip_id_msb = RDC_REG_GET(0x0000);
	chip_id_lsb = RDC_REG_GET(0x0001);

	*chip_id = ((U16_T)chip_id_msb << 8) | chip_id_lsb;
	*rev_num = 0;

	return 0;
}

/******************************************************************************
 * START.
 * 1. Reference codes for RDC200A power On/Off/Suspend/Resume.
 * 2. Reference codes for RDC200A MIPI configuration and monitoring.
 *****************************************************************************/

/* NOTE: The gpio numbers are pseudo number for use in the examples. */
#define GPIO_RDC200A_LDO_1_2V    14
#define GPIO_RDC200A_LDO_1_8V    18
#define GPIO_RDC200A_LDO_3_3V    19
#define GPIO_RDC200A_RESETB      20
#define GPIO_BOOT_DONE_FROM_RDC200A_GPIOB3      21

/* NOTE: Mapping System output GPIO for Power On/OFF and HW Reset control */
void gpio_set_value(unsigned int gpio, int value)
{
}

/*
 RDC200A INPUT VCD Monitoring
 */
typedef struct {
    unsigned short hsw;
    unsigned short hbp;
    unsigned short hfp;
    unsigned short vsw;
    unsigned short vbp;
    unsigned short vfp;
    unsigned short ha;
    unsigned short va;
} RDC_VIDEO_COORD_INFO_T;

/* Get the informations of video coordination. */
void rdc200a_get_video_coord_info(RDC_VIDEO_COORD_INFO_T *vcd)
{

    vcd->hsw = (unsigned short)RDC_REG_GET(0x0293);
    vcd->hbp = (unsigned short)RDC_REG_GET(0x0294);
    vcd->hfp = (unsigned short)RDC_REG_GET(0x0292);
    vcd->ha  = (((unsigned short)RDC_REG_GET(0x0295) << 8) | (unsigned short)RDC_REG_GET(0x0296));

    vcd->vsw = (unsigned short)RDC_REG_GET(0x0299);
    vcd->vbp = (unsigned short)RDC_REG_GET(0x029A);
    vcd->vfp = (((unsigned short)RDC_REG_GET(0x0297) << 8) | (unsigned short)RDC_REG_GET(0x0298));
    vcd->va =  (((unsigned short)RDC_REG_GET(0x029B) << 8) | (unsigned short)RDC_REG_GET(0x029C));

    printf("HSW = %d HBP = %d HFP = %d HACTIVE = %d \n", vcd->hsw, vcd->hbp,  vcd->hfp, vcd->ha);
    printf("VSW = %d VBP = %d VFP = %d VACTIVE = %d \n", vcd->vsw, vcd->vbp,  vcd->vfp, vcd->va);

}

static void rdc200a_mipi_inf_reset()
{
    RDC_REG_SET(0x0010, 0x01);
    RDC_REG_SET(0x0241, 0x00);
    RDC_REG_SET(0x0011, 0xC0);
    RDC_REG_SET(0x0010, 0x00);
    RDC_REG_SET(0x0012, 0x10);
    RDC_REG_SET(0x0013, 0x7F);
    RDC_REG_SET(0x0241, 0x10);
    VC_M_DELAY(1);
    RDC_REG_SET(0x0012, 0x11);

    RDC_REG_SET(0x0012, 0x1F);
    RDC_REG_SET(0x0013, 0xFF);

}

void rdc200a_sbs_on()
{
	rdc200a_set_image_display_format(VC_IMAGE_DISPLAY_FORMAT_3D_SBS, FALSE);
}

void rdc200a_sbs_off()
{
	rdc200a_set_image_display_format(VC_IMAGE_DISPLAY_FORMAT_2D, FALSE);
}


// CH: Switch to 2D based on RDC200A EVB excel sheet
static void rdc200a_set_3d_display()
{
    RDC_REG_SET(0x0015, 0x33);
    RDC_REG_SET(0x0013, 0xFB);
    RDC_REG_SET(0x0013, 0xFF);
    RDC_REG_SET(0x0014, 0x09);
    RDC_REG_SET(0x0200, 0x04);
    RDC_REG_SET(0x0280, 0x00);
    RDC_REG_SET(0x0200, 0x02);
    RDC_REG_SET(0x0280, 0x00);
    RDC_REG_SET(0x0015, 0x3F);
}

// CH: Switch to 2D based on RDC200A EVB excel sheet
static void rdc200a_set_2d_display()
{
    RDC_REG_SET(0x0015, 0x33);
    RDC_REG_SET(0x0013, 0xFB);
    RDC_REG_SET(0x0013, 0xFF);
    RDC_REG_SET(0x0014, 0x01);
    RDC_REG_SET(0x0200, 0x04);
    RDC_REG_SET(0x0280, 0x00);
    RDC_REG_SET(0x0200, 0x02);
    RDC_REG_SET(0x0280, 0x00);
    RDC_REG_SET(0x0015, 0x3F);
}

// CH: Switch RDC200A from input to output and low to high to activate full process
bool rdc200a_setupOutputpin()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GpioRDC200A.mPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(GpioRDC200A.mPort, &GPIO_InitStruct);

	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GpioRDC200A.mPort, GpioRDC200A.mPin, GPIO_PIN_RESET); // PC13
	HAL_Delay(5);

	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GpioRDC200A.mPort, GpioRDC200A.mPin, GPIO_PIN_SET); // PC13
	HAL_Delay(5);
    return true;
}

int rdc200a_check_boot_done()
{
    int i, ret;
    //unsigned char flag_boot_done;

    VC_M_DELAY(100);  //100ms delay Must need.

    for(i = 0; i <= 100; i++) {
        //flag_boot_done = gpio_get_value(GPIO_BOOT_DONE_FROM_RDC200A_GPIOB3);

        //if(flag_boot_done == 0) break;
    	if(HAL_GPIO_ReadPin(RDC200A_BOOTING_INDICATION_GPIO_Port, RDC200A_BOOTING_INDICATION_Pin) == GPIO_PIN_RESET) break;

        VC_M_DELAY(10);

        printf("Boot Wait Count = %d  Time  = %d \n", i, i * 10);
    }

    if(i >= 100)
        ret =  -1; //Power On fail
    else ret =  0; //Power On success

    return ret;
}

int rdc200a_power_on(void)
{
    //gpio_set_value(GPIO_RDC200A_LDO_1_2V, 1);
    //gpio_set_value(GPIO_RDC200A_LDO_1_8V, 1);
    //gpio_set_value(GPIO_RDC200A_LDO_3_3V, 1);
    //VC_M_DELAY(1);

    //gpio_set_value(GPIO_RDC200A_RESETB, 1);
	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(RDC200A_RSTB_H_GPIO_Port, RDC200A_RSTB_H_Pin, GPIO_PIN_SET); // PC13

    return rdc200a_check_boot_done();
}

int rdc200a_power_off(void)
{
    rdc200a_poweron_panel(FALSE);
    VC_M_DELAY(10);

    //gpio_set_value(GPIO_RDC200A_RESETB, 0);
	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(RDC200A_RSTB_H_GPIO_Port, RDC200A_RSTB_H_Pin, GPIO_PIN_RESET); // PC13
    VC_M_DELAY(10);

    //gpio_set_value(GPIO_RDC200A_LDO_3_3V, 0);
    //gpio_set_value(GPIO_RDC200A_LDO_1_8V, 0);
    //gpio_set_value(GPIO_RDC200A_LDO_1_2V, 0);

    return 0;
}

int display_suspend(unsigned char poweroff)
{
    rdc200a_poweron_panel(FALSE);
    VC_M_DELAY(10);

    if(poweroff) //Case for RDC200A and Panel Power Down
    //    gpio_set_value(GPIO_RDC200A_RESETB, 0);
	/* Configure GPIO pin Output Level */
    	HAL_GPIO_WritePin(RDC200A_RSTB_H_GPIO_Port, RDC200A_RSTB_H_Pin, GPIO_PIN_RESET); // PC13

    // Disable Host LVDS or MIPI out.

    return 0;
}

#define TARGET_H_RESOLUTION 1920
#define TARGET_V_RESOLUTION 1080
int display_resume(unsigned char poweroff, E_RDC_VIDEO_COORD_INPUT_T iface)
{
    int i, ret, recv_confirm = 0;
    RDC_VIDEO_COORD_INFO_T vcd;

    //STEP 1. Host MIPI or LVDS output must disable.

    //STEP 2. RDC200A Reset
    if(poweroff) {//Case for RDC200A and Panel Power Up
    //    gpio_set_value(GPIO_RDC200A_RESETB, 1);
    	/* Configure GPIO pin Output Level */
    	HAL_GPIO_WritePin(RDC200A_RSTB_H_GPIO_Port, RDC200A_RSTB_H_Pin, GPIO_PIN_SET); // PC13
        rdc200a_check_boot_done();
    }

    if(iface == RDC_VIDEO_COORD_INPUT_LVDS) {
        //STEP 3. Enable Host MIPI out.
    }
    else {//Case for using MIPI interface
        rdc200a_mipi_inf_reset();
        //STEP 3. Enable Host MIPI out.
    }

    //STEP 4. Check Display Input status.
    for(i = 0; i < 50; i++) {
        rdc200a_get_video_coord_info(&vcd);

        if(vcd.ha == TARGET_H_RESOLUTION && vcd.va == TARGET_V_RESOLUTION) {
            recv_confirm ++;
            if(recv_confirm == 2) break; // criteria value 2 is changable after platform test.
        }

        VC_M_DELAY(20);
        printf("COUNT = %d recv_confirm = %d RECV_H = %d  RECV_V  = %d \n", i, recv_confirm, vcd.ha, vcd.va);
    }

    if(recv_confirm == 2) {
        rdc200a_poweron_panel(TRUE);
        ret = 0; //Power On success
        //STEP 5. If use External LED/Laser Driver, Turn  it on after rdc200a panel power on.
    }
    else ret = -1;//Power On fail, check the Host MIPI or LVDS output quality.

    return ret;
}

/******************************************************************************
 * END.
 * 1. Reference codes for RDC200A power On/Off/Suspend/Resume.
 * 2. Reference codes for RDC200A MIPI configuration and monitoring.
 *****************************************************************************/


#define NUM_RDC200A_MIPI_PCLK_BND	7 // PCLK boundary
#define NUM_RDC200A_LVDS_PCLK_BND	11 // PCLK boundary

/* MIPI Interface clock table. */
typedef struct {
	U32_T boundary_pclk_hz; /* PCLK + PCLK/16 */

	U8_T pll0_plln;
	U8_T pll0_refdiv;
	U8_T pll0_postd;

	U8_T mipi_clk_periph_sel;
	U8_T mipi_rxclkinesc_sel;
	U8_T mipi_txclkinesc_sel;
	U8_T mipi_dpi_pclk_sel;
	U8_T mipi_clk_rx_sel;
	U8_T mipi_dphy_prg_rxhs_settle;
} RDC200A_MIPI_CLK_TABLE_T;

/* LVDS Interface clock table. */
typedef struct {
	U32_T boundary_pclk_hz; /* PCLK + PCLK/16 */

	U8_T pll0_plln;
	U8_T pll0_refdiv;
	U8_T pll0_postd;
} RDC200A_LVDS_CLK_TABLE_T;

static const RDC200A_MIPI_CLK_TABLE_T
	rdc200a_mipi_clk_table[MAX_NUM_VC_MIPI_LANE][NUM_RDC200A_MIPI_PCLK_BND] = {
	{	/* Num of lane = 1 */
		{ 18000000, 0x27, 0, 1, 0, 0, 0, 2, 2, 0xC}, /* 18 MHz */
		{ 34000000, 0x36, 0, 1, 0, 0, 0, 1, 1, 0xB}, /* 34 MHz */
		{ 45000000, 0x48, 0, 1, 0, 1, 1, 1, 1, 0x8}, /* 45 MHz */
		{ 62500000, 0x2F, 0, 2, 0, 1, 1, 1, 1, 0x8}, /* 62.5 MHz */
		{ 62500000, 0x2F, 0, 2, 0, 1, 1, 1, 1, 0x8}, /* 62.5 MHz */
		{ 62500000, 0x2F, 0, 2, 0, 1, 1, 1, 1, 0x8}, /* 62.5 MHz */
		{ 62500000, 0x2F, 0, 2, 0, 1, 1, 1, 1, 0x8}, /* 62.5 MHz */
	},
	{	/* Num of lane = 2 */
		{ 18000000, 0x27, 0, 1, 1, 0, 0, 2, 2, 0xC}, /* 18 MHz */
		{ 35000000, 0x26, 0, 1, 1, 0, 0, 1, 1, 0xB}, /* 35 MHz */
		{ 45000000, 0x30, 0, 1, 1, 0, 0, 1, 1, 0xB}, /* 45 MHz */
		{ 65000000, 0x48, 0, 1, 1, 1, 1, 1, 1, 0x9}, /* 65 MHz */
		{ 90000000, 0x30, 0, 1, 0, 0, 0, 0, 0, 0x9}, /* 90 MHz */
		{110000000, 0x3C, 0, 1, 0, 1, 0, 0, 0, 0x7}, /* 110 MHz */
		{125000000, 0x2F, 0, 2, 0, 1, 1, 0, 0, 0x8}  /* 125 MHz */
	},
	{	/* Num of lane = 3 */
		{ 18000000, 0x26, 0, 1, 1, 0, 0, 2, 2, 0xC}, /* 18 MHz */
		{ 35000000, 0x26, 0, 1, 1, 0, 0, 1, 1, 0xB}, /* 35 MHz */
		{ 75000000, 0x28, 0, 1, 0, 0, 0, 0, 0, 0xB}, /* 75 MHz */
		{100000000, 0x36, 0, 1, 0, 0, 0, 0, 0, 0xC}, /* 100 MHz */
		{140000000, 0x26, 0, 2, 0, 1, 1, 0, 0, 0x7}, /* 140 MHz */
		{160000000, 0x26, 0, 2, 0, 1, 1, 0, 0, 0x9}, /* 160 MHz */
		{187500000, 0x30, 0, 2, 0, 1, 2, 0, 0, 0x9}  /* 187.5 MHz */

	},
	{	/* Num of lane = 4                         boundary pclk : i_rx_clk */
		{ 75000000, 0x28, 0, 1, 0, 0, 0, 0, 0, 0xA}, /* 75 : 80 MHz */
		{100000000, 0x36, 0, 1, 0, 0, 0, 0, 0, 0xC}, /* 100: 108 MHz */
		{134000000, 0x48, 0, 1, 0, 1, 1, 0, 0, 0x7}, /* 134: 144 MHz */
		{160000000, 0x2B, 0, 2, 0, 1, 1, 0, 0, 0x9}, /* 160: 172 MHz */
		{190000000, 0x33, 0, 2, 0, 1, 2, 0, 0, 0x9}, /* 190: 204 MHz */
		{220000000, 0x38, 0, 2, 0, 1, 2, 0, 0, 0xA}, /* 220: 236 MHz */
		{250000000, 0x3F, 0, 2, 0, 2, 2, 0, 0, 0x8}  /* 250: 252 MHz */
	}
};

static const RDC200A_LVDS_CLK_TABLE_T
	rdc200a_lvds_clk_table[NUM_RDC200A_LVDS_PCLK_BND] = {
	{ 18000000, 0x28, 1, 0}, /* < 18 MHz */
	{ 36000000, 0x26, 0, 0}, /* < 36 MHz */
	{ 45000000, 0x30, 0, 0}, /* < 45 MHz */
	{ 60000000, 0x40, 0, 0}, /* < 60 MHz */
	{ 75000000, 0x27, 0, 1}, /* < 75 MHz */
	{100000000, 0x35, 0, 1}, /* < 100 MHz */
	{120000000, 0x3F, 0, 1}, /* < 120 MHz */
	{150000000, 0x26, 0, 2}, /* < 150 MHz */
	{186000000, 0x2F, 0, 2}, /* < 186 MHz */
	{225000000, 0x39, 0, 2}, /* < 225 MHz */
	{225000000, 0x3F, 0, 2}  /* >= 225 MHz */

};

int rdc200a_config_lvds_rx_clock(U32_T pclk_hz)
{
	const RDC200A_LVDS_CLK_TABLE_T *tbl;
	UINT_T i;
	U32_T check_pclk_hz = pclk_hz;

	tbl = (const RDC200A_LVDS_CLK_TABLE_T *)rdc200a_lvds_clk_table;

	/* If it is not found in the 10 boundaries, the last setting is selected. */
	for (i = 0; i < (NUM_RDC200A_LVDS_PCLK_BND - 1); i++) {
		if (pclk_hz < tbl->boundary_pclk_hz)
			break;

		tbl++;
	}

	RDC_REG_SET(0x0027, 0x20 | (tbl->pll0_refdiv) | (tbl->pll0_postd << 2));
	RDC_REG_SET(0x0028, (U8_T)((tbl->pll0_plln & 0x300) >> 8));
	RDC_REG_SET(0x0029, (tbl->pll0_plln & 0xFF));

	if (!rdc200a_check_pll_lock(0))
		return -1;

	return 0;
}

int rdc200a_config_mipi_rx_clock(U32_T pclk_hz)
{
	UINT_T num_lane;
	const RDC200A_MIPI_CLK_TABLE_T *tbl;
	UINT_T i;
	U32_T check_pclk_hz = pclk_hz + (pclk_hz >> 4);

	num_lane = RDC_REG_GET(0x0253) & 0x3;
	tbl = (const RDC200A_MIPI_CLK_TABLE_T *)&rdc200a_mipi_clk_table[num_lane];

	/* If it is not found in the 6 boundaries, the last setting is selected. */
	for (i = 0; i < (NUM_RDC200A_MIPI_PCLK_BND - 1); i++) {
		if (check_pclk_hz < tbl->boundary_pclk_hz)
			break;

		tbl++;
	}

	RDC_REG_SET(0x0027, 0x20 | (tbl->pll0_refdiv) | (tbl->pll0_postd << 2));
	RDC_REG_SET(0x0028, (U8_T)((tbl->pll0_plln & 0x300) >> 8));
	RDC_REG_SET(0x0029, (tbl->pll0_plln & 0xFF));

	RDC_REG_MASK_SET(0x0241, 0x01, tbl->mipi_clk_periph_sel);
	RDC_REG_SET(0x0242, tbl->mipi_txclkinesc_sel|(tbl->mipi_rxclkinesc_sel<<2)|
						(tbl->mipi_dpi_pclk_sel<<4)|(tbl->mipi_clk_rx_sel<<6));
	RDC_REG_SET(0x0245, tbl->mipi_dphy_prg_rxhs_settle);

	if (!rdc200a_check_pll_lock(0))
		return -1;

	return 0;
}


static BOOL_T rdc200a_is_available_input_resolution(VC_INPUT_RES_T res)
{
	switch (res) {
	case VC_INPUT_RES_RESERVED0:
	case VC_INPUT_RES_RESERVED2:
	case VC_INPUT_RES_RESERVED3:
	case VC_INPUT_RES_RESERVED4:
	case VC_INPUT_RES_RESERVED5:
	case VC_INPUT_RES_RESERVED6:
	case VC_INPUT_RES_RESERVED7:
	case VC_INPUT_RES_RESERVEDA:
	case VC_INPUT_RES_RESERVEDB:
	case VC_INPUT_RES_RESERVEDC:
	case VC_INPUT_RES_RESERVEDD:
	case VC_INPUT_RES_RESERVEDE:
	case VC_INPUT_RES_ETC:
		return FALSE;

	default:
		break;
	}

	return TRUE;
}

static int rdc200a_set_scaleup(U8_T h_scale_ration, U8_T v_scale_ration)
{
	U8_T scale_reg_val = h_scale_ration | (v_scale_ration << 2);

	RDC_REG_SET(0x0834, scale_reg_val);
	RDC_REG_SET(0x0934, scale_reg_val);

	return 0;
}

static int rdc200a_get_scale_ration(U8_T *h_scale_ration, U8_T *v_scale_ration,
							U32_T h_input_size, U32_T v_input_size)
{
	/* Even if an error occurs, the default ration is x1 */
	U8_T h_ration = 0, v_ration = 0; /* init to zero to make default ration */
	U32_T h_scale, v_scale;
	U32_T h_output_size, v_output_size;
	UINT_T err_mask = 0x0; /* 0x1: H scale up, 0x2: V scale up */

	rdc200a_get_output_resolution(&h_output_size, &v_output_size);

	h_scale = (h_output_size * 100) / h_input_size;
	switch (h_scale) {
	case 100:
		h_ration = 0; /* x1 */
		break;
	case 133:
		h_ration = 1; /* x1.33 */
		break;
	case 150:
		h_ration = 2; /* x1.5 */
		break;
	case 200:
		h_ration = 3; /* x2 */
		break;
	default:
		err_mask = 0x1;
		break;
	}

	v_scale = (v_output_size * 100) / v_input_size;
	switch (v_scale) {
	case 100:
		v_ration = 0; /* x1 */
		break;
	case 133:
		v_ration = 1; /* x1.33 */
		break;
	case 150:
		v_ration = 2; /* x1.5 */
		break;
	case 200:
		v_ration = 3; /* x2 */
		break;
	default:
		err_mask |= 0x2;
		break;
	}

	*h_scale_ration = h_ration;
	*v_scale_ration = v_ration;

	switch (err_mask) {
	case 0x0:
		return 0;
	case 0x1:
		EMSG("Cannot be scale up horizontally!\n");
		return -1;
	case 0x2:
		EMSG("Cannot be scale up vertically!\n");
		return -2;
	case 0x3:
		EMSG("Cannot be scale up horizontally and vertically!\n");
		return -3;
	default:
		return 0;
	}
}

static int rdc200a_config_input_resolution(U8_T dif_path, VC_INPUT_RES_T res,
										U32_T pclk_hz, BOOL_T enable_3D)
{
	int ret = 0;
	U32_T h_input_size = rdc200a_input_res[res].h_size;
	U32_T v_input_size = rdc200a_input_res[res].v_size;
	U32_T target_h_input_size, target_v_input_size;
	E_VC_IMAGE_DISPLAY_FORMAT_T fmt;
	U8_T input_res_idx = (U8_T)res;
	U16_T DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL;
	U8_T h_scale_ration, v_scale_ration;

	DMSG("PCLK(%u) for the new input resolution\n", pclk_hz);

	if (enable_3D == TRUE) {
		fmt = VC_IMAGE_DISPLAY_FORMAT_3D_SBS_LL;
		target_h_input_size = h_input_size >> 1; /* 1/2 */
	}
	else {
		fmt = VC_IMAGE_DISPLAY_FORMAT_2D;
		target_h_input_size = h_input_size;
	}

	target_v_input_size = v_input_size;

	rdc200a_get_scale_ration(&h_scale_ration, &v_scale_ration,
							target_h_input_size, target_v_input_size);

	if (dif_path == 0x2) /* MIPI */
		ret = rdc200a_config_mipi_rx_clock(pclk_hz);
	else /*0x0: LVDS */
		ret = rdc200a_config_lvds_rx_clock(pclk_hz);

	if (ret)
		return ret;

	/* INPUT_SIZE_IDX */
	RDC_REG_SET(0x0201, (U8_T)((U16_T)input_res_idx << 2));

	/* Change FIFO level. */
	DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL = (h_input_size >> 2) * 3; // h_input_size * 3/4;
	RDC_REG_SET(0x0269, (U8_T)(DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL >> 8));
	RDC_REG_SET(0x026A, DSI_DPI_PAYLOAD_FIFO_SEND_LEVEL & 0xFF);

	rdc200a_set_scaleup(h_scale_ration, v_scale_ration);

	rdc200a_set_image_display_format(fmt, FALSE);

	return ret;
}

/**
 @ pclk_hz[in] Pixel clock in Hz.
 @NOTE Only support to Side By Side 3D.
*/
static int rdc200a_change_input_resolution2(VC_INPUT_RES_T res, BOOL_T enable_3D,
										U32_T pclk_hz)
{
	int ret = 0;
	U8_T prev_input_res_idx, input_res_idx = (U8_T)res;
	U8_T dif_path; /* Display Interface path */

	prev_input_res_idx = RDC_REG_GET(0x0201) >> 2;
	if (prev_input_res_idx == input_res_idx) {
		IMSG("Same resolution as previous input resolution.\n");
		return 0;
	}

	if (!rdc200a_is_available_input_resolution(res)) {
		EMSG("Unsupported input resolution!\n");
		return -1;
	}

	dif_path = RDC_REG_GET(0x0200) & 0x3;
	if ((dif_path != 0x02/* MIPI */) && (dif_path != 0x00))
	{
		EMSG("Unsupported DIF to change input resolution!\n");
		return -2;
	}

	ret = rdc200a_config_input_resolution(dif_path, res, pclk_hz, enable_3D);

	return ret;
}

/**
 @NOTE Only support to Side By Side 3D.
*/
static int rdc200a_change_input_resolution(VC_INPUT_RES_T res, BOOL_T enable_3D,
										U32_T h_blank, U32_T v_blank,
										U32_T frame_rate)
{
	int ret = 0;
	U32_T h_input_size = rdc200a_input_res[res].h_size;
	U32_T v_input_size = rdc200a_input_res[res].v_size;
	U8_T prev_input_res_idx, input_res_idx = (U8_T)res;
	U8_T dif_path; /* Display Interface path */
	U32_T pclk_hz;

	prev_input_res_idx = RDC_REG_GET(0x0201) >> 2;
	if (prev_input_res_idx == input_res_idx) {
		IMSG("Same resolution as previous input resolution.\n");
		//return 0;
	}

	if (!rdc200a_is_available_input_resolution(res)) {
		EMSG("Unsupported input resolution!\n");
		return -1;
	}

	dif_path = RDC_REG_GET(0x0200) & 0x3;
	if ((dif_path != 0x02/* MIPI */) && (dif_path != 0x00))
	{
		EMSG("Unsupported DIF to change input resolution!\n");
		return -2;
	}

	pclk_hz = (h_input_size + h_blank) * (v_input_size + v_blank) * frame_rate;

	ret = rdc200a_config_input_resolution(dif_path, res, pclk_hz, enable_3D);

	return ret;
}

int RDC200A_Probe(VC_RDC_DEV_INFO_T *dev)
{
	int ret;

	rdc200a_set_default(dev);

	ret = RDC200A_ProbeFLASH(dev);

	dev->get_fw_version = rdc200a_get_fw_version;
	dev->prepare_controller = rdc200a_prepare_controller;

#if 0
	dev->power_on = rdc200_power_on;
	dev->power_off = rdc200_power_off;
	dev->suspend = rdc200_suspend;
	dev->resume = rdc200_resume;


#endif

	dev->get_device_id = rdc200a_get_device_id;

	dev->enable_test_pattern = rdc200a_enable_test_pattern;
	dev->generate_test_pattern = rdc200a_generate_test_pattern;


	dev->update_lut = rdc200a_update_lut_from_flash;
	dev->update_gamma_lut = rdc200a_update_gamma_lut;

	dev->enable_distortion_correction = rdc200a_enable_cokoa;

	dev->get_output_resolution = rdc200a_get_output_resolution;

	dev->poweron_panel = rdc200a_poweron_panel;

 	dev->enable_gamma_correction = rdc200a_enable_gamma_correction;

	dev->set_image_display_format = rdc200a_set_image_display_format;

	dev->shift_horizontal_pixel = rdc200a_shift_horizontal_pixel;
	dev->shift_vertical_pixel = rdc200a_shift_vertical_pixel;

	dev->get_frame_rate = rdc200a_get_frame_rate;
	dev->step_frame_rate = rdc200a_step_frame_rate;

	dev->get_led_duty = rdc200a_get_led_duty;
	dev->set_led_duty = rdc200a_set_led_duty;
	dev->fill_step_value = rdc200a_fill_step_value;

	dev->enable_video_sync_interrupt = rdc200a_enable_video_sync_interrupt;
	dev->disable_video_sync_interrupt = rdc200a_disable_video_sync_interrupt;

	dev->request_atw_shift_pixel = rdc200a_request_atw_shift_pixel;

	dev->change_input_display_interface = rdc200a_change_input_display_interface;

	dev->change_input_resolution = rdc200a_change_input_resolution;
	dev->change_input_resolution2 = rdc200a_change_input_resolution2;

	dev->draw_bitmap_osd = rdc200a_draw_bitmap_osd;
	dev->move_bitmap_osd = rdc200a_move_bitmap_osd;
	dev->show_bitmap_osd = rdc200a_show_bitmap_osd;
	dev->hide_bitmap_osd = rdc200a_hide_bitmap_osd;
	dev->change_property_bitmap_osd = rdc200a_change_property_bitmap_osd;
	dev->change_dimension_bitmap_osd = rdc200a_change_dimension_bitmap_osd;
	dev->disable_bitmap_osd = rdc200a_disable_bitmap_osd;

	dev->draw_text_osd = rdc200a_draw_text_osd;
	dev->move_text_osd = rdc200a_move_text_osd;
	dev->show_text_osd = rdc200a_show_text_osd;
	dev->hide_text_osd = rdc200a_hide_text_osd;
	dev->change_property_text_osd = rdc200a_change_property_text_osd;
	dev->change_dimension_text_osd = rdc200a_change_dimension_text_osd;
	dev->disable_text_osd = rdc200a_disable_text_osd;

	dev->change_laser_sparkling_pwm = rdc200a_change_laser_sparkling_pwm;
	dev->disable_laser_sparkling = rdc200a_disable_laser_sparkling;

#if 0
	dev->request_cokoa_generation = rdc200_request_cokoa_generation;

	dev->request_led_brightness_mode = rdc200_request_led_brightness_mode;
	dev->request_led_color_mode = rdc200_request_led_color_mode;
	dev->confirm_led_control = rdc200_confirm_led_control;
#endif

	return ret;
}

#endif // #if defined(CFG_MDC_RDC200A)



////2D->3D
//#if 0
//static int rdc200a_set_image_display_format(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
//		BOOL_T swap_image_3d_mode)
//#else
//int rdc200a_set_image_display_format2(void)
//#endif
//{
////	UINT_T panel_offon_mask = 0x0;
////	U8_T dimension_reg_val[MAX_NUM_VC_DISPLAY_FORMAT] = {
////		0/*2D*/, SBS3D_EN, TAB3D_EN,
////		SBS_LL_3D_EN, FBF_3D_EN, FBF_EX_SIG
////	};
//
////	if ((fmt != VC_IMAGE_DISPLAY_FORMAT_2D) && swap_image_3d_mode)
////		dimension_reg_val[fmt] |= PANEL_SWAP;
//
//    // panel off
//    RDC_REG_SET(0x0015,	0x33);	//rdc200a_drive_panel(FALSE);
//    // reset on/off
//    RDC_REG_SET(0x0013,	0xFB);	//RDC_REG_MASK_SET(0x0013, 0x04, 0x00);
//    RDC_REG_SET(0x0013,	0xFF);	//RDC_REG_MASK_SET(0x0013, 0x04, 0x04);
//    // 3d func
//    RDC_REG_SET(0x0014,	0x09);	//RDC_REG_MASK_SET(0x0014, 0xFA, dimension_reg_val[fmt]);//0x09 is for SBS 3D; if needing to left/right image swap,0x14reg need to be set 0x0B.
//    // pgen on
//    RDC_REG_SET(0x0200,	0x04);	//RDC_REG_MASK_SET(0x0200, 0x07, 0x04);
//    RDC_REG_SET(0x0280, 0x00);
//    // wait more than rx 3 frame
//    VC_U_DELAY(50000);//delay 50ms
//    // pgen  off
//    RDC_REG_SET(0x0200,	0x02);	//RDC_REG_SET(0x0200, rdc200a_mrx_config[vc_sel_rdc_idx]); //0x02 is for MIPI port; 0x00 is for LVDS port;
//    RDC_REG_SET(0x0280, 0x00);	//RDC_REG_SET(0x0280, rdc200a_pgen_config[vc_sel_rdc_idx]);
//    // panel on
//    RDC_REG_SET(0x0015,	0x3F);	//rdc200a_drive_panel(TRUE);
//
//	return 0;
//}
//
////3D->2D
//
//
//#if 0
//static int rdc200a_set_image_display_format(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
//										BOOL_T swap_image_3d_mode)
//#else
//int rdc200a_set_image_display_format(void)
//#endif
//{
////	UINT_T panel_offon_mask = 0x0;
////	U8_T dimension_reg_val[MAX_NUM_VC_DISPLAY_FORMAT] = {
////		0/*2D*/, SBS3D_EN, TAB3D_EN,
////		SBS_LL_3D_EN, FBF_3D_EN, FBF_EX_SIG
////	};
//
////	if ((fmt != VC_IMAGE_DISPLAY_FORMAT_2D) && swap_image_3d_mode)
////		dimension_reg_val[fmt] |= PANEL_SWAP;
//
//    // panel off
//    RDC_REG_SET(0x0015,	0x33);	//rdc200a_drive_panel(FALSE);
//    // reset on/off
//    RDC_REG_SET(0x0013,	0xFB);	//RDC_REG_MASK_SET(0x0013, 0x04, 0x00);
//    RDC_REG_SET(0x0013,	0xFF);	//RDC_REG_MASK_SET(0x0013, 0x04, 0x04);
//    // 3d func
//    RDC_REG_SET(0x0014,	0x01);	//RDC_REG_MASK_SET(0x0014, 0xFA, dimension_reg_val[fmt]); //2D mode
//    // pgen on
//    RDC_REG_SET(0x0200,	0x04);	//RDC_REG_MASK_SET(0x0200, 0x07, 0x04);
//    RDC_REG_SET(0x0280, 0x00);
//    // wait more than rx 3 frame
//    VC_U_DELAY(50000);//delay 50ms
//    // pgen  off
//    RDC_REG_SET(0x0200,	0x02);	//RDC_REG_SET(0x0200, rdc200a_mrx_config[vc_sel_rdc_idx]); //0x02 is for MIPI port; 0x00 is for LVDS port;
//    RDC_REG_SET(0x0280, 0x00);	//RDC_REG_SET(0x0280, rdc200a_pgen_config[vc_sel_rdc_idx]);
//    // panel on
//    RDC_REG_SET(0x0015,	0x3F);	//rdc200a_drive_panel(TRUE);
//
//	return 0;
//}

uint8_t RDC_REG_GET2(uint16_t reg)
{
#if 0
	// TODO get byte data
	uint16_t timeout = 0;
	uint8_t read_data = 0;
	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_WRITE , (uint8_t*)&reg, 2, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_READ , (uint8_t*)&read_data, 1, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
#else
	uint8_t read_data = 0;
	// CH: Test if a value of an input variable passed one byte size
	if (reg > 0xff)
		I2CReadBytes16(&RDC200A_I2C1, RDC200A_7bit_ADDR, reg, (uint8_t*)&read_data, 1);
	else
		I2CReadBytes(&RDC200A_I2C1, RDC200A_7bit_ADDR, (uint8_t)reg, (uint8_t*)&read_data, 1);
//	DPRINTF("RDC_REG_GET: reg: 0x%x read_data: 0x%x", reg, read_data);
#endif
	return read_data;

}

void RDC_REG_SET2(uint16_t reg, uint8_t val)
{
#if 1
	uint16_t timeout = 0;

		uint8_t data[3];
		uint16_t size = 3;

		data[0] = (uint8_t)((reg&0xff00)>>8);
		data[1] = (uint8_t)reg&0xff;
		data[2] = (uint8_t) (val & 0xFF);

	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_WRITE , (uint8_t*)&data, size, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
#else
	// CH: Test if a value of an input variable passed one byte size
	if (reg > 0xff)
		I2CWriteBytes16(&RDC200A_I2C1, RDC200A_7bit_ADDR, reg, (uint8_t*)&val, sizeof(val));
	else
		I2CWriteBytes(&RDC200A_I2C1, RDC200A_7bit_ADDR, (uint8_t)reg, (uint8_t*)&val, sizeof(val));
#endif
}

uint32_t RDC_REG_GET16(uint16_t reg)
{
#if 1
	// TODO get byte data
	uint16_t timeout = 0;
	uint8_t read_data[4] = {0};
	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_WRITE , (uint8_t*)&reg, 2, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_READ , (uint8_t*)&read_data, sizeof(read_data), I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
#else
	uint8_t read_data[2] = {0};
	I2CReadBytes16(&RDC200A_I2C1, RDC200A_7bit_ADDR, reg, (uint8_t*)&read_data, sizeof(read_data));
#endif
	uint32_t val = ((uint32_t)read_data[3] << 24 | (uint32_t)read_data[2] << 16 | (uint32_t)read_data[1] << 8) | (uint32_t)read_data[0];
	return val;
}

uint32_t RDC_REG_GET32(uint16_t reg)
{
#if 1
	// TODO get byte data
	uint16_t timeout = 0;
	uint8_t read_data[4] = {0};
	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_WRITE , (uint8_t*)&reg, 2, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);

	if(HAL_I2C_Master_Receive(&RDC200A_I2C1, (RDC200A_7bit_ADDR << 1) | I2C_READ , (uint8_t*)&read_data, sizeof(read_data), I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
#else
	uint8_t read_data[4] = {0};
	I2CReadBytes16(&RDC200A_I2C1, RDC200A_7bit_ADDR, reg, (uint8_t*)&read_data, sizeof(read_data));
#endif
	uint32_t val = ((uint32_t)read_data[3] << 24 | (uint32_t)read_data[2] << 16 | (uint32_t)read_data[1] << 8) | (uint32_t)read_data[0];
	return val;
}
