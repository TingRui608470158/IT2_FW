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

#ifndef __RTI_VC_API_H__
#define __RTI_VC_API_H__

#include "rti_vc_config.h"
#include "rti_vc_types.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * RDP register size in bytes
 */
#if defined(CFG_PANEL_RDP501H) || defined(CFG_PANEL_RDP360H) || defined(CFG_PANEL_RDP550F)
	#define VC_RDP_REG_SIZE		1

#elif defined(CFG_PANEL_RDP551F) || defined(CFG_PANEL_RDP700Q)\
	|| defined(CFG_PANEL_RDP502H) || defined(CFG_PANEL_RDP370F)\
	|| defined(CFG_PANEL_RDP250H)
	#define VC_RDP_REG_SIZE		2

#else
	#error "Code not present"
#endif

/**
 * @brief Micro Display Controller device types.
 */
typedef enum {
	VC_MDC_DEVICE_RDC100 = 0, /* RDC100 */
	VC_MDC_DEVICE_RDC200 = 1, /* RDC200 */
	VC_MDC_DEVICE_RDC200A = 2, /* RDC200A */
	MAX_NUM_VC_MDC_DEVICE
} E_VC_MDC_DEVICE_T;

/**
 * @brief Panel device types.
 */
typedef enum {
	VC_PANEL_DEVICE_RDP501H = 0,
	VC_PANEL_DEVICE_RDP360H = 1,
	VC_PANEL_DEVICE_RDP550F = 2,
	VC_PANEL_DEVICE_RDP700Q = 3,
	VC_PANEL_DEVICE_RDP551F = 4,
	VC_PANEL_DEVICE_RDP502H = 5,
	VC_PANEL_DEVICE_RDP370F = 6,
	VC_PANEL_DEVICE_RDP250H = 7,
	MAX_NUM_VC_PANEL_DEVICE
} E_VC_PANEL_DEVICE_T;

/**
 * @brief Device channel type for I2C interface.
 */
typedef enum {
	VC_DEVICE_CH_INVALID = -1,

	/** RDC and RDP of port 0 are use the same I2C line. */
	VC_DEVICE_CH_RDC_RDP0 = 0,

	 /** RDP of port 1 */
	VC_DEVICE_CH_RDP1,
	MAX_NUM_VC_DEVICE_CH
} E_VC_DEVICE_CH_T;

/**
 * @brief Panel port types.
 */
typedef enum {
	VC_PANEL_PORT_0 = 0,
	VC_PANEL_PORT_1 = 1,
	MAX_NUM_VC_PANEL_PORT
} E_VC_PANEL_PORT_T;

/**
 * @brief Index of RDC devices.
 */
typedef enum {
	VC_RDC_INDEX_PRIMARY = 0,

	/** Used if present */
	VC_RDC_INDEX_SECONDARY = 1
} E_VC_RDC_INDEX_T;

/**
 * @brief Input resolution type.
 */
