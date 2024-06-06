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

#ifndef __RDC200_FLASH_PARTITIONS_H__
#define __RDC200_FLASH_PARTITIONS_H__

#ifdef __cplusplus 
extern "C"{
#endif

#include "rti_vc_types.h"

#define MAX_NUM_PANEL	2

typedef enum {
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP501H = 0,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP360H = 1,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP550F = 2,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP700Q = 3,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP551F = 4,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP502H = 5,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP370F = 6,
	FLASH_FW_CFG_PANEL_MODEL_IDX_RDP250H = 7,
	MAX_NUM_FLASH_FW_CFG_PANEL_MODEL_IDX
} E_FLASH_FW_CFG_PANEL_MODEL_IDX_T;

typedef enum {
	FLASH_FW_RESOULTION_IDX_VGA        	= 0x00,
	FLASH_FW_RESOULTION_IDX_WVGA       	= 0x01,	
	FLASH_FW_RESOULTION_IDX_QFHD       	= 0x02,	
	FLASH_FW_RESOULTION_IDX_WSVGA      	= 0x03,	
	FLASH_FW_RESOULTION_IDX_XGA        	= 0x04,	
	FLASH_FW_RESOULTION_IDX_HD         	= 0x05,	
	FLASH_FW_RESOULTION_IDX_WXGA_S     	= 0x06,	
	FLASH_FW_RESOULTION_IDX_WXGA       	= 0x07,	
	FLASH_FW_RESOULTION_IDX_1K1K       	= 0x08,	
	FLASH_FW_RESOULTION_IDX_1080R      	= 0x09,	
	FLASH_FW_RESOULTION_IDX_MLO        	= 0x0A,	
	FLASH_FW_RESOULTION_IDX_SXGA       	= 0x0B,	
	FLASH_FW_RESOULTION_IDX_1600x900	= 0x0C,	
	FLASH_FW_RESOULTION_IDX_1440x108	= 0x0D,	
	FLASH_FW_RESOULTION_IDX_1280R      	= 0x0E,	
	FLASH_FW_RESOULTION_IDX_DUALHD     	= 0x0F,	
	FLASH_FW_RESOULTION_IDX_UXGA_S     	= 0x10,	
	FLASH_FW_RESOULTION_IDX_DUALWXGA_S 	= 0x11,	
	FLASH_FW_RESOULTION_IDX_UXGA       	= 0x12,	
	FLASH_FW_RESOULTION_IDX_DUALWXGA   	= 0x13,	
	FLASH_FW_RESOULTION_IDX_1440R      	= 0x14,	
	FLASH_FW_RESOULTION_IDX_FHD        	= 0x15,	
	FLASH_FW_RESOULTION_IDX_2K1K       	= 0x16,	
	FLASH_FW_RESOULTION_IDX_WQHD       	= 0x17,	
	FLASH_FW_RESOULTION_IDX_WQXGA      	= 0x18, 
	FLASH_FW_RESOULTION_IDX_2K2K       	= 0x1A, 
	FLASH_FW_RESOULTION_IDX_1280x728	= 0x1B, 
	FLASH_FW_RESOULTION_IDX_PVGA       	= 0x20, 
	FLASH_FW_RESOULTION_IDX_PWVGA      	= 0x21, 
	FLASH_FW_RESOULTION_IDX_PQFHD      	= 0x22, 
	FLASH_FW_RESOULTION_IDX_PWSVGA     	= 0x23, 
	FLASH_FW_RESOULTION_IDX_PXGA       	= 0x24, 
	FLASH_FW_RESOULTION_IDX_PHD        	= 0x25, 
	FLASH_FW_RESOULTION_IDX_PWXGA_S    	= 0x26, 
	FLASH_FW_RESOULTION_IDX_PWXGA      	= 0x27, 
	FLASH_FW_RESOULTION_IDX_PMLO       	= 0x2A, 
	FLASH_FW_RESOULTION_IDX_PSXGA      	= 0x2B, 
	FLASH_FW_RESOULTION_IDX_900x1600	= 0x2D, 
	FLASH_FW_RESOULTION_IDX_1080x144	= 0x2D, 
	FLASH_FW_RESOULTION_IDX_PDUALHD    	= 0x2F, 
	FLASH_FW_RESOULTION_IDX_PUXGA_S    	= 0x30, 
	FLASH_FW_RESOULTION_IDX_PDUALWXGA_S	= 0x31, 
	FLASH_FW_RESOULTION_IDX_PUXGA      	= 0x32, 
	FLASH_FW_RESOULTION_IDX_PDUALWXGA  	= 0x33, 
	FLASH_FW_RESOULTION_IDX_PFHD       	= 0x35, 
	FLASH_FW_RESOULTION_IDX_P2K1K      	= 0x36, 
	FLASH_FW_RESOULTION_IDX_PWQHD      	= 0x37, 
	FLASH_FW_RESOULTION_IDX_PWQXGA     	= 0x38, 
	FLASH_FW_RESOULTION_IDX_PDUALFHD   	= 0x39, 
	FLASH_FW_RESOULTION_IDX_MANUAL  	= 0x3F,
	MAX_NUM_FLASH_FW_RESOULTION_IDX
} E_FLASH_FW_RESOULTION_IDX_T;

typedef enum {
	FLASH_FW_CFG_DIF_PATH_IDX_LVDS = 0,
	FLASH_FW_CFG_DIF_PATH_IDX_LVCMOS = 1,
	FLASH_FW_CFG_DIF_PATH_IDX_MIPI = 2,
	FLASH_FW_CFG_DIF_PATH_IDX_HDMI = 3,
	FLASH_FW_CFG_DIF_PATH_IDX_PG = 4, // Pattern Generator
	FLASH_FW_CFG_DIF_PATH_IDX_MIXED_RGB_0 = 5, // Mixed RGB (LVCMOS 0 + LVCMOS 1) with LVCMOS 0's pclk
	FLASH_FW_CFG_DIF_PATH_IDX_MIXED_RGB_1 = 6, // Mixed RGB (LVCMOS 0 + LVCMOS 1) with LVCMOS 1's pclk
	FLASH_FW_CFG_DIF_PATH_IDX_DIS = 7, // Disable all input
	MAX_NUM_FLASH_FW_CFG_DIF_PATH_IDX
} E_FLASH_FW_CFG_DIF_PATH_IDX_T;

typedef enum {
	FLASH_FW_CFG_LVDS_PCLK_PORT0 = 0,
	FLASH_FW_CFG_LVDS_PCLK_PORT1 = 1,
	MAX_NUM_FLASH_FW_CFG_LVDS_PCLK
} E_FLASH_FW_CFG_LVDS_PLCK_T;

typedef enum {
	FLASH_FW_CFG_LVDS_PORT_SINGLE_0 = 0,
	FLASH_FW_CFG_LVDS_PORT_SINGLE_1 = 1,
	FLASH_FW_CFG_LVDS_PORT_DUAL_0 = 2,
	FLASH_FW_CFG_LVDS_PORT_DUAL_1 = 3,
	MAX_NUM_FLASH_FW_CFG_LVDS_PORT
} E_FLASH_FW_CFG_LVDS_PORT_T;

typedef enum {
	FLASH_FW_CFG_LVDS_REV_BUS_DIS = 0,
	FLASH_FW_CFG_LVDS_REV_BUS_0 = 1,
	FLASH_FW_CFG_LVDS_REV_BUS_1 = 2,
	FLASH_FW_CFG_LVDS_REV_BUS_BOTH = 3,
	MAX_NUM_FLASH_FW_CFG_LVDS_REV_BUS
} E_FLASH_FW_CFG_LVDS_REV_BUS_T;

typedef enum {
	FLASH_FW_CFG_LVDS_COLOR_DEPTH_24 = 0,
	FLASH_FW_CFG_LVDS_COLOR_DEPTH_18 = 1,
	MAX_NUM_FLASH_FW_CFG_LVDS_COLOR_DEPTH
} E_FLASH_FW_CFG_LVDS_COLOR_DEPTH_T;

typedef enum {
	FLASH_FW_CFG_RDC_INIT_SEL_FILE = 0,
	FLASH_FW_CFG_RDC_INIT_SEL_CFG = 1,
	MAX_NUM_FLASH_FW_CFG_RDC_INIT_SEL
} E_FLASH_FW_CFG_RDC_INIT_SEL_T;

// DR: Data Rate
typedef enum {
	FLASH_FW_CFG_MIPI_DR_500Mbps = 0,
	FLASH_FW_CFG_MIPI_DR_700Mbps = 1,
	FLASH_FW_CFG_MIPI_DR_864Mbps = 2,
	FLASH_FW_CFG_MIPI_DR_1000Mbps = 3,
	FLASH_FW_CFG_MIPI_DR_1344Mbps = 4,
	FLASH_FW_CFG_MIPI_DR_1500Mbps = 5,
	MAX_NUM_FLASH_FW_CFG_MIPI_DR
} E_FLASH_FW_CFG_MIPI_DR_T;

typedef enum
{
	FLASH_FW_CFG_MIPI_NUM_LANE_1 = 0,
	FLASH_FW_CFG_MIPI_NUM_LANE_2 = 1,
	FLASH_FW_CFG_MIPI_NUM_LANE_3 = 2,
	FLASH_FW_CFG_MIPI_NUM_LANE_4 = 3,
	MAX_NUM_FLASH_FW_CFG_MIPI_NUM_LANE
} E_FLASH_FW_CFG_MIPI_NUM_LANE_T;

// Auto power down(PD)
typedef enum {
	FLASH_FW_CFG_MIPI_AUTO_PD_pwr_up_inactive = 0,
	FLASH_FW_CFG_MIPI_AUTO_PD_pwr_down_inactive = 1,
	MAX_NUM_FLASH_FW_CFG_MIPI_AUTO_PD
} E_FLASH_FW_CFG_MIPI_AUTO_PD_T;

// Continnous clock mode
typedef enum {
	FLASH_FW_CFG_MIPI_CONT_CLK_none = 0,
	FLASH_FW_CFG_MIPI_CONT_CLK_cont = 1,
	MAX_NUM_FLASH_FW_CFG_MIPI_CONT_CLK
} E_FLASH_FW_CFG_MIPI_CONT_CLK_T;

#if defined(_WIN32) || defined(WINDOWS)
#pragma pack(push, 1)
#endif

/*
Configuration Version
*/

#define RDC_FW_CFG_VER_MAJOR		2
#define RDC_FW_CFG_VER_MINOR		0
#define RDC_FW_CFG_VER_REL			0

#define FLASH_FW_CFG_MAGIC_CODE		0xDEAD4EAD

/*
 Fiwmware configuration information.
 Save as the big-endian format.
*/
typedef struct {
	U32_T magic_code;        // offset[0x00], 4 bytes
	U32_T version;           // offset[0x04], VC version: [0x04](Major), [0x05](Minor), [0x06](release), [0x07](reserved)

	U8_T panel_model;       // offset[0x08],
	U8_T panel_attached_port_bits; // offset[0x09], for RDP501H (No has I2C)
	U8_T I2C_addr_panel_0;  // offset[0x0A], [6:0]: addr
	U8_T I2C_addr_panel_1;  // offset[0x0B], [6:0]: addr
	U8_T rdc_init_sel;      // offset[0x0C], 0: init file, 1: configuration data

	U8_T input_resoultion;  // offset[0x0D],
	U8_T output_resoultion; // offset[0x0E],

	U8_T dif_path;          // offset[0x0F],
	U16_T h_blank_total;
	U16_T v_blank_total;
	U16_T frame_rate;

	U8_T lvds_pclk;
	U8_T lvds_port;
	U8_T lvds_rev_bus;
	U8_T lvds_color_depth;

	U8_T mipi_video_mode;
	U8_T mipi_disable_eotp;
	U8_T mipi_bllp_packet;
	U8_T mipi_num_lane;
	U8_T mipi_auto_pd;
	U8_T mipi_cont_clk;

	/*
     * Determine if the firmware apply 1 or separate lut of gamma to each panel.
     * false: Apply to each panle with 1 LUT.
     * true:  Applied to each panle with 2 LUTs with different "lut_offset."
	 */
	U8_T gamma_apply_separate_lut;

	U8_T bootlogo_en; // [7:0]  0x01: enable, 0x00 or 0xFF: disable
	U8_T bootlogo_lut;
	U8_T mag; // [7:0] used
	U8_T debug_en; // 1: enable, 0: disable: fw cfg, build date, i2c fail, boot completed
	U16_T pos_x; // [7:0]
	U16_T pos_y; // [7:0]

	/* Power on/off Panel when boot time. 0: power off, 1: power on */
	U8_T boot_panel_on;

	/* Vsync signal indicator */
	U8_T vsync_sig_indicator;
	U16_T vsync_pulse_pos;

	/* Determine if the firmware apply the below cokoa setting when bootup time. */
	U8_T boot_cokoa_apply;

	/*
     * Determine if the firmware apply 1 or separate lut of CoKOA to each panel.
     * false: Apply to each panle with 1 LUT.
     * true:  Applied to each panle with 2 LUTs with different "lut_offset."
     *		  if "boot_cokoa_apply" is 1.
	 */
	U8_T cokoa_apply_separate_lut;

	/* Enable/disable CoKOA when boot time. */
	U8_T cokoa_en_boot_panel_0;
	U8_T cokoa_en_boot_panel_1;

	/* Enable/disable filp when bootup time. */
	U8_T flip_en_boot_panel_apply;
	U8_T flip_vertical_en_boot_panel;
	U8_T flip_horizontal_en_boot_panel;

	/* Select 2D/3D dimension format when bootup time. */
	U8_T boot_display_dimension_apply;
	U8_T boot_image_swap_panel;
	U8_T boot_display_dimension_format;

	U8_T lvds_tx_pin_mux_apply;

	/* LVDS Tx Pin Mux for tx0 */
	U32_T ldt0_ch_switch;
	U8_T ldt0_data_inv;
	U8_T ldt0_clk_inv;

	/* LVDS Tx Pin Mux for tx1 */
	U32_T ldt1_ch_switch;
	U8_T ldt1_data_inv;
	U8_T ldt1_clk_inv;

	/* LED on/off time configuration */
	U8_T led_on_off_time_apply;

	U16_T tx0_led_p0_on_red;
	U16_T tx0_led_p0_off_red;
	U16_T tx0_led_p1_on_red;
	U16_T tx0_led_p1_off_red;
	U16_T tx0_led_p0_on_green;
	U16_T tx0_led_p0_off_green;
	U16_T tx0_led_p1_on_green;
	U16_T tx0_led_p1_off_green;
	U16_T tx0_led_p0_on_blue;
	U16_T tx0_led_p0_off_blue;
	U16_T tx0_led_p1_on_blue;
	U16_T tx0_led_p1_off_blue;

	U16_T tx1_led_p0_on_red;
	U16_T tx1_led_p0_off_red;
	U16_T tx1_led_p1_on_red;
	U16_T tx1_led_p1_off_red;
	U16_T tx1_led_p0_on_green;
	U16_T tx1_led_p0_off_green;
	U16_T tx1_led_p1_on_green;
	U16_T tx1_led_p1_off_green;
	U16_T tx1_led_p0_on_blue;
	U16_T tx1_led_p0_off_blue;
	U16_T tx1_led_p1_on_blue;
	U16_T tx1_led_p1_off_blue;

	U8_T auto_refresh_rate_config_apply;

	U8_T auto_vc_vt_apply;
	U8_T img_ss_disable;
} GNUPACKED FLASH_FW_CFG_INFO_T;

#define FLASH_RDC_PATCH_MAGIC_CODE		0x1EDA4C3D
#define FLASH_PANEL_0_PATCH_MAGIC_CODE  0xFCBA0D10
#define FLASH_PANEL_1_PATCH_MAGIC_CODE  0xFCBB0D10
    
/**
 RDC/Panel LUT node.
*/
typedef struct {
	U16_T	reg;
	U8_T	val;
} GNUPACKED RD_LUT_NODE_T;

typedef struct {
	U32_T magic_code;
	U16_T num_node;
} GNUPACKED RD_LUT_HEADER_T;

/* RDC record type stored in FLASH. */
typedef struct {
	RD_LUT_HEADER_T	hdr; /* LUT Header */
	RD_LUT_NODE_T node[1]; /* N nodes */
} GNUPACKED RD_LUT_RECORD_T;

// Used to init LUT . To be changed in futher.....
typedef struct {
    U8_T panel_idx;
    U16_T num_set; // Number of data set (address and value).
    U32_T next_hdr; // next header offset. 0x0 = none.
} GNUPACKED PANEL_INIT_DATA_HEADER_T;

#if defined(_WIN32) || defined(WINDOWS) || defined(_WIN64)
#pragma pack(pop)
#endif

#define RDC200A_FLASH_BLOCK_SIZE		0x8000
#define RDC200A_FLASH_SECTOR_SIZE		0x1000
#define RDC200A_FLASH_PAGE_SIZE			256
#define RDC200A_FLASH_PAGE_SHIFT		8


/*=============================================================================
*
* Partitions
*
* NOTE: Assum that the size of FLASH block is 32KB(max)
*       and the size of FLASH sector is 4KB.
*=============================================================================*/
#define FLASH_PART_OFFSET_BOOT_FW				0x000000
	#define FLASH_PART_SIZE_BOOT_FW				0x8000

#define FLASH_PART_OFFSET_FW_CFG				0x008000
	#define FLASH_PART_SIZE_FW_CFG				0x1000 // 4KB

#define FLASH_PART_OFFSET_RDC200A_INIT_REG		0x009000
	#define FLASH_PART_SIZE_RDC200A_INIT			0x2000 // 8KB
	#define FLASH_ITEM_SIZE_RDC200A_INIT			0x2000 // 8KB. MAX: count(2 bytes) [addr(2 bytes) + data(1byte) ....]

#define FLASH_PART_OFFSET_PANEL_INIT_BASE		0x00B000
	#define FLASH_PART_SIZE_PANEL_INIT			0x14000 // 80KB
	#define FLASH_ITEM_SIZE_PANEL_INIT			0x2000 // 8KB. addr(2 bytes) + data(1byte) + sizeof(PANEL_LUT_HEADER_T): per panel

	#define FLASH_ITEM_ADDR_PANEL_INIT(lut_offset)\
		(FLASH_PART_OFFSET_PANEL_INIT_BASE + (FLASH_ITEM_SIZE_PANEL_INIT * (lut_offset)))

/* Gamma Correction. */
#define FLASH_PART_OFFSET_PANEL_GAMMA_BASE		0x01F000 // port 0
	#define FLASH_PART_SIZE_PANEL_GAMMA_LUT		0x1000 // each LUT item size
	
	#define FLASH_SUBITEM_SIZE_PANNEL_GAMMA		512 // each sub-item size
	#define FLASH_REAL_LUT_SIZE_PANEL_GAMMA		(6 * FLASH_SUBITEM_SIZE_PANNEL_GAMMA) // real LUT size

	// Memory offset in a LUT.
	#define FLASH_ITEM_OFFSET_PANEL_GAMMA(lut_offset)\
		(FLASH_PART_SIZE_PANEL_GAMMA_LUT * (lut_offset))

	#define FLASH_ITEM_OFFSET_GAMMA_LOW_RED(lut_offset)\
		(FLASH_ITEM_OFFSET_PANEL_GAMMA(lut_offset))

	#define FLASH_ITEM_OFFSET_GAMMA_LOW_GREEN(lut_offset)\
		(FLASH_ITEM_OFFSET_GAMMA_LOW_RED(lut_offset) + FLASH_SUBITEM_SIZE_PANNEL_GAMMA)

	#define FLASH_ITEM_OFFSET_GAMMA_LOW_BLUE(lut_offset)\
		(FLASH_ITEM_OFFSET_GAMMA_LOW_GREEN(lut_offset) + FLASH_SUBITEM_SIZE_PANNEL_GAMMA)

	#define FLASH_ITEM_OFFSET_GAMMA_HIGH_RED(lut_offset)\
		(FLASH_ITEM_OFFSET_GAMMA_LOW_BLUE(lut_offset) + FLASH_SUBITEM_SIZE_PANNEL_GAMMA)

	#define FLASH_ITEM_OFFSET_GAMMA_HIGH_GREEN(lut_offset)\
		(FLASH_ITEM_OFFSET_GAMMA_HIGH_RED(lut_offset) + FLASH_SUBITEM_SIZE_PANNEL_GAMMA)

	#define FLASH_ITEM_OFFSET_GAMMA_HIGH_BLUE(lut_offset)\
		(FLASH_ITEM_OFFSET_GAMMA_HIGH_GREEN(lut_offset) + FLASH_SUBITEM_SIZE_PANNEL_GAMMA)

/*
 Distortion Correction (Optic)

 -40 degree: base + sector_size x 0 (RED, GREEN, BLUE)
 -39 degree: base + sector_size x 1
 	.
  0 degree: base + sector_size x 40	
	.
 +40 degree: base + sector_size x 80
*/
#define FLASH_PART_OFFSET_DC				0x027000 // port 0

	#define FLASH_PART_DC_ALLOC_LUT_SIZE		0x1000 // 4KB. each LUT allocation size

	#define FLASH_ITEM_SIZE_DC					0x4e6 // 1 item: 1254 bytes
	#define FLASH_NUM_ITEM_DC_PER_SECTOR		3
	#define FLASH_DC_LUT_INFO_SIZE				(FLASH_NUM_ITEM_DC_PER_SECTOR * FLASH_ITEM_SIZE_DC)

	#define FLASH_ITEM_NUM_DC_SET				(2/*port*/ * 82)

	#define MIN_DC_DEGREE			-40
	#define MAX_DC_DEGREE			40

	// Memory offset in a LUT.
	#define FLASH_ITEM_OFFSET_DC(lut_offset)\
		(FLASH_PART_DC_ALLOC_LUT_SIZE * (lut_offset))
	
	#define FLASH_ITEM_OFFSET_DC_RED(lut_offset)\
		(FLASH_ITEM_OFFSET_DC(lut_offset) + FLASH_ITEM_SIZE_DC * 0)

	#define FLASH_ITEM_OFFSET_DC_GREEN(lut_offset)\
		(FLASH_ITEM_OFFSET_DC(lut_offset) + FLASH_ITEM_SIZE_DC * 1)

	#define FLASH_ITEM_OFFSET_DC_BLUE(lut_offset)\
		(FLASH_ITEM_OFFSET_DC(lut_offset) + FLASH_ITEM_SIZE_DC * 2)


#define FLASH_PART_OFFSET_RDC_PATCH_BASE	0x0CB000
	#define FLASH_PART_SIZE_RDC_PATCH		0x1000

#define FLASH_PART_OFFSET_PANEL_0_PATCH_BASE		0x0CC000
#define FLASH_PART_OFFSET_PANEL_1_PATCH_BASE		0x0CD000
	#define FLASH_PART_SIZE_PANEL_PATCH		0x1000

#define FLASH_PART_OFFSET_FONT				0x0CE000
	#define FLASH_PART_SIZE_FONT			0x10000
	#define FLASH_ITEM_SIZE_FONT			0x1000 // each item size

	#define MAX_NUM_FLASH_ITEM_FONT			16

#define FLASH_PART_OFFSET_PALETTE			0x0DE000
	#define FLASH_PART_SIZE_PALETTE			0x2000
	#define FLASH_ITEM_SIZE_PALETTE			48 // each item size (0x30)
	#define FLASH_SET_SIZE_PALETTE			0x1000 // The size of color set.
	#define FLASH_COLOR_SET_ALIGN_SIZE		0x100

	#define MAX_NUM_FLASH_ITEM_PALETTE		2

	#define FLASH_ITEM_OFFSET_PALETTE(lut_offset)\
		(FLASH_SECTOR_SIZE * (lut_offset))
	

#define FLASH_PART_OFFSET_BMP				0x0E0000
	#define FLASH_ITEM_SIZE_BMP				0x4000 // each item size

	#define FLASH_ITEM_SIZE_SHIFT			14
	#define FLASH_ITEM_ACTUAL_SIZE_BMP		15360 

	#define MAX_NUM_FLASH_ITEM_BMP			16

	#define FLASH_ITEM_ADDR_BMP(lut_offset)\
		(FLASH_PART_OFFSET_BMP + ((U32_T)(lut_offset) << FLASH_ITEM_SIZE_SHIFT))

/*=============================================================================
*
* Target SRAM ID
*
*=============================================================================*/
/* CoKOA(Distortion Correction) */
#define TSID_TX0_CoKOA_RED		0x00
#define TSID_TX0_CoKOA_GREEN	0x01
#define TSID_TX0_CoKOA_BLUE		0x02

#define TSID_TX1_CoKOA_RED		0x10
#define TSID_TX1_CoKOA_GREEN	0x11
#define TSID_TX1_CoKOA_BLUE		0x12

/* Gamma Correction */
#define TSID_TX0_DIG_GAMMA_RED_POL_H	0x03
#define TSID_TX0_DIG_GAMMA_GREEN_POL_H	0x04
#define TSID_TX0_DIG_GAMMA_BLUE_POL_H	0x05
#define TSID_TX0_DIG_GAMMA_RED_POL_L	0x06
#define TSID_TX0_DIG_GAMMA_GREEN_POL_L	0x07
#define TSID_TX0_DIG_GAMMA_BLUE_POL_L	0x08

#define TSID_TX1_DIG_GAMMA_RED_POL_H	0x13
#define TSID_TX1_DIG_GAMMA_GREEN_POL_H	0x14
#define TSID_TX1_DIG_GAMMA_BLUE_POL_H	0x15
#define TSID_TX1_DIG_GAMMA_RED_POL_L	0x16
#define TSID_TX1_DIG_GAMMA_GREEN_POL_L	0x17
#define TSID_TX1_DIG_GAMMA_BLUE_POL_L	0x18

/* Bitmap drawer */
#define TSID_BMP_DRAWER_0			0x80
#define TSID_BMP_DRAWER_1			0x81
#define TSID_BMP_DRAWER_2			0x82
#define TSID_BMP_DRAWER_3			0x83

/* Font Bitmap for text box */
#define TSID_FONT_BMP_0				0x84
#define TSID_FONT_BMP_1				0x85
#define TSID_FONT_BMP_2				0x86
#define TSID_FONT_BMP_3				0x87

/* font/bg color for text box */
#define TSID_FONT_BG_COLOR_0		0x88
#define TSID_FONT_BG_COLOR_1		0x89
#define TSID_FONT_BG_COLOR_2		0x8A
#define TSID_FONT_BG_COLOR_3		0x8B

#ifdef __cplusplus
}
#endif

#endif // __RDC200_FLASH_PARTITIONS_H__