typedef enum {
	VC_INPUT_RES_VGA         = 0x00, /* ( 640 x  480) */
	VC_INPUT_RES_WVGA        = 0x01, /* ( 854 x  480) */
	VC_INPUT_RES_QFHD        = 0x02, /* ( 960 x  540) */
	VC_INPUT_RES_WSVGA       = 0x03, /* (1024 x  600) */
	VC_INPUT_RES_XGA         = 0x04, /* (1024 x  768) */
	VC_INPUT_RES_HD          = 0x05, /* (1280 x  720) */
	VC_INPUT_RES_WXGA_S      = 0x06, /* (1280 x  768) */
	VC_INPUT_RES_WXGA        = 0x07, /* (1280 x  800) */
	VC_INPUT_RES_1K1K        = 0x08, /* (1024 x 1024) */
	VC_INPUT_RES_1080R       = 0x09, /* (1080 x 1080) */
	VC_INPUT_RES_MLO         = 0x0a, /* (1280 x  960) */
	VC_INPUT_RES_SXGA        = 0x0b, /* (1280 x 1024) */
	VC_INPUT_RES_1600X900    = 0x0c, /* (1600 x  900) */
	VC_INPUT_RES_1440X1080   = 0x0d, /* (1440 x 1080) */
	VC_INPUT_RES_1280R       = 0x0e, /* (1280 x 1280) */
	VC_INPUT_RES_DUALHD      = 0x0f, /* (2560 x  720) */
	VC_INPUT_RES_UXGA_S      = 0x10, /* (1600 x 1200) */
	VC_INPUT_RES_DUALWXGA_S  = 0x11, /* (2560 x  768) */
	VC_INPUT_RES_UXGA        = 0x12, /* (1600 x 1280) */
	VC_INPUT_RES_DUALWXGA    = 0x13, /* (2560 x  800) */
	VC_INPUT_RES_1440R       = 0x14, /* (1440 x 1440) */
	VC_INPUT_RES_FHD         = 0x15, /* (1920 x 1080) */
	VC_INPUT_RES_2K1K        = 0x16, /* (2048 x 1024) */
	VC_INPUT_RES_WQHD        = 0x17, /* (2560 x 1440) */
	VC_INPUT_RES_WQXGA       = 0x18, /* (2560 x 1600) */
	VC_INPUT_RES_RESERVED0   = 0x19,
	VC_INPUT_RES_2K2K        = 0x1a, /* (2048 x 2048) */
	VC_INPUT_RES_1280X728    = 0x1b, /* (1280 x  728) */
	VC_INPUT_RES_RESERVED2   = 0x1c,
	VC_INPUT_RES_RESERVED3   = 0x1d,
	VC_INPUT_RES_RESERVED4   = 0x1e,
	VC_INPUT_RES_RESERVED5   = 0x1f,
	VC_INPUT_RES_PVGA        = 0x20, /* ( 480 x  640) */
	VC_INPUT_RES_PWVGA       = 0x21, /* ( 480 x  854) */
	VC_INPUT_RES_PQFHD       = 0x22, /* ( 540 x  960) */
	VC_INPUT_RES_PWSVGA      = 0x23, /* ( 600 x 1024) */
	VC_INPUT_RES_PXGA        = 0x24, /* ( 768 x 1024) */
	VC_INPUT_RES_PHD         = 0x25, /* ( 720 x 1280) */
	VC_INPUT_RES_PWXGA_S     = 0x26, /* ( 768 x 1280) */
	VC_INPUT_RES_PWXGA       = 0x27, /* ( 800 x 1280) */
	VC_INPUT_RES_RESERVED6   = 0x28,
	VC_INPUT_RES_RESERVED7   = 0x29,
	VC_INPUT_RES_PMLO        = 0x2a, /* ( 960 x 1280) */
	VC_INPUT_RES_PSXGA       = 0x2b, /* (1024 x 1280) */
	VC_INPUT_RES_900X1600    = 0x2c, /* ( 900 x 1600) */
	VC_INPUT_RES_1080X1440   = 0x2d, /* (1080 x 1440) */
	VC_INPUT_RES_RESERVED8   = 0x2e,
	VC_INPUT_RES_PDUALHD     = 0x2f, /* ( 720 x 2560) */
	VC_INPUT_RES_PUXGA_S     = 0x30, /* (1200 x 1600) */
	VC_INPUT_RES_PDUALWXGA_S = 0x31, /* ( 768 x 2560) */
	VC_INPUT_RES_PUXGA       = 0x32, /* (1200 x 1600) */
	VC_INPUT_RES_PDUALWXGA   = 0x33, /* ( 800 x 2560) */
	VC_INPUT_RES_RESERVED9   = 0x34,
	VC_INPUT_RES_PFHD        = 0x35, /* (1080 x 1920) */
	VC_INPUT_RES_P2K1K       = 0x36, /* (1024 x 2048) */
	VC_INPUT_RES_PWQHD       = 0x37, /* (1440 x 2560) */
	VC_INPUT_RES_PWQXGA      = 0x38, /* (1600 x 2560) */
	VC_INPUT_RES_PDUALFHD    = 0x39, /* (1080 x 3840) */
	VC_INPUT_RES_RESERVEDA   = 0x3a,
	VC_INPUT_RES_RESERVEDB   = 0x3b,
	VC_INPUT_RES_RESERVEDC   = 0x3c,
	VC_INPUT_RES_RESERVEDD   = 0x3d,
	VC_INPUT_RES_RESERVEDE   = 0x3e,
	VC_INPUT_RES_ETC         = 0x3f /* (INPUT_H_SIZE_IDX, INPUT_V_SIZE_IDX) */
} VC_INPUT_RES_T;

/**
 * @brief LUT(LookUp Table) class type.
 */
typedef enum {
	VC_LUT_CLASS_RDC_FW = 0,
	VC_LUT_CLASS_RDC_INIT,

	VC_LUT_CLASS_PANEL_INIT,
	VC_LUT_CLASS_PANEL_GAMMA,

	/** Firmware configuration */
	VC_LUT_CLASS_RDC_CFG,
	VC_LUT_CLASS_PANEL_HC,
	VC_LUT_CLASS_PANEL_VC,

	/** CoKOA */
	VC_LUT_CLASS_OPTIC_DC,

	VC_LUT_CLASS_FONT,
	VC_LUT_CLASS_PALETTE,
	VC_LUT_CLASS_BMP,

	MAX_NUM_VC_LUT_CLASS
} E_VC_LUT_CLASS_T;

/**
 * @brief Download status types.
 */
typedef enum {
	/** Starting action for erase, program, verify and item processing */
	VC_DLOAD_STATUS_STARTING = 0,

	/** In progress processing for erasing, programming, verifying, item processing */
	VC_DLOAD_STATUS_INPROGRESS,

	/** Action completed for erase, program, verify, item processing without error */
	VC_DLOAD_STATUS_COMPLTED,

	/** Save the programmed other data in the overlapping sector before the operaion of erase */
	VC_DLOAD_STATUS_ERASE_SAVE,

	/** Restore the saved data after the operaion of erase. */
	VC_DLOAD_STATUS_ERASE_RESTORE,

	/** Forcely stop the operation of downloading by calling */
	VC_DLOAD_STATUS_FORCE_STOP,

	/** Failure in communication with device */
	VC_DLOAD_STATUS_DEV_FAIL,

	/** The programmed data and the original data are in different. */
	VC_DLOAD_STATUS_VERIFY_ERR,

	/** Shutdown the downloader calling by rtiVC_DeinitDownload() function. */
	VC_DLOAD_STATUS_SHUTDOWN,

	/** Erase the registered items for download. */
	VC_DLOAD_STATUS_REMOVE_SETUP_ITEM
} E_VC_DLOAD_STATUS_T;

/**
 * @brief Horizontal shift types.
 */
typedef enum {
	VC_SHIFT_HORIZONTAL_LEFT = 0,
	VC_SHIFT_HORIZONTAL_RIGHT
} E_VC_SHIFT_HORIZONTAL_T;

/**
 * @brief Vertical shift types.
 */
typedef enum {
	VC_SHIFT_VERTICAL_UP = 0,
	VC_SHIFT_VERTICAL_DOWN
} E_VC_SHIFT_VERTICAL_T;

/**
 * @brief Image display formate types.
 */
typedef enum {
	/** 2D image format */
	VC_IMAGE_DISPLAY_FORMAT_2D = 0,

	/** Side By Side 3D */
	VC_IMAGE_DISPLAY_FORMAT_3D_SBS = 1,

#if defined(CFG_MDC_RDC200A)
	/** Top and Bottom 3D */
	VC_IMAGE_DISPLAY_FORMAT_3D_TAB = 2,

	/** Lossless Side by Side 3D */
	VC_IMAGE_DISPLAY_FORMAT_3D_SBS_LL = 3,

	/** Frame By Frame 3D */
	VC_IMAGE_DISPLAY_FORMAT_3D_FBF = 4,

	/** Select pi_rsvd1 pad for Frame by frame indicator */
	VC_IMAGE_DISPLAY_FORMAT_3D_FBF_EX_SIG = 5,
#endif

	MAX_NUM_VC_DISPLAY_FORMAT
} E_VC_IMAGE_DISPLAY_FORMAT_T;

/**
 * @brief Color types to control LED duty and current.
 */
typedef enum {
	VC_LED_CTRL_COLOR_R = 0x1,
	VC_LED_CTRL_COLOR_G = 0x2,
	VC_LED_CTRL_COLOR_B = 0x4,
	VC_LED_CTRL_COLOR_ALL = 0x7
} E_VC_LED_CTRL_COLOR_T;

/**
 * @brief LED currnet information type.
 */
typedef struct {
	S32_T R;
	S32_T G;
	S32_T B;
} VC_LED_CURRENT_INFO_T;

/**
 * @brief Panel port types to control panel.
 */
typedef enum {
	VC_PANEL_CTRL_INVAILD = -1,

	/** Control for panel port 0 only */
	VC_PANEL_CTRL_PORT_0 = 0x1,

	/** Control for panel port 1 only */
	VC_PANEL_CTRL_PORT_1 = 0x2,

	/** Control for panel port all */
	VC_PANEL_CTRL_PORT_ALL = 0x3
} E_VC_PANEL_CTRL_PORT_T;

/**
 * @brief LED control mode for firmware requests.
 */
typedef enum {
	VC_LED_CTRL_MODE_BRIGHTNESS	= 0,
	VC_LED_CTRL_MODE_COLOR
} E_VC_LED_CTRL_MODE;

/**
 * @brief LED control information for firmware requests.
 */
typedef struct {
	E_VC_LED_CTRL_MODE	mode;

	union {
		/** The value of brightness when brightness mode */
		U8_T brightness;

		/** The color parameters when color mode */
		struct {
			E_VC_LED_CTRL_COLOR_T ctrl;
			U8_T R;
			U8_T G;
			U8_T B;
		} color;
	} param;
} VC_LED_CTRL_INFO_T;

#define VC_LED_DUTY_DIVIDER		1000

#define VC_LED_DUTY_TO_PERCENTAGE(duty)\
	((double)(duty) / (double)VC_LED_DUTY_DIVIDER)

#define VC_FRAME_RATE_DIVIDER		1000

#define VC_FRAME_RATE_TO_HZ(frame_rate)\
	((double)(frame_rate) / (double)VC_FRAME_RATE_DIVIDER)

/**
 * @brief LED duty information.
 */
typedef struct {
	S32_T R;
	S32_T G;
	S32_T B;
} VC_LED_DUTY_INFO_T;

/**
 * @brief Callback function type to know the status of erasing operation
 *        during download.
 */
typedef void (*VC_DLOAD_ERASE_CALLBACK_T)(E_VC_DLOAD_STATUS_T status,
											E_VC_LUT_CLASS_T lut_class,
											U32_T curr_erase_addr,
											U16_T erased_sector_cnt,
											U16_T total_sector);

/**
 * @brief Callback function type to know the status of programming operation
 *        during download.
 */
typedef void (*VC_DLOAD_PROGRAM_CALLBACK_T)(E_VC_DLOAD_STATUS_T status,
											E_VC_LUT_CLASS_T lut_class,
											U32_T curr_pgm_addr,
											U16_T programmed_bytes,
											U16_T total_pgm_bytes);

/**
 * @brief Callback function type to know the status of verification operation
 *        during download.
 */
typedef void (*VC_DLOAD_VERIFY_CALLBACK_T)(E_VC_DLOAD_STATUS_T status,
											E_VC_LUT_CLASS_T lut_class,
											U32_T curr_verify_addr,
											U16_T verified_bytes,
											U16_T total_verify_bytes);

/**
 * @brief Callback function type to know the status of item's processing
 *        during download.
 */
typedef void (*VC_DLOAD_PROC_CALLBACK_T)(E_VC_DLOAD_STATUS_T status,
											E_VC_LUT_CLASS_T lut_class,
											U8_T *pgm_data_buf_ptr,
											U16_T processed_item_cnt,
											U16_T total_item);


/*=============================================================================
 Panel APIs
=============================================================================*/
int rtiVC_GetPanelDeviceID(E_VC_PANEL_PORT_T port, U16_T *chip_id, U16_T *rev_num);

#ifdef CFG_PANEL_RDP501H
void rtiVC_WriteRDP501H_1(U8_T reg, U8_T val);
void rtiVC_WriteRDP501H_0(U8_T reg, U8_T val);
#endif

/**
 * @brief Panel temperature information struture.
 */
typedef struct {
	/** Flag for OTP was applied or not */
	BOOL_T	otp_applied;

	/** The previous code of temperature sensor */
	int		prev_tsensor_code;

	/** The previous temperature value in degree celsius */
	S32_T	prev_temperature;

	/** The current code of temperature sensor */
	int		tsensor_code;

	/** The current temperature value in degree celsius */
	S32_T	temperature;
} VC_PANEL_TEMPERATURE_INFO_T;

/* Divider for temperature degree. */
#define VC_TEMPERATURE_DEGREE_DIV		1000

int rtiVC_GetPanelOtpSummary(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						char summary_buf[MAX_NUM_VC_PANEL_PORT][48]);

int rtiVC_GetTemperature(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_PANEL_TEMPERATURE_INFO_T tinfo[MAX_NUM_VC_PANEL_PORT]);

int rtiVC_GetVerticalPixelShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port,
							int shift_val[MAX_NUM_VC_PANEL_PORT]);
int rtiVC_GetHorizontalPixelShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port,
							int shift_val[MAX_NUM_VC_PANEL_PORT]);

int rtiVC_SetVerticalPixelShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port, int shift_val);
int rtiVC_SetHorizontalPixelShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port, int shift_val);

void rtiVC_SetBrightness(int brightness);

int rtiVC_DecreaseBrightness(void);
int rtiVC_IncreaseBrightness(void);

int rtiVC_SetLedCurrent(E_VC_PANEL_CTRL_PORT_T ctrl_port,
					VC_LED_CURRENT_INFO_T level[MAX_NUM_VC_PANEL_PORT],
					E_VC_LED_CTRL_COLOR_T color);

int rtiVC_GetLedLevelRange(U8_T *min, U8_T *max);

int rtiVC_GetLedCurrent(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_CURRENT_INFO_T info[MAX_NUM_VC_PANEL_PORT]);

/*=============================================================================
 RDC control APIs
=============================================================================*/
/**
 * @brief Input Display Interface types.
 */
typedef enum {
	VC_INPUT_DISPLAY_IF_LVDS_0 = 0,    /**< LVDS port 0 only */
	VC_INPUT_DISPLAY_IF_LVDS_1 = 1,    /**< LVDS port 1 only */
	VC_INPUT_DISPLAY_IF_LVDS_BOTH = 2, /**< LVDS port 0 and 1 */
	VC_INPUT_DISPLAY_IF_MIPI = 3	   /**< MIPI */
} E_VC_INPUT_DISPLAY_IF_T;

/**
 * @brief LVDS dual mode types.
 */
typedef enum {
	/** Dual pixel mode (port 0 is upper) */
	VC_LVDS_DUAL_MODE_PORT0_EVEN = 0,

	/** Dual pixel mode (port 1 is upper) */
	VC_LVDS_DUAL_MODE_PORT1_EVEN
} E_VC_LVDS_DUAL_MODE_T;

/**
 * @brief LVDS color depth types.
 */
typedef enum {
	VC_LVDS_COLOR_DEPTH_24 = 0,
	VC_LVDS_COLOR_DEPTH_18 = 1
} E_VC_LVDS_COLOR_DEPTH_T;

/**
 * @brief MIPI Data Rate types.
 */
typedef enum {
	VC_MIPI_DR_500Mbps = 0,
	VC_MIPI_DR_700Mbps = 1,
	VC_MIPI_DR_864Mbps = 2,
	VC_MIPI_DR_1000Mbps = 3,
	VC_MIPI_DR_1344Mbps = 4,
	VC_MIPI_DR_1500Mbps = 5
} E_VC_MIPI_DR_T;

/**
 * @brief MIPI number of lane types.
 */
typedef enum {
	VC_MIPI_NUM_LANE_1 = 0,
	VC_MIPI_NUM_LANE_2 = 1,
	VC_MIPI_NUM_LANE_3 = 2,
	VC_MIPI_NUM_LANE_4 = 3,
	MAX_NUM_VC_MIPI_LANE
} E_VC_MIPI_NUM_LANE_T;

/**
 * @brief MIPI Auto power down(PD) types.
 */
typedef enum {
	VC_MIPI_AUTO_PD_pwr_up_inactive = 0,
	VC_MIPI_AUTO_PD_pwr_down_inactive = 1
} E_VC_MIPI_AUTO_PD_T;

/**
 * @brief MIPI Continnous clock mode types.
 */
typedef enum {
	VC_MIPI_CONT_CLK_none = 0, /**< Non-continuous clock mode */
	VC_MIPI_CONT_CLK_cont = 1  /**< Continuous clock mode */
} E_VC_MIPI_CONT_CLK_T;

/**
 * @brief Structure of Input Display Interface configurations.
 */
typedef union {
	struct {
		E_VC_LVDS_DUAL_MODE_T	dual_mode_type;
		E_VC_LVDS_COLOR_DEPTH_T	color_depth;
	} lvds;
	
	struct {
		E_VC_MIPI_DR_T dr; /**< Data rate */
		E_VC_MIPI_NUM_LANE_T num_lane; /**< Number of MIPI lane */
		E_VC_MIPI_AUTO_PD_T auto_pd; /**< Auto power down mode */
		E_VC_MIPI_CONT_CLK_T cont_clk; /**< Continuous clock mode */
	} mipi;
} VC_INPUT_DISPLAY_IF_CFG;

/**
 * @brief Structure of CoKOA parameter.
 */
typedef struct {
	/** Barrel/Pincushion/Mustache parametes */
	double k1;
	double k2;
    
	/** Interpolation parametes */
	double interp;

	/** Keystone parametes */
	S16_T lt_x; /**< Left Top X */
	S16_T rt_x; /**< Right Top X */
	S16_T lb_x; /**< Left Bottom X */
	S16_T rb_x; /**< Right Bottom X */

	S8_T  lt_y; /**< Left Top Y */
	S8_T  rt_y; /**< Right Top Y */
	S8_T  lb_y; /**< Left Bottom Y */
	S8_T  rb_y; /**< Right Bottom Y */

	/** Chromatic aberration parametes */
	U8_T chrm_r;
	U8_T chrm_g;
	U8_T chrm_b;
} VC_COKOA_PARAM_T;

typedef struct {
	unsigned int lut_offset;
		
    int pix_w;
    int pix_h;

	S16_T lt_x; /**< Left Top X */
	S16_T rt_x; /**< Right Top X */
	S16_T lb_x; /**< Left Bottom X */
	S16_T rb_x; /**< Right Bottom X */

	S8_T  lt_y; /**< Left Top Y */
	S8_T  rt_y; /**< Right Top Y */
	S8_T  lb_y; /**< Left Bottom Y */
	S8_T  rb_y; /**< Right Bottom Y */
} VC_COKOA_KEYSTONE_PARAM_T;

/**
 * @brief Structure of extension CoKOA parameter.
 */
typedef struct {
    int lut_id; // CoKOA LUT offset
    
    int pix_w;
    int pix_h;
    
    /** Barrel/Pincushion/Mustache parametes */
    double k1_x;
    double k2_x;
    double k3_x;
    double k1_y;
    double k2_y;
    double k3_y;
    
    /** Side by Side 3D Lossless enable */
    BOOL_T sbys_3d;
    
    /** Traonsformation parametes */
    double rotation;
    double rotation_r;
    
    /** Interpolation parametes */
    double interp_x;
    double interp_y;

    /** Chromatic aberration parametes */
    U8_T chrm_r;
    U8_T chrm_g;
    U8_T chrm_b;
} VC_COKOA_PARAM_EX_T;

#define VC_COKOA_PI 3.14159265358979323846

#if defined(CFG_MDC_RDC200)    
	#define NUM_VC_BMP_DRAWER	4
	#define NUM_VC_TEXT_DRAWER	3

#elif defined(CFG_MDC_RDC200A)
	#define NUM_VC_BMP_DRAWER	3
	#define NUM_VC_TEXT_DRAWER	4

#else
	#error "Code not present"
#endif

/**
 * @brief OSD alpha blending types.
 */
typedef enum {
	/** 0 % */
	VC_OSD_ALPHA_0 = 0,

	/** 12.5 % */
	VC_OSD_ALPHA_12_5 = 1,

	/** 25 % */
	VC_OSD_ALPHA_25 = 2,

	/** 32.5 % */
	VC_OSD_ALPHA_32_5 = 3,

	/** 50 % */
	VC_OSD_ALPHA_50 = 4,

	/** 62.5 % : default */
	VC_OSD_ALPHA_62_5 = 5,

	/** 75 % */
	VC_OSD_ALPHA_75 = 6,

	/** 82.5 % */
	VC_OSD_ALPHA_82_5 = 7,

	/** 100 % */
	VC_OSD_ALPHA_100 = 8,
	VC_OSD_ALPHA_CLEAR_BLACK = 9,
	VC_OSD_ALPHA_CLEAR_WHITE = 10
} E_VC_OSD_ALPHA_T;

/**
 * @brief OSD dimension types.
 */
typedef enum {
	VC_OSD_DIMENSTION_2D = 0,
	VC_OSD_DIMENSTION_3D
} E_VC_OSD_DIMENSTION_T;

/**
 * @brief Bitmap OSD magnification types.
 */
typedef enum {
	VC_BITMAP_OSD_MAG_x1 = 0,
	VC_BITMAP_OSD_MAG_x2 = 1,
	VC_BITMAP_OSD_MAG_x3 = 2,
	VC_BITMAP_OSD_MAG_x4 = 3,
	MAX_NUM_VC_BITMAP_OSD_MAG
} E_VC_OSD_BITMAP_MAG_T;

/**
 * @brief Text OSD magnification types.
 */
typedef enum {
	VC_TEXT_OSD_MAG_x1 = 0,
	VC_TEXT_OSD_MAG_x2 = 1,
	VC_TEXT_OSD_MAG_x3 = 2,
	VC_TEXT_OSD_MAG_x4 = 3,
	VC_TEXT_OSD_MAG_x5 = 4,
	VC_TEXT_OSD_MAG_x6 = 5,
	VC_TEXT_OSD_MAG_x7 = 6,
	VC_TEXT_OSD_MAG_x8 = 7,
	MAX_NUM_VC_TEXT_OSD_MAG
} E_VC_OSD_TEXT_MAG_T;

/**
 * @brief The information of explicit dynamic pixel shift for ATW.
 */
typedef struct {
	/**
	 * X coordinate of starting position to be shift.
	 * RDC200A range: from -4095 to 4095
	 * if shift_x > 0, image start position is to the right of (0, y).
	 * if shift_x < 0, image start position is to the left of (0, y).
	 */
	S16_T					shift_x;

	/**
	 * Y coordinate of starting position to be shift.
	 * RDC200A range: from -8191 to 8191
	 * if shift_y > 0, image start position is above the (x, 0) coordinate.
	 * if shift_y < 0, image start position is below the (x, 0) coordinate.
	 */
	S16_T					shift_y;
} RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T;

#define VC_OSD_FONT_LUT_SUBSET_SIZE	0x100

int rtiVC_GetStatusTextOSD(int handle, BOOL_T *enabled, BOOL_T *showed,
								U32_T *pos_x, U32_T *pos_y,
								U32_T *font_addr,  U32_T *palette_addr,
								E_VC_OSD_TEXT_MAG_T *mag,
								E_VC_OSD_ALPHA_T *font_alpha,
								E_VC_OSD_ALPHA_T *back_alpha,
								U8_T *font_color_idx, U8_T *back_color_idx,
								E_VC_OSD_DIMENSTION_T *d);


int rtiVC_ChangeDimensionTextOSD(int handle, E_VC_OSD_DIMENSTION_T d);

/**
* Change the property of text OSD.
* foreground a
*/

/**
 * @fn rtiVC_ChangePropertyTextOSD
 * @brief Change the property of text OSD.
 * @mag[in] Magnification
 * @font_alpha[in] Foreground alpha blending
 * @back_alpha[in] Background alpha blending
 * @return 0 on success. Otherwise error.
 *
 */
int rtiVC_ChangePropertyTextOSD(int handle, E_VC_OSD_TEXT_MAG_T mag,
								E_VC_OSD_ALPHA_T font_alpha,
								E_VC_OSD_ALPHA_T back_alpha);

/**
* Show the text OSD after being hidden.
*/
int rtiVC_ShowTextOSD(int handle);

/**
* Hide the text OSD.
*/
int rtiVC_HideTextOSD(int handle);

/**
* Move the horizontal and vertical poistion of text OSD on the display.
*/
int rtiVC_MoveTextOSD(int handle, U32_T mov_x, U32_T mov_y);

int rtiVC_DrawTextOSD(int handle, U32_T pos_x, U32_T pos_y,
						U32_T font_addr, U32_T palette_addr,
						E_VC_OSD_TEXT_MAG_T mag,
						E_VC_OSD_ALPHA_T font_alpha,
						E_VC_OSD_ALPHA_T back_alpha,
						U8_T font_color_idx, U8_T back_color_idx,
						E_VC_OSD_DIMENSTION_T d,
						const char *string);

int rtiVC_DestoryTextOsdDrawer(int handle);

int rtiVC_CreateTextOsdDrawer(void);



int rtiVC_GetStatusBitmapOSD(int handle, BOOL_T *enabled, BOOL_T *showed,
								U32_T *pos_x, U32_T *pos_y,
								U32_T *bmp_addr, E_VC_OSD_BITMAP_MAG_T *mag,
								E_VC_OSD_ALPHA_T *alpha,
								E_VC_OSD_DIMENSTION_T *d);

int rtiVC_ChangeDimensionBitmapOSD(int handle, E_VC_OSD_DIMENSTION_T d);

/**
* Change the property of bitmap OSD.
*/
int rtiVC_ChangePropertyBitmapOSD(int handle, E_VC_OSD_BITMAP_MAG_T mag,
									E_VC_OSD_ALPHA_T alpha);

/**
* Hide the bitmap OSD.
*/
int rtiVC_HideBitmapOSD(int handle);

/**
* Show the bitmap OSD after being hidden.
*/
int rtiVC_ShowBitmapOSD(int handle);

/**
* Move the horizontal and vertical poistion of bitmap OSD on the display.
*/
int rtiVC_MoveBitmapOSD(int handle, U32_T mov_x, U32_T mov_y);

/**
* Draw bitmap OSD.
*/
int rtiVC_DrawBitmapOSD(int handle, U32_T pos_x, U32_T pos_y,
						U32_T bmp_addr, E_VC_OSD_BITMAP_MAG_T mag,
						E_VC_OSD_ALPHA_T alpha,
						E_VC_OSD_DIMENSTION_T d);

int rtiVC_DestoryBitmapOsdDrawer(int handle);

int rtiVC_CreateBitmapOsdDrawer(void);

int rtiVC_RequestAtwShiftPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
				RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T info[MAX_NUM_VC_PANEL_PORT]);

int rtiVC_DisableVideoSyncInterrupt(void);

int rtiVC_EnableVideoSyncInterrupt(void);

int rtiVC_EnableGammaCorrection(E_VC_PANEL_CTRL_PORT_T ctrl_port,
								BOOL_T enable,
								UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT]);

int rtiVC_RequestCoKOAGeneration(VC_COKOA_PARAM_T *param);

int rtiVC_GetOutputResolution(U32_T *width, U32_T *height);

int rtiVC_UpdateLUT(U32_T addr, U32_T size, U8_T id);

void rtiVC_AutoGenerateAndDownloadKeystone(void);

int rtiVC_DisableLaserSparklingPwm(void);
int rtiVC_ChangeLaserSparklingPwm(U16_T on_time_period, U16_T off_time_period);

/**
* ca_en: Chromatic aberration
*/
int rtiVC_EnableDistortionCorrection(E_VC_PANEL_CTRL_PORT_T ctrl_port, BOOL_T dc_en,
						UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT], BOOL_T ca_en);

int rtiVC_ConfirmLedControl(E_VC_RDC_INDEX_T rdc_idx);
int rtiVC_requestLedControl(E_VC_RDC_INDEX_T rdc_idx,
							E_VC_PANEL_CTRL_PORT_T ctrl_port,
							VC_LED_CTRL_INFO_T *led_ctrl_info);

int rtiVC_GenerateTestPattern(U8_T r_val, U8_T g_val, U8_T b_val);
int rtiVC_EnableTestPattern(BOOL_T en);

int rtiVC_GetLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
					VC_LED_DUTY_INFO_T info[MAX_NUM_VC_PANEL_PORT]);

int rtiVC_StepDownLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T applied_duty[MAX_NUM_VC_PANEL_PORT],
						E_VC_LED_CTRL_COLOR_T color);

int rtiVC_StepUpLedDuty(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						VC_LED_DUTY_INFO_T applied_duty[MAX_NUM_VC_PANEL_PORT],
						E_VC_LED_CTRL_COLOR_T color);

/*
 * Returns the applied frame rate value if >= 0.
 */
S32_T rtiVC_StepDownFrameRate(void);
S32_T rtiVC_StepUpFrameRate(void);
S32_T rtiVC_GetFrameRate(void);

int rtiVC_SetImageDisplayFormat(E_VC_IMAGE_DISPLAY_FORMAT_T fmt,
								BOOL_T swap_image_3d_mode);

int rtiVC_ShiftVerticalPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
						E_VC_SHIFT_VERTICAL_T v_direction);
int rtiVC_ShiftHorizontalPixel(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							E_VC_SHIFT_HORIZONTAL_T h_direction);

int rtiVC_GetRdcDeviceID(U16_T *chip_id, U16_T *rev_num);

int rtiVC_PowerOnPanel(BOOL_T on);

int rtiVC_GetFirmwareVersion(U8_T *major_ver, U8_T *minor_ver, U8_T *rel_ver);

int rtiVC_ChangeInputDisplayInterface(E_VC_INPUT_DISPLAY_IF_T dif,
										VC_INPUT_DISPLAY_IF_CFG *cfg);

int rtiVC_ChangeInputResolution2(VC_INPUT_RES_T res, BOOL_T enable_3D,
							U32_T pclk_hz);

int rtiVC_ChangeInputResolution(VC_INPUT_RES_T res, BOOL_T enable_3D,
							U32_T h_blank, U32_T v_blank,
							U32_T frame_rate);

/*=============================================================================
 Native FLASH APIs
=============================================================================*/
int rtiVC_ReadFLASH(U32_T addr, U8_T *buf, UINT_T len);
int rtiVC_EraseSectorFLASH(U32_T sector_addr);
int rtiVC_ProgramFLASH(U32_T addr, const U8_T *buf, UINT_T len);
int rtiVC_EraseFlashChip(void);

/*=============================================================================
 Download APIs
=============================================================================*/
void rtiVC_StopDownload(void);

void rtiVC_StartDownload(void);

void rtiVC_RemoveAllSetupItem(void);

int rtiVC_SetupDownloadItem(U32_T pgm_addr, U8_T *pgm_data,
							U16_T pgm_data_size,
							U16_T pgm_unit_size, E_VC_LUT_CLASS_T lut_class);

/**
 * @brief Register callback function to know the status of downloading.
 *
 * @param[in] erase_callback Pointer to erase callback function.
 * @param[in] program_callback Pointer to program callback function.
 * @param[in] verify_callback Pointer to verify callback function.
 * @param[in] proc_callback Pointer to item processing callback function.
 *
 */
void rtiVC_RegisterDownloadCallback(VC_DLOAD_ERASE_CALLBACK_T erase_callback,
									VC_DLOAD_PROGRAM_CALLBACK_T program_callback,
									VC_DLOAD_VERIFY_CALLBACK_T verify_callback,
									VC_DLOAD_PROC_CALLBACK_T proc_callback);

/*=============================================================================
 Device Interface APIs
=============================================================================*/

U8_T rtiVC_ReadDevice16(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr, U16_T reg);
void rtiVC_WriteDevice16(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
						U16_T reg, U8_T val);

int rtiVC_ReadBurstDeviceExt(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
							U8_T *w_buf, U16_T w_size,
							U8_T *r_buf, U16_T r_size);

int rtiVC_WriteBurstDeviceExt(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
								U8_T *buf, U16_T size);

U8_T rtiVC_ReadDevice(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr, U8_T reg);

void rtiVC_WriteDevice(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
						U8_T reg, U8_T val);

void rtiVC_CloseDevice(void);

int rtiVC_OpenDevice(void);

/*=============================================================================
 Startup and terminate APIs
=============================================================================*/
int rtiVC_ResumeController(void);
int rtiVC_SuspendController(void);
int rtiVC_PowerOffController(void);
int rtiVC_PowerOnController(void);

void rtiVC_SetSlaveAddressRDC(U8_T rdc_i2c_slave_addr);
void rtiVC_ChangeI2CBusNumber(E_VC_DEVICE_CH_T ch, int i2c_bus_no);
void rtiVC_Denitialize(void);

int rtiVC_Initialize(U8_T rdc_i2c_slave_addr);

int rtiVC_GetVersion(char* version);

//VC_MUTEX_EXTERN(vc_dev_access_lock);

/**
	Bit mask of attached panel port.
		0x01: Port 0 attached
		0x02: Port 1 attached
		0x03: Port 0 and Port 1 attached
 */
extern UINT_T vc_panel_attached_port_mask;

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_API_H__ */

