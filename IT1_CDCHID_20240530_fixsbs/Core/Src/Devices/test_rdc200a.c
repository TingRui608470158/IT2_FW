
#if !defined(__KERNEL__) && defined(__linux__) /* Linux application */ 
#include <termios.h>  
#endif

#include "rdc_prepare_dload.h"

#ifdef CFG_MDC_RDC200A
#include "test.h"
#include "rdc200a_flash_partitions.h"

/**
 * Directory path for LUT file.
 */
#if defined(ECLIPSE_RCP)
	#define RDC200A_LUT_FILE_PATH		NULL
#elif defined(CFG_CONSOLE_APP) && (!defined(__KERNEL__) && defined(__linux__))
	#define RDC200A_LUT_FILE_PATH		"/data/vclut/rdc200a"
#elif defined(__ANDROID__)
	#define RDC200A_LUT_FILE_PATH		"/sdcard/Movies/vclut/rdc200a"
#elif defined(_WIN32)
	//#define RDC200A_LUT_FILE_PATH		"C:\\home\\vclut\\rdc200a"
	//#define RDC200A_LUT_FILE_PATH		"C:\\home\\vclut\\rdc200a\\RDP550F-F"
	//#define RDC200A_LUT_FILE_PATH		"D:\\vclut\\rdc200a_gen_flash_data"
	//#define RDC200A_LUT_FILE_PATH		"D:\\vclut\\rdc200a\\RDP700Q"

	#define RDC200A_LUT_FILE_PATH		"C:\\vclut\\rdc200a\\RDP370F\\keystone"
#elif defined(__APPLE__)
    #define RDC200A_LUT_FILE_PATH        "."
    //#define RDC200A_LUT_FILE_PATH        "C:\\home\\vclut\\rdc200a"
	//#define RDC200A_LUT_FILE_PATH        "/Users/maverick/Desktop/tmp"

#endif


int rtiVC_GenerateCoKOALuto(VC_COKOA_PARAM_EX_T *param, const char *filename);
int rtiVC_GenerateKeystoneCoKOALut(VC_COKOA_KEYSTONE_PARAM_T *param, const char *filename);

E_VC_PANEL_CTRL_PORT_T test_InputSelection(void);

/*
 The file extensions for download.
*/
static const char *fw_hex_ext[] = {".hex"}; // The list of file extension for firmware hex.
static const char *fw_cfg_ext[] = {".cfg"};
static const char *rdc200a_init_ext[] = {".rega", ".lut", ".txt"};
static const char *panel_init_ext[] = {".rega", ".lut", ".txt"};
static const char *gamma_lut_ext[] = {".lut"}; // for download in directly
static const char *gamma_vt_ext[] = {".vt"}; // for tunning
static const char *cokoa_ext[] = {".lut", ".txt"};
static const char *font_ext[] = {".bmp"};
static const char *palette_ext[] = {".lut", ".txt"};
static const char *bitmap_ext[] = {".bmp", ".dat"};

#if 0
#define GET_RDC200A_DOWNLOAD_FILE_FROM_USER(file_ext_list)\
	get_download_file_from_user(RDC200A_LUT_FILE_PATH,\
								file_ext_list,\
								ARRAY_SIZE(file_ext_list));
#endif


static void make_keystone_lut_file_name(int lut_offset, char *path, char *filename_buf)
{
	sprintf(filename_buf, "%s%ckeystone_%d.lut",
					  path, PathSeparator,
					  lut_offset);
}

static void make_cokoa_file_name(int lut_offset, char *path, char *filename_buf)
{
	struct tm *t;
	time_t timer;

	timer = time(NULL);
	t = localtime(&timer);

	sprintf(filename_buf, "%s%ccokoa_%d_%04d%02d%02d_%02d%02d%02d.lut",
					  path, PathSeparator,
					  lut_offset,
					  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					  t->tm_hour, t->tm_min, t->tm_sec);
}

static int get_double_from_user(double item_buf[], int no_item, char *input_msg)
{
	char read_buf[128], *token;
	int input_item_cnt;

RETRY:
	DMSG("%s", input_msg);
	input_item_cnt = 0;

	fgets(read_buf, 128, stdin);

	token = strtok(read_buf, " \n\r");
	while (token != NULL) {
		if (token[0] == 'q' || token[0] == 'Q')
			return -1000;

		if (token[0] == 'r' || token[0] == 'R')
			return -2000;

		if (is_string_double(token) == FALSE) {
			EMSG("NOT digit value. Retry!\n");
			goto RETRY;
		}

		item_buf[input_item_cnt++] = atof(token);

		token = strtok(NULL, " \n\r");
	}

	if (input_item_cnt != no_item) {
		EMSG("Wrong number of input parameters. Retry!\n");
		goto RETRY;
	}

	return 0;
}

static int get_digit_from_user(int item_buf[], int no_item, char *input_msg)
{
	char read_buf[128], *token;
	int input_item_cnt;

RETRY:
	DMSG("%s", input_msg);
    input_item_cnt = 0;
    
	fgets(read_buf, 128, stdin);

	token = strtok(read_buf, " \n\r");
	while (token != NULL) {
		if (token[0] == 'q' || token[0] == 'Q')
			return -1000;

		if (token[0] == 'r' || token[0] == 'R')
			return -2000;

		if (is_string_digit(token) == FALSE) {
			EMSG("NOT digit value. Retry!\n");
			goto RETRY;
		}

		item_buf[input_item_cnt++] = atoi(token);

		token = strtok(NULL, " \n\r");
	}

	if (input_item_cnt != no_item) {
		EMSG("Wrong number of input parameters. Retry!\n");
		goto RETRY;
	}

	return 0;
}

static int input_cokoa_param_from_user(VC_COKOA_PARAM_EX_T *param, BOOL_T SBYS_3D, int key)
{
//	int ret, key;
	int ret;
	int digitb[3];
	double doubleb[6];

RETYPE_COKOA:
	DMSG("Input CoKOA parameters\n");

#if 1
RETRY_RESOLUTION:
	ret = get_digit_from_user(digitb, 2, "[Width(1 ~ 2560) Height(1 ~ 1080)], q(Quit):\n");
	if (ret == 0) {
		if ((digitb[0] < 1) || (digitb[0] > 2560)) {
			EMSG("Width: Out of range. Retry!\n");
			goto RETRY_RESOLUTION;
		}

		if ((digitb[1] < 1) || (digitb[1] > 1080)) {
			EMSG("Height: Out of range. Retry!\n");
			goto RETRY_RESOLUTION;
		}
		param->pix_w = (U16_T)digitb[0];
		param->pix_h = (U16_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;

RETRY_PINCUSHION: /* Pincushion/barrel */
	ret = get_double_from_user(doubleb, 6, "Pincushion/barrel (-0.500 ~ 0.500): [k1_x k2_x k3_x k1_y k2_y k3_y], q(Quit), r(Retype):\n");
	if (ret == 0) {
        if ((doubleb[0] < -0.500) || (doubleb[0] > 0.500) || (doubleb[3] < -0.500) || (doubleb[3] > 0.500)) {
			EMSG("k1: Out of range. Retry!\n");
			goto RETRY_PINCUSHION;
		}
		if ((doubleb[1] < -0.500) || (doubleb[1] > 0.500) || (doubleb[4] < -0.500) || (doubleb[4] > 0.500)) {
			EMSG("k2: Out of range. Retry!\n");
			goto RETRY_PINCUSHION;
		}
		if ((doubleb[2] < -0.500) || (doubleb[2] > 0.500) || (doubleb[5] < -0.500) || (doubleb[5] > 0.500)) {
			EMSG("k3: Out of range. Retry!\n");
			goto RETRY_PINCUSHION;
		}
		param->k1_x = doubleb[0];
		param->k2_x = doubleb[1];
		param->k3_x = doubleb[2];
        param->k1_y = doubleb[3];
        param->k2_y = doubleb[4];
        param->k3_y = doubleb[5];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;

RETRY_CHROM: /* Chromatic Aberration */
	ret = get_digit_from_user(digitb, 3, "Chromatic Aberration (0 ~ 100)%: [R G B], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 100)) {
			EMSG("R: Out of range. Retry!\n");
			goto RETRY_CHROM;
		}

		if ((digitb[1] < 0) || (digitb[1] > 100)) {
			EMSG("G: Out of range. Retry!\n");
			goto RETRY_CHROM;
		}
		if ((digitb[2] < 0) || (digitb[2] > 100)) {
			EMSG("B: Out of range. Retry!\n");
			goto RETRY_CHROM;
		}
		param->chrm_r = (U8_T)digitb[0];
		param->chrm_g = (U8_T)digitb[1];
		param->chrm_b = (U8_T)digitb[2];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;
    
RETRY_ROTATION: /* Rotation */
    if(SBYS_3D)
    {
        ret = get_double_from_user(doubleb, 2, "Rotation (~3.00 ~ 3.00): [degree_left degree_right], q(Quit), r(Retype):\n");
        if (ret == 0) {
            if ((doubleb[0] < -3.000) || (doubleb[0] > 3.000) || (doubleb[1] < -3.000) || (doubleb[1] > 3.000)) {
                EMSG("Rotation: Out of range. Retry!\n");
                goto RETRY_ROTATION;
            }
            param->rotation = doubleb[0];
            param->rotation_r = doubleb[1];
        }
        else if (ret == -1000)
            return ret;
        else if (ret == -2000)
            goto RETYPE_COKOA;
    }
    else
    {
        ret = get_double_from_user(doubleb, 1, "Rotation (~3.00 ~ 3.00): [degree], q(Quit), r(Retype):\n");
        if (ret == 0) {
            if ((doubleb[0] < -3.000) || (doubleb[0] > 3.000)) {
                EMSG("Rotation: Out of range. Retry!\n");
                goto RETRY_ROTATION;
            }
            param->rotation = doubleb[0];
        }
        else if (ret == -1000)
            return ret;
        else if (ret == -2000)
            goto RETYPE_COKOA;
    }
RETRY_INTERPOL: /* Interpolation */
	ret = get_double_from_user(doubleb, 2, "Interpolation (0.500 ~ 1.500): [interp_x inter_y], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((doubleb[0] < 0.500) || (doubleb[0] > 1.500) || (doubleb[1] < 0.500) || (doubleb[1] > 1.500)) {
			EMSG("Interpolation: Out of range. Retry!\n");
			goto RETRY_INTERPOL;
		}
        
		param->interp_x = doubleb[0];
        param->interp_y = doubleb[1];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;
#endif

	DMSG("------------------- Inputted parametes ---------------------------\n");
	DMSG("Resolution: width(%d) height(%d)\n", param->pix_w, param->pix_h);
	DMSG("Pincushion/barrel: k1_x(%.3lf) k2_x(%.3lf) k3_x(%.3lf) k1_y(%.3lf) k2_y(%.3lf) k3_y(%.3lf)\n",
         param->k1_x, param->k2_x, param->k3_x, param->k1_y, param->k2_y, param->k3_y);
	DMSG("Chromatic Aberration: R(%d%%) G(%d%%) B(%d%%)\n", param->chrm_r, param->chrm_g, param->chrm_b);
    if (SBYS_3D) {
        DMSG("Rotation: left(%.2lf) deg right(%.2lf) deg\n", param->rotation, param->rotation_r);
    } else {
        DMSG("Rotation: %.2lf deg\n", param->rotation);
	}
    DMSG("Interpolation: interp_x(%.3lf) interp_y(%.3lf)\n", param->interp_x, param->interp_y);
	
	while (1) {
		DMSG("Use this inputted parameters(y) or retype(n)?"); 

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		if (key == 'y' || key == 'Y')
			break;

		if (key == 'n' || key == 'N')
			goto RETYPE_COKOA;
	}

	return 0;
}

#if defined(CFG_MDC_RDC200)
	#define FLASH_ADDR_PALETTE_COLOR_SET_0		0x088000
	#define FLASH_ADDR_PALETTE_COLOR_SET_1		0x089000
	#define FLASH_ADDR_FONT						0x078000
#elif  defined(CFG_MDC_RDC200A)
	#define FLASH_ADDR_PALETTE_COLOR_SET_0		0x0DE000
	#define FLASH_ADDR_PALETTE_COLOR_SET_1		(0x0DE000 + 0x1000)
	#define FLASH_ADDR_FONT						0x0CE000
#else
	#error "Code not present"
#endif

static void test_FontOSD(void)
{
	int handle;
	U32_T pos_x, pos_y;
	U32_T font_addr, palette_addr;
	E_VC_OSD_TEXT_MAG_T mag;
	E_VC_OSD_ALPHA_T font_alpha;
	E_VC_OSD_ALPHA_T back_alpha;
	U8_T font_color_idx, back_color_idx;
	E_VC_OSD_DIMENSTION_T d;
	const char *string;

	handle = rtiVC_CreateTextOsdDrawer();
	if (handle < 0) {
		EMSG("Can't create a text OSD drawer\n");
		return;
	}

#if 0
	// To test OSD, Test pattern or HMDI input must enabled.
	rtiVC_EnableTestPattern(TRUE);
	rtiVC_GenerateTestPattern(0xFF, 0xFF, 0xFF);
#endif

	// Input 
	pos_x = 1;
	pos_y = 500;
	font_addr = FLASH_ADDR_FONT;
	//palette_addr = FLASH_ADDR_PALETTE_COLOR_SET_0 + (1/*lut offset*/ * VC_OSD_FONT_LUT_SUBSET_SIZE);
	palette_addr = FLASH_ADDR_PALETTE_COLOR_SET_0;
	mag = VC_TEXT_OSD_MAG_x1;
	font_alpha = VC_OSD_ALPHA_100;
	back_alpha = VC_OSD_ALPHA_100;
	font_color_idx = 1; /* from 0 to 15 */
	back_color_idx = 9;
	d = VC_OSD_DIMENSTION_2D;
	string = "Text OSD";

	rtiVC_DrawTextOSD(handle, pos_x, pos_y, font_addr, palette_addr,
					mag, font_alpha, back_alpha,
					font_color_idx, back_color_idx, d, string);

	rtiVC_DrawTextOSD(handle, pos_x, pos_y, font_addr, palette_addr,
					mag, font_alpha, back_alpha,
					font_color_idx, back_color_idx, d, "UUU");


	rtiVC_DrawTextOSD(handle, pos_x, pos_y, font_addr, palette_addr,
					mag, font_alpha, back_alpha,
					font_color_idx, back_color_idx, d, "ZZZZZZZ");

#if 0 ////


#if 0
	rtiVC_ChangeDimensionTextOSD(handle, VC_OSD_DIMENSTION_3D);

	rtiVC_ChangeDimensionTextOSD(handle, VC_OSD_DIMENSTION_2D);
#endif

	rtiVC_ChangePropertyTextOSD(handle, VC_TEXT_OSD_MAG_x3, VC_OSD_ALPHA_25, VC_OSD_ALPHA_25);

	rtiVC_ChangePropertyTextOSD(handle, VC_TEXT_OSD_MAG_x2, VC_OSD_ALPHA_100, VC_OSD_ALPHA_100);

	rtiVC_HideTextOSD(handle);

	rtiVC_ShowTextOSD(handle);

	rtiVC_MoveTextOSD(handle, 200, 200);

	rtiVC_DrawTextOSD(handle, 500, 500, FLASH_ADDR_FONT, FLASH_ADDR_PALETTE_COLOR_SET_0,
					//VC_TEXT_OSD_MAG_x1, VC_OSD_ALPHA_62_5, VC_OSD_ALPHA_62_5,
					VC_TEXT_OSD_MAG_x4, VC_OSD_ALPHA_100, VC_OSD_ALPHA_100,
					4, 11, VC_OSD_DIMENSTION_2D, "abcd");


#endif ///

	rtiVC_DestoryTextOsdDrawer(handle);

	//	rtiVC_EnableTestPattern(FALSE);
}

static int input_change_property_bitmap_osd_param(E_VC_OSD_BITMAP_MAG_T *mag,
									E_VC_OSD_ALPHA_T *alpha,
									E_VC_OSD_DIMENSTION_T *dimension, int key)
{
//	int ret, key;
	int ret;
	int digitb[2];
	static const char *mag_str[] = {"x1", "x2", "x3", "x4"}; 
	static const char *alpha_str[] = {
		"0%", "12.5%", "25%", "32.5%", "50%", "62.5%", "75%", "82.5%",
		"100%", "Clear black", "Clear white"
	}; 
	static const char *dimension_str[] = {"2D", "3D"};

RETYPE_DRAW_PARAM:
	DMSG("Input change property bitmap OSD parameters\n");

ccc:
	DMSG("Magnification\n");
	DMSG("0: x1\n");
	DMSG("1: x2\n");
	DMSG("2: x3\n");
	DMSG("3: x4\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 3)) {
			EMSG("Out of range. Retry!\n");
			goto ccc;
		}
		*mag = (E_VC_OSD_BITMAP_MAG_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto ccc;

ddd:
	DMSG("Alpha blending\n");
	DMSG("0: 0%%\n");
	DMSG("1: 12.5%%\n");
	DMSG("2: 25%%\n");
	DMSG("3: 32.5%%\n");
	DMSG("4: 50%%\n");
	DMSG("5: 62.5%%\n");
	DMSG("6: 75%%\n");
	DMSG("7: 82.5%%\n");
	DMSG("8: 100%%\n");
	DMSG("9: Clear black\n");
	DMSG("10: Clear white\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 10)) {
			EMSG("Out of range. Retry!\n");
			goto ddd;
		}
		*alpha = (E_VC_OSD_ALPHA_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto ddd;

eee:
	DMSG("Dimension\n");
	DMSG("0: 2D\n");
	DMSG("1: 3D\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 1)) {
			EMSG("Out of range. Retry!\n");
			goto eee;
		}
		*dimension = (E_VC_OSD_DIMENSTION_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_DRAW_PARAM;

	DMSG("------------------- Inputted parametes ---------------------------\n");
	DMSG("Magnification: %s\n", mag_str[*mag]);
	DMSG("Alpha blending: %s\n", alpha_str[*alpha]);
	DMSG("Dimension: %s\n", dimension_str[*dimension]);

	while (1) {
		DMSG("Use this inputted parameters(y) or retype(n)?"); 

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		if (key == 'y' || key == 'Y')
			break;

		if (key == 'n' || key == 'N')
			goto RETYPE_DRAW_PARAM;
	}

	return 0;
}

static int input_draw_bitmap_osd_param(U32_T *pos_x, U32_T *pos_y,
									U32_T *bmp_addr, E_VC_OSD_BITMAP_MAG_T *mag,
									E_VC_OSD_ALPHA_T *alpha,
									E_VC_OSD_DIMENSTION_T *dimension, int key)
{
//	int ret, i, key, bitmap_idx = 0;
	int ret, i, bitmap_idx = 0;
	int digitb[2];
	U32_T width, height;
	U32_T flash_addr;
	static const char *mag_str[] = {"x1", "x2", "x3", "x4"}; 
	static const char *alpha_str[] = {
		"0%", "12.5%", "25%", "32.5%", "50%", "62.5%", "75%", "82.5%",
		"100%", "Clear black", "Clear white"
	}; 
	static const char *dimension_str[] = {"2D", "3D"};

RETYPE_DRAW_PARAM:
	DMSG("Input bitmap OSD parameters\n");

	rtiVC_GetOutputResolution(&width, &height);
	DMSG("Output Resolution: width(%u) x height(%u)\n", width, height);

aaa:
	ret = get_digit_from_user(digitb, 2, "Left Top position [x  y], q(Quit):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || ((U32_T)digitb[0] > width)) {
			EMSG("Width: Out of range. Retry!\n");
			goto aaa;
		}

		if ((digitb[1] < 0) || ((U32_T)digitb[1] > height)) {
			EMSG("Height: Out of range. Retry!\n");
			goto aaa;
		}
		*pos_x = (U32_T)digitb[0];
		*pos_y = (U32_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;

bbb:
	ret = get_digit_from_user(digitb, 1, "Bitmap index in the FLASH (0 ~ 15): [ index], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 15)) {
			EMSG("Index: Out of range. Retry!\n");
			goto bbb;
		}

		bitmap_idx = digitb[0];

		flash_addr = FLASH_PART_OFFSET_BMP;
		for (i = 0; i < digitb[0]; i++)
			flash_addr += FLASH_ITEM_SIZE_BMP; // 1 image partition boundary

		*bmp_addr = flash_addr;
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_DRAW_PARAM;

ccc:
	DMSG("Magnification\n");
	DMSG("0: x1\n");
	DMSG("1: x2\n");
	DMSG("2: x3\n");
	DMSG("3: x4\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 3)) {
			EMSG("Out of range. Retry!\n");
			goto ccc;
		}
		*mag = (E_VC_OSD_BITMAP_MAG_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto ccc;

ddd:
	DMSG("Alpha blending\n");
	DMSG("0: 0%%\n");
	DMSG("1: 12.5%%\n");
	DMSG("2: 25%%\n");
	DMSG("3: 32.5%%\n");
	DMSG("4: 50%%\n");
	DMSG("5: 62.5%%\n");
	DMSG("6: 75%%\n");
	DMSG("7: 82.5%%\n");
	DMSG("8: 100%%\n");
	DMSG("9: Clear black\n");
	DMSG("10: Clear white\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 10)) {
			EMSG("Out of range. Retry!\n");
			goto ddd;
		}
		*alpha = (E_VC_OSD_ALPHA_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto ddd;

eee:
	DMSG("Dimension\n");
	DMSG("0: 2D\n");
	DMSG("1: 3D\n");
	ret = get_digit_from_user(digitb, 1, "q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < 0) || (digitb[0] > 1)) {
			EMSG("Out of range. Retry!\n");
			goto eee;
		}
		*dimension = (E_VC_OSD_DIMENSTION_T)digitb[0];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_DRAW_PARAM;

	DMSG("------------------- Inputted parametes ---------------------------\n");
	DMSG("Position: x(%d) y(%d)\n", *pos_x, *pos_y);
	DMSG("FLASH : Index(%d), Address(0x%06X)\n", bitmap_idx, *bmp_addr);
	DMSG("Magnification: %s\n", mag_str[*mag]);
	DMSG("Alpha blending: %s\n", alpha_str[*alpha]);
	DMSG("Dimension: %s\n", dimension_str[*dimension]);

	while (1) {
		DMSG("Use this inputted parameters(y) or retype(n)?"); 

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		if (key == 'y' || key == 'Y')
			break;

		if (key == 'n' || key == 'N')
			goto RETYPE_DRAW_PARAM;
	}

	return 0;
}

static void test_BitmapOSD(int key)
{
//	int key, ret;
	int ret;
	int handle;
	U32_T pos_x, pos_y;
	U32_T bmp_addr;
	E_VC_OSD_BITMAP_MAG_T mag;
	E_VC_OSD_ALPHA_T alpha;
	E_VC_OSD_DIMENSTION_T dimension;
	int digitb[2];
	U32_T width, height;

	handle = rtiVC_CreateBitmapOsdDrawer();
	if (handle < 0) {
		EMSG("Can't create a bitmap OSD drawer\n");
		return;
	}

#if 0
	// To test OSD, Test pattern or HMDI input must enabled.
	rtiVC_EnableTestPattern(TRUE);
	rtiVC_GenerateTestPattern(0x00, 0x00, 0xFF);
#endif	

#if 1
{
	rtiVC_DrawBitmapOSD(handle, 100, 100, FLASH_ITEM_ADDR_BMP(1), VC_BITMAP_OSD_MAG_x1,
						VC_OSD_ALPHA_100, VC_OSD_DIMENSTION_2D);

	rtiVC_HideBitmapOSD(handle);

	rtiVC_ShowBitmapOSD(handle);

	rtiVC_ChangeDimensionBitmapOSD(handle, VC_OSD_DIMENSTION_3D);

	rtiVC_MoveBitmapOSD(handle, 200, 200);

	rtiVC_ChangePropertyBitmapOSD(handle, VC_BITMAP_OSD_MAG_x2, VC_OSD_ALPHA_32_5);

	rtiVC_ChangePropertyBitmapOSD(handle, VC_BITMAP_OSD_MAG_x1, VC_OSD_ALPHA_100);

	rtiVC_ChangeDimensionBitmapOSD(handle, VC_OSD_DIMENSTION_2D);

	rtiVC_ChangeDimensionBitmapOSD(handle, VC_OSD_DIMENSTION_3D);
}
#endif

	while(1) {
		DMSG("================ Bitmap OSD test ===============\n");
		DMSG("\t0: Draw\n");
		DMSG("\t1: Move\n");
		DMSG("\t2: Change Property\n");
		DMSG("\t3: Hide\n");
		DMSG("\t4: Show\n");
		DMSG("\t5: Status\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch(key)
		{
		case '0':
			DMSG("Draw\n");
			ret = input_draw_bitmap_osd_param(&pos_x, &pos_y, &bmp_addr, &mag,
											&alpha,	&dimension, key);
			if (ret == 0) {
				rtiVC_DrawBitmapOSD(handle, pos_x, pos_y, bmp_addr, mag,
									alpha, dimension);
			}
			break;

		case '1':
			DMSG("Move\n");
			while (1)
			{
				rtiVC_GetOutputResolution(&width, &height);
				DMSG("Output Resolution: width(%u) x height(%u)\n", width, height);
			
				ret = get_digit_from_user(digitb, 2, "Left Top position [x	y], q(Quit):\n");
				if (ret == 0) {
					if ((digitb[0] < 0) || ((U32_T)digitb[0] > width)) {
						EMSG("Width: Out of range. Retry!\n");
						continue;
					}
				
					if ((digitb[1] < 0) || ((U32_T)digitb[1] > height)) {
						EMSG("Height: Out of range. Retry!\n");
						continue;
					}
					pos_x = (U32_T)digitb[0];
					pos_y = (U32_T)digitb[1];

					rtiVC_MoveBitmapOSD(handle, pos_x, pos_y);
				}
				else if (ret == -1000)
					break;
			}
			break;

		case '2':
			DMSG("Change Property\n");
			ret = input_change_property_bitmap_osd_param(&mag, &alpha, &dimension, key);
			if (ret == 0)
				rtiVC_ChangePropertyBitmapOSD(handle, mag, alpha);
			break;

		case '3':
			DMSG("Hide\n");
			rtiVC_HideBitmapOSD(handle);
			break;

		case '4':
			DMSG("Show");
			rtiVC_ShowBitmapOSD(handle);
			break;

		case '5':
			break;

		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
	}

TEST_TP_EXIT:
	rtiVC_DestoryBitmapOsdDrawer(handle);

	rtiVC_EnableTestPattern(FALSE);

	return;
}

void test_LaserSparklingPwm(void)
{
	// input on_time_period, off_time_period
	//rtiVC_ChangeLaserSparklingPwm(1234, 5678);
	rtiVC_ChangeLaserSparklingPwm(1000, 1000);

	rtiVC_ChangeLaserSparklingPwm(1000, 1000);

	rtiVC_DisableLaserSparklingPwm();

	rtiVC_ChangeLaserSparklingPwm(2000, 2000);

	rtiVC_ChangeLaserSparklingPwm(24000, 25000);

	rtiVC_ChangeLaserSparklingPwm(60000, 55000);
}

/* example keysteone batch download function */
#define ROW_NUM (16+1)
#define COL_NUM (8+1)
//#define X_MAX (128) // 256
#define X_MAX (256)
#define Y_MAX (64)

void rtiVC_AutoGenerateAndDownloadKeystone(void)
{
#if 0
	int ret;
	int Left_Top_X = 0;
	int Left_Top_Y = 0;
	int Right_Top_X = 0;
	int Right_Top_Y = 0;
	int Left_Bottom_X = 0;
	int Left_Bottom_Y = 0;
	int Right_Bottom_X = 0;
	int Right_Bottom_Y = 0;
	char filename_buf[256];
	VC_COKOA_KEYSTONE_PARAM_T param;
	int flash_id = 0;
	int row, col;
	int xstep = (int)((float)X_MAX*2/(float)(ROW_NUM-1));
	int ystep = (int)((float)Y_MAX*2/(float)(COL_NUM-1));	

	for(row = 0; row < ROW_NUM; row++)
	{
		int x = row*xstep - X_MAX;
		if(x < 0) 
		{
			//Left_Bottom_X = Right_Bottom_X = -x;
			Left_Bottom_X = Right_Bottom_X = MIN(-x, 255);
			Left_Top_X = Right_Top_X = 0;
		} else {
			Left_Bottom_X = Right_Bottom_X = 0;
			Left_Top_X = Right_Top_X = x;
		}
		
		for(col = 0; col < COL_NUM; col++) 
		{
			int y = col*ystep - Y_MAX;
			if(y < 0)
			{
				Right_Top_Y = Right_Bottom_Y = MIN(-y, 63);
				Left_Top_Y = Left_Bottom_Y = 0;
			} else {
				Right_Top_Y = Right_Bottom_Y = 0;
				Left_Top_Y = Left_Bottom_Y = MIN(y, 63);
			}

			param.lut_offset = flash_id;
			param.pix_w = 1920;
		    param.pix_h = 1080;
			param.lt_x = Left_Top_X; /**< Left Top X */
			param.lt_y = Left_Top_Y; /**< Left Top Y */
			param.rt_x = Right_Top_X; /**< Right Top X */
			param.rt_y = Right_Top_Y; /**< Right Top Y */
			param.lb_x = Left_Bottom_X; /**< Left Bottom X */
			param.lb_y = Left_Bottom_Y; /**< Left Bottom Y */
			param.rb_x = Right_Bottom_X; /**< Right Bottom X */	
			param.rb_y = Right_Bottom_Y; /**< Right Bottom Y */

			DMSG("Generate Keystone %d\n", flash_id);
			make_keystone_lut_file_name(flash_id, RDC200A_LUT_FILE_PATH, filename_buf);
			ret = rtiVC_GenerateKeystoneCoKOALut(&param, filename_buf);
		    if (ret == 0)
			{
#if 1
		        ret = prepare_optic_dc_lut(filename_buf);
		        if (ret == 0) {
					DMSG("Download Keystone %d\n", flash_id);
		            rtiVC_StartDownload(); // API call.
				}
#endif
			}			

			flash_id = flash_id + 1;
		}
	}

	printf("flash_id(%d)\n", flash_id);
#endif
}

void test_SelectKeystone(int key, int lut_id)
{
//	int key;
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];
	UINT_T ctrl_port = 0x0;
//	int lut_id;

    ctrl_port = test_InputSelection();
    if(ctrl_port == VC_PANEL_CTRL_INVAILD)
        return ;
    
    if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
        lut_offset[0] = 0;
    }
    if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
        lut_offset[1] = 0;
    }
    
	while(1) {
		DMSG("================ Select Keystone test ===============\n");
		DMSG("\t0: Select LUT\n");
        DMSG("\t1: Enable CoKOA\n");
        DMSG("\t2: Disable CoKOA\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");
		
#if 0
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
#endif
       
		switch (key) {
		case '0':
			DMSG("Select LUT\n");

RETRY_INPUT_LUT_OFFSET:
#if 0
			DMSG("Input LUT offset(Decimal) : ");
			scanf("%d", &lut_id);
			CLEAR_STDIN;
#endif
			if ((lut_id >= 0) && (lut_id <= 163)) {
				lut_offset[0] = lut_offset[1] = lut_id;
			}
			else {
				EMSG("Invalid LUT offset (0 ~ 163)\n");
				goto RETRY_INPUT_LUT_OFFSET;
			}
			break;

		case '1':
            DMSG("Keystone Enabled\n");
            rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);
			break;

		case '2':
            DMSG("Keystone Disabled\n");
            rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset, TRUE);
			break;

		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;
		default:
			DMSG("[%c]\n", key);
		}   
	}

TEST_TP_EXIT:

	return;
}


void test_AutoGenKeystoneCoKOA(int key, int lut_id)
{
//	int key;
	VC_COKOA_KEYSTONE_PARAM_T param;
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];
//	int lut_id, i;
	int i;

#if defined(_CFG_RDP_ATTACHED_TO_PORT_0) && !defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	UINT_T ctrl_port = 0x1;
#elif !defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	UINT_T ctrl_port = 0x2;
#elif defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	UINT_T ctrl_port = 0x3;
#else
	#error "Code not present"
#endif	

	while(1) {
		DMSG("================ Keystone Test ===============\n");
		DMSG("\t0: Select LUT\n");
		DMSG("\t1: Slide Show\n");
		DMSG("\tg: Generate LUT and Download\n");		
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");
		
#if 0
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
#endif

		switch(key) {
		case '0':
			DMSG("Select LUT\n");

RETRY_INPUT_LUT_OFFSET:
#if 0
			DMSG("Input LUT offset(Decimal) : ");
			scanf("%d", &lut_id);
			CLEAR_STDIN;
#endif
			if ((lut_id >= 0) && (lut_id <= 163)) {
				lut_offset[0] = lut_offset[1] = lut_id;
				rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port,
												//TRUE, lut_offset, FALSE);
												TRUE, lut_offset, TRUE);
			}
			else {
				EMSG("Invalid LUT offset (0 ~ 163)\n");
				goto RETRY_INPUT_LUT_OFFSET;
			}
			break;

		case '1':
			DMSG("Slide Show\n");

RETRY_INPUT_SLIDE_SHOW:
#if 0
			DMSG("Input the last LUT offset(Decimal) : ");			
			scanf("%d", &lut_id);
			CLEAR_STDIN;
#endif
			if ((lut_id <= 163)) {
				for (i = 0; i <= lut_id; i++) {				
					lut_offset[0] = lut_offset[1] = i;
					DMSG("Keystone %d\n", i);
					rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port,
													TRUE, lut_offset, TRUE);
					VC_M_DELAY(1000);
				}
			}
			else {
				EMSG("Invalid LUT offset (0 ~ 163)\n");
				goto RETRY_INPUT_SLIDE_SHOW;
			}
			break;

		case 'g':
			DMSG("Generate LUT and Download\n");
			rtiVC_AutoGenerateAndDownloadKeystone();
			break;

		case 'q':
		case 'Q':
			goto KEYSTONE_TEST_EXIT;
		default:
			DMSG("[%c]\n", key);
		}
	}

KEYSTONE_TEST_EXIT:
	return;
}

void test_KeystoneCoKOA(void)
{
#if 0
	int ret;
	UINT_T ctrl_port = 0x3;
	char filename_buf[512];
	VC_COKOA_KEYSTONE_PARAM_T param;
	unsigned int cokoa_lut_offset;
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];
	
	cokoa_lut_offset = 1; // test
	
	param.lut_offset = cokoa_lut_offset;
	param.pix_w = 1920;
    param.pix_h = 1080;
	param.lt_x = 100; /**< Left Top X */
	param.lt_y = 0; /**< Left Top Y */
	param.rt_x = 0; /**< Right Top X */
	param.rt_y = 0; /**< Right Top Y */
	param.lb_x = 0; /**< Left Bottom X */
	param.lb_y = 0; /**< Left Bottom Y */
	param.rb_x = 100; /**< Right Bottom X */	
	param.rb_y = 0; /**< Right Bottom Y */
	
	make_keystone_lut_file_name(cokoa_lut_offset, RDC200A_LUT_FILE_PATH, filename_buf);
	ret = rtiVC_GenerateKeystoneCoKOALut(&param, filename_buf);
    if (ret == 0)
	{
        ret = prepare_optic_dc_lut(filename_buf);
        if (ret == 0)
            rtiVC_StartDownload(); // API call.

		lut_offset[0] = lut_offset[1] = cokoa_lut_offset;
		rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);

		rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset, TRUE);

		rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);
	}
#endif
}

void test_CoKOA(void)
{
#if 0
	int key, ret;
	char filename_buf[256];
    VC_COKOA_PARAM_EX_T param;
    BOOL_T sbys_3d = FALSE;
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];
	UINT_T ctrl_port = 0x0;

    ctrl_port = test_InputSelection();
    if(ctrl_port == VC_PANEL_CTRL_INVAILD)
        return ;
    
    if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
        lut_offset[0] = 0;
    }
    if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
        lut_offset[1] = 0;
    }
    
	while(1) {
		DMSG("================ CoKOA test ===============\n");
		DMSG("\t0: Create CoKOA file\n");
		DMSG("\t1: Create CoKOA file and download\n");
        DMSG("\t2: Enable CoKOA\n");
        DMSG("\t3: Disable CoKOA\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");
		
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
       
        if(key == '2')
        {
            DMSG("CoKOA Enabled\n");
            rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);
            continue;
        }
        else if(key == '3')
        {
            DMSG("CoKOA Disabled\n");
            rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset, TRUE);
            continue;
        }
        else if(key == '0' || key == '1')
        {
            sbys_3d = FALSE;
        }
        else if(key == 'q' || key == 'Q')
        {
            goto TEST_TP_EXIT;
        }
        else
        {
            DMSG("[%c]\n", key);
            continue;
        }
        
        input_cokoa_param_from_user(&param, sbys_3d);
        param.lut_id = 0;
        
        make_cokoa_file_name(param.lut_id, RDC200A_LUT_FILE_PATH, filename_buf);
        ret = rtiVC_GenerateCoKOALuto(&param, filename_buf);
        if (ret == 0)
        {
            DMSG("CoKOA file created: %s\n", filename_buf);
            if(key == '1' || key == '5')
            {
                ret = prepare_optic_dc_lut(filename_buf);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
        }
        else
        {
            DMSG("Can't create: %s (%d)\n", filename_buf, ret);
        }
	}

TEST_TP_EXIT:

	return;
#endif
}

static int input_cokoa_keystone_from_user(S16_T *lt_x, S8_T *lt_y,
										S16_T *rt_x, S8_T *rt_y,
										S16_T *lb_x, S8_T *lb_y,
										S16_T *rb_x, S8_T *rb_y, int key)
{
//	int ret, key;
	int ret;
	int digitb[4];

RETYPE_COKOA:
	DMSG("Input CoKOA keystone\n");

#if 1
RETRY_KEYSTONE_LT: /* Left Top */
	ret = get_digit_from_user(digitb, 2, "Left top x(-256 ~ 255) y(-64 ~ 63): [x y], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < -256) || (digitb[0] > 255)) {
			EMSG("x: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_LT;
		}

		if ((digitb[1] < -64) || (digitb[1] > 63)) {
			EMSG("y: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_LT;
		}
		*lt_x = (S16_T)digitb[0];
		*lt_y = (S8_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;

RETRY_KEYSTONE_RT: /* RIGHT Top */
	ret = get_digit_from_user(digitb, 2, "Right top x(-256 ~ 255) y(-64 ~ 63): [x y], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < -256) || (digitb[0] > 255)) {
			EMSG("x: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_RT;
		}

		if ((digitb[1] < -64) || (digitb[1] > 63)) {
			EMSG("y: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_RT;
		}
		*rt_x = (S16_T)digitb[0];
		*rt_y = (S8_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;

RETRY_KEYSTONE_LB: /* Left Bottom */
	ret = get_digit_from_user(digitb, 2, "Left Bottom x(-256 ~ 255) y(-64 ~ 63): [x y], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < -256) || (digitb[0] > 255)) {
			EMSG("x: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_LB;
		}

		if ((digitb[1] < -64) || (digitb[1] > 63)) {
			EMSG("y: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_LB;
		}
		*lb_x = (S16_T)digitb[0];
		*lb_y = (S8_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;

RETRY_KEYSTONE_RB: /* Right Bottom */
	ret = get_digit_from_user(digitb, 2, "Right Bottom x(-256 ~ 255) y(-64 ~ 63): [x y], q(Quit), r(Retype):\n");
	if (ret == 0) {
		if ((digitb[0] < -256) || (digitb[0] > 255)) {
			EMSG("x: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_RB;
		}

		if ((digitb[1] < -64) || (digitb[1] > 63)) {
			EMSG("y: Out of range. Retry!\n");
			goto RETRY_KEYSTONE_RB;
		}
		*rb_x = (S16_T)digitb[0];
		*rb_y = (S8_T)digitb[1];
	}
	else if (ret == -1000)
		return ret;
	else if (ret == -2000)
		goto RETYPE_COKOA;
#endif

#if 0 // test input
	*lt_x = 0; *lt_y = 0;
	*rt_x = 255; *rt_y = 0;
	*lb_x = 0; *lb_y = 63;
	*rb_x = 255; *rb_y = 63;
#endif

	DMSG("------------------- Inputted parametes ---------------------------\n");
	DMSG("Keystone :\n");
	DMSG("\t LT(%d, %d) \t RT(%d, %d)\n", *lt_x, *lt_y, *rt_x, *rt_y);
	DMSG("\t LB(%d, %d) \t RB(%d, %d)\n", *lb_x, *lb_y, *rb_x, *rb_y);
	
	while (1) {
		DMSG("Use this inputted parameters(y) or retype(n)?"); 

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		if (key == 'y' || key == 'Y')
			break;

		if (key == 'n' || key == 'N')
			goto RETYPE_COKOA;
	}

	return 0;
}

static void demo_OSD(void)
{
	int loop;
	int bmp_drawer_handle, text_drawer_handle;
	U32_T font_addr, palette_addr;
	E_VC_OSD_ALPHA_T font_alpha;
	E_VC_OSD_ALPHA_T back_alpha;
	U8_T font_color_idx, back_color_idx;
	const char *string;
	U32_T text_mov_x;
	int display_cnt = 0;

	text_drawer_handle = rtiVC_CreateTextOsdDrawer();
	if (text_drawer_handle < 0) {
		EMSG("Can't create a text OSD drawer\n");
		return;
	}

	bmp_drawer_handle = rtiVC_CreateBitmapOsdDrawer();
	if (bmp_drawer_handle < 0) {
		rtiVC_DestoryTextOsdDrawer(text_drawer_handle);
		EMSG("Can't create a bitmap OSD drawer\n");
		return;
	}

#if 0
	// To test OSD, Test pattern or HMDI input must enabled.
	rtiVC_EnableTestPattern(TRUE);
	rtiVC_GenerateTestPattern(0x00, 0x00, 0xFF);
#endif

	font_addr = FLASH_ADDR_FONT;
	palette_addr = FLASH_ADDR_PALETTE_COLOR_SET_0 + (1/*lut offset*/ * VC_OSD_FONT_LUT_SUBSET_SIZE);
	font_alpha = VC_OSD_ALPHA_100;
	back_alpha = VC_OSD_ALPHA_0;
	font_color_idx = 10; /* from 0 to 15 */
	back_color_idx = 9;
	string = "REALITY ALWAYS ON";
	text_mov_x = 100;

	rtiVC_DrawBitmapOSD(bmp_drawer_handle, 100, 100, FLASH_PART_OFFSET_BMP,
						VC_BITMAP_OSD_MAG_x2,
						VC_OSD_ALPHA_100, VC_OSD_DIMENSTION_2D);			

	rtiVC_DrawTextOSD(text_drawer_handle, text_mov_x, 900, font_addr, palette_addr,
					VC_TEXT_OSD_MAG_x3, font_alpha, back_alpha,
					font_color_idx, back_color_idx, VC_OSD_DIMENSTION_2D, string);

	for (loop = 0; loop < 20; loop++) {
		VC_M_DELAY(1000);

		switch (display_cnt) {
		case 0:
			rtiVC_MoveBitmapOSD(bmp_drawer_handle, 1400, 100);
			break;

		case 1:
			rtiVC_MoveBitmapOSD(bmp_drawer_handle, 800, 400); // center
			break;
		
		case 2:
			rtiVC_MoveBitmapOSD(bmp_drawer_handle, 100, 600);
			break;

		case 3:
			rtiVC_MoveBitmapOSD(bmp_drawer_handle, 1400, 600);
			break;

		case 4:
			rtiVC_MoveBitmapOSD(bmp_drawer_handle, 100, 100);
			break;
		}

		text_mov_x += 100;
		rtiVC_MoveTextOSD(text_drawer_handle, text_mov_x, 900);

		if (text_mov_x == 900)
			text_mov_x = 100;

		
		display_cnt++;
		if (display_cnt == 5)
			display_cnt = 0;
	}

	rtiVC_DestoryBitmapOsdDrawer(bmp_drawer_handle);
	rtiVC_DestoryTextOsdDrawer(text_drawer_handle);

	rtiVC_EnableTestPattern(FALSE);	
}

static void demo_CoKOA(void)
{
	int i;
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];
	int lut_idx = 0;
	UINT_T ctrl_port = 0x0;

#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	ctrl_port = VC_PANEL_CTRL_PORT_0;
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	ctrl_port |= VC_PANEL_CTRL_PORT_1;
#endif

#if 0
	// To test OSD, Test pattern or HMDI input must enabled.
	rtiVC_EnableTestPattern(TRUE);
	rtiVC_GenerateTestPattern(0xFF, 0xFF, 0xFF);
#endif

{
	VC_COKOA_PARAM_T param;

#if 1
	param.lt_x = 255;
	param.lt_y = 0;
	param.rt_x = 255;
	param.rt_y = 0;

	param.lb_x = 0;
	param.lb_y = 0;
	param.rb_x = 0;
	param.rb_y = 0;

#else
	param.lt_x = -256;
	param.lt_y = -64;
	param.rt_x = 255;
	param.rt_y = 63;

	param.lb_x = 255;
	param.lb_y = 0;
	param.rb_x = -256;
	param.rb_y = 0;
#endif

	param.k1 = 0;
	param.k2 = 0;

	param.chrm_r = 0;
	param.chrm_g = 0;
	param.chrm_b = 10;

	param.interp = 1;

	rtiVC_RequestCoKOAGeneration(&param);

	param.lt_x = 0;
	param.rt_x = 0;
	param.k1 = 0.1;
	param.chrm_r = 70;
	param.chrm_g = 30; 
	param.interp = 1;

	rtiVC_RequestCoKOAGeneration(&param);
}

	lut_offset[VC_PANEL_PORT_0] = 0;
	lut_offset[VC_PANEL_PORT_1] = 1;
	rtiVC_EnableGammaCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset);

	for(i = 0; i < 20; i++)	{
		if ((lut_idx == 0) || (lut_idx == 1)) { // CoKOA_fhd_color_ab_barrel.lut, CoKOA_fhd_color_ab_pinc.lut
			if (lut_idx == 0) { // toggle test
				lut_offset[VC_PANEL_PORT_0] = 0;
				lut_offset[VC_PANEL_PORT_1] = 1;
			}
			else {
				lut_offset[VC_PANEL_PORT_0] = 1;
				lut_offset[VC_PANEL_PORT_1] = 0;
			}				
		
			rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port,
											TRUE, lut_offset, TRUE);
		}
		else
			rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port,
											TRUE, lut_offset, FALSE);

		if (++lut_idx == 7)
			lut_idx = 0;

		VC_M_DELAY(1000);
	}

	rtiVC_EnableGammaCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset);
}

void test_ChangeInputDisplayInterface_RDC200A(int key)
{
//	int key, ret;
	int ret;
	VC_INPUT_DISPLAY_IF_CFG cfg;

	while(1) {
		DMSG("========== Test Change Input Display Interface =============\n");
		DMSG("\t0: LVDS port 0\n");
		DMSG("\t1: LVDS port 1\n");
		DMSG("\t2: LVDS both(0 and 1)\n");
		DMSG("\t3: MIPI\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case '0':
			DMSG("LVDS port 0\n");
			cfg.lvds.color_depth = VC_LVDS_COLOR_DEPTH_24;

			ret = rtiVC_ChangeInputDisplayInterface(VC_INPUT_DISPLAY_IF_LVDS_0,
													&cfg);
			if (ret)
				EMSG("Changing of DIF failed. (%d)\n", ret);
			break;

		case '1':
			DMSG("LVDS port 1\n");
			cfg.lvds.color_depth = VC_LVDS_COLOR_DEPTH_24;
			ret = rtiVC_ChangeInputDisplayInterface(VC_INPUT_DISPLAY_IF_LVDS_1,
													&cfg);
			if (ret)
				EMSG("Changing of DIF failed. (%d)\n", ret);			
			break;

		case '2':
			DMSG("LVDS both(0 and 1)\n");
			cfg.lvds.color_depth = VC_LVDS_COLOR_DEPTH_24;
			cfg.lvds.dual_mode_type = VC_LVDS_DUAL_MODE_PORT0_EVEN;
			ret = rtiVC_ChangeInputDisplayInterface(VC_INPUT_DISPLAY_IF_LVDS_BOTH,
													&cfg);
			if (ret)
				EMSG("Changing of DIF failed. (%d)\n", ret);			
			break;

		case '3':
			DMSG("MIPI\n");
			cfg.mipi.dr = VC_MIPI_DR_1000Mbps;
			cfg.mipi.num_lane = VC_MIPI_NUM_LANE_4;
			cfg.mipi.auto_pd = VC_MIPI_AUTO_PD_pwr_up_inactive;
			cfg.mipi.cont_clk = VC_MIPI_CONT_CLK_none;
			
			ret = rtiVC_ChangeInputDisplayInterface(VC_INPUT_DISPLAY_IF_MIPI,
													&cfg);
			if (ret)
				EMSG("Changing of DIF failed. (%d)\n", ret);			
			break;
	
		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
	}

TEST_TP_EXIT:

	return;
}

void test_ATW_pixel_shift(void)
{
	int i;
	E_VC_PANEL_CTRL_PORT_T ctrl_port;
	RTI_VC_ATW_PIXEL_SHIFT_SET_INFO_T info[MAX_NUM_VC_PANEL_PORT];

#if defined(_CFG_RDP_ATTACHED_TO_PORT_0) && !defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_0;
#elif !defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_1;
#elif defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_ALL;
#else
	#error "Code not present"
#endif

	rtiVC_EnableVideoSyncInterrupt();

#if 1 // betta masked
	for (i = 0; i < 1000; i++) {
		if (i & 0x1) {
			/*
			info[0].shift_x = -10;
			info[0].shift_y = -30;

			info[1].shift_x = -10;
			info[1].shift_y = -30;
*/
			info[0].shift_x = -400;
			info[0].shift_y = -300;

			info[1].shift_x = -400;
			info[1].shift_y = -300;

			//info[1].shift_x = 0x100;
			//info[1].shift_y = 0x100;
		
			rtiVC_RequestAtwShiftPixel(ctrl_port, info);
			//VC_M_DELAY(3);
		}
		else {
			info[0].shift_x = 100;
			info[0].shift_y = 80;

			info[1].shift_x = 100;
			info[1].shift_y = 80;

			//info[1].shift_x = 0xFF;
			//info[1].shift_y = 0xFF;

			rtiVC_RequestAtwShiftPixel(ctrl_port, info);
			//VC_M_DELAY(3);
		}

		//VC_M_DELAY(3);
	}

#else
	info[0].shift_x = 0x12;
	info[0].shift_y = 0x34;

	info[1].shift_x = 0x56;
	info[1].shift_y = 0x78;

#if 0 // betta changed
	if (count & 0x1) {
				info[0].shift_x = -10;
				info[0].shift_y = -30;

				info[1].shift_x = -10;
				info[1].shift_y = -30;

				//info[1].shift_x = 0x100;
				//info[1].shift_y = 0x100;

		rtiVC_RequestAtwShiftPixel(ctrl_port, info);
				//VC_M_DELAY(3);
	}
	else {
				info[0].shift_x = 100;
				info[0].shift_y = 80;

				info[1].shift_x = 100;
				info[1].shift_y = 80;

				//info[1].shift_x = 0xFF;
				//info[1].shift_y = 0xFF;

				rtiVC_RequestAtwShiftPixel(ctrl_port, info);
				//VC_M_DELAY(3);
	}
	count++;
	if (count>=1000) count=0;
#else
	info[0].shift_x = 0x12;
		info[0].shift_y = 0x34;

		info[1].shift_x = 0x56;
		info[1].shift_y = 0x78;
	for (i = 0; i < 1000; i++) {
		rtiVC_RequestAtwShiftPixel(ctrl_port, info);
		//VC_M_DELAY(3);
	}
#endif
#endif

	rtiVC_DisableVideoSyncInterrupt();
}

static void test_change_input_resol(void)
{
	VC_INPUT_RES_T res;
	BOOL_T enable_3D;
	U32_T h_blank, v_blank,	frame_rate;

	res = VC_INPUT_RES_HD;
	enable_3D = FALSE;
	h_blank = 370;
	v_blank = 30;
	frame_rate = 60;
	
    rtiVC_ChangeInputResolution(VC_INPUT_RES_HD, enable_3D, h_blank, v_blank, frame_rate);

	//enable_3D = TRUE;
	//rtiVC_ChangeInputResolution(VC_INPUT_RES_DUALHD, enable_3D, h_blank, v_blank, frame_rate);
}

int test_RDC200A(E_VC_PANEL_DEVICE_T panel_type, int key)
{
//	int key, ret = 0;
	int ret = 0;
	double gamma_val;
	static char name_buf[64], fname_buf[1024];
	char *filename = NULL, *filename_1 = NULL;
	U8_T major_ver, minor_ver, rel_ver;
	U16_T chip_id, rev_num;
    unsigned int reg_addr, write_data;
	reg_addr = 0x0550;
	write_data = 0;

    // Open device
    ret = rtiVC_OpenDevice();
    if (ret)
        return -1;
    
    rtiVC_GetFirmwareVersion(&major_ver, &minor_ver, &rel_ver);
	DMSG("RDC200A firmware version: %d.%d.%d\n", major_ver, minor_ver, rel_ver);

	rtiVC_GetRdcDeviceID(&chip_id, &rev_num);
	DMSG("RDC200A chipid(0x%02X) rev_num(%d)\n", chip_id, rev_num);

#if 0
	// temp. flag and file path are will be recv from UI
	vc_SetLutDirectoryPath(RDC200A_LUT_FILE_PATH);
#endif

#if 0
	while(1) {
#endif
		DMSG("================ [RDC200A - %s] Test ===============\n", g_panel_name);
		DMSG("\t0: [Download] Firmware Hex file\n");
		DMSG("\t1: [Download] Firmware Configuration\n");
		DMSG("\t2: [Download] RDC200A Init LUT\n");
		DMSG("\t3: [Download] Panel Init LUT\n");
		DMSG("\t4: [Download] Panel Gamma LUT\n");
		DMSG("\t7: [Download] Optic CoKOA LUT\n");
		DMSG("\t8: [Download] Font LUT\n");
		DMSG("\t9: [Download] Palette LUT\n");
		DMSG("\ta: [Download] BMP LUT\n");

		DMSG("\n");
		DMSG("\t5: [LED Control] Request LED backlight<Using RDC200A>\n");


		DMSG("\n");
		DMSG("\tb: [RDP] Temperature sensor calibration\n");
		DMSG("\n");
		DMSG("\tp: [RDC] Change I2C slave address and Bus number \n");
		DMSG("\n");
		DMSG("\ts: [Full Download] Using file list <rdc200a_download_file_list.txt> instead of user input\n");
		DMSG("\n");
		DMSG("\tc: [Tuning] Brightness\n");
		DMSG("\td: [Tuning] Gamma (include LUT file generation)\n");
		DMSG("\tf: [Tuning] Horizontal Pixel shift<Using RDC200A>\n");
		DMSG("\tg: [Tuning] Vertical Pixel shift<Using RDC200A>\n");
		DMSG("\th: [Tuning] Frame rate control\n");
		DMSG("\ti: [Tuning] LED duty control\n");
		DMSG("\tr: [Tuning] LED current control<Using RDP>\n");
#if defined(CFG_PANEL_RDP550F) || defined(CFG_PANEL_RDP551F) || defined(CFG_PANEL_RDP502H) || defined(CFG_PANEL_RDP370F)
		DMSG("\to: [Tuning] Factory Calibration\n");
#endif
		DMSG("\n");
		DMSG("\tj: Test Pattern\n");
		DMSG("\tk: Image display format (2D/3D_SBS)\n");
		DMSG("\tl: Generate CoKOA Lut\n");
		DMSG("\n");
		DMSG("\tm: [OSD] Bitmap\n");
		DMSG("\tn: [OSD] Font\n");
		DMSG("\n");
		DMSG("\tx: [Register IO Test] RDC200A\n");
#if defined(_CFG_RDP_ATTACHED_TO_PORT_0)
		DMSG("\ty: [Register IO Test] Panel 0: %s\n", g_panel_name);
#endif
#if defined(_CFG_RDP_ATTACHED_TO_PORT_1)
		DMSG("\tz: [Register IO Test] Panel 1: %s\n", g_panel_name);
#endif
		DMSG("\n");
		DMSG("\tv: Change Input Display Interface\n");
		DMSG("\n");
		DMSG("\tt [Demo] CoKOA\n");
		DMSG("\tu [Demo] OSD\n");
		DMSG("\n");
		DMSG("\tw: [FLASH] Test\n");
		DMSG("\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		switch(key)
		{
		case '0':
			DMSG("[Download] Firmware Hex file\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(fw_hex_ext);
			if (filename) {
				ret = prepare_firmware_hex_file(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '1':
			DMSG("[Download] Firmware Configuration\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(fw_cfg_ext);
			if (filename) {
				ret = prepare_firmware_config_file(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '2':
			DMSG("[Download] RDC200A Init LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(rdc200a_init_ext);
			if (filename) {
				ret = prepare_rdc_init_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '3':
			DMSG("[Download] Panel Init LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(panel_init_ext);
			if (filename) {
				ret = prepare_panel_init_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '4':
			DMSG("[Download] Panel Gamma LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(gamma_lut_ext);
			if (filename) {
				ret = prepare_panel_gamma_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;
		case '7':
			DMSG("[Download] Optic CoKOA LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(cokoa_ext);
			if (filename) {
				ret = prepare_optic_dc_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '8':
			DMSG("[Download] Font LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(font_ext);
			if (filename) {
				ret = prepare_font_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '9':
			DMSG("[Download] Palette LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(palette_ext);
			if (filename) {
				ret = prepare_palette_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case 'a':
			DMSG("[Download] BMP LUT\n");
#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(bitmap_ext);
			if (filename) {
				ret = prepare_bmp_lut(filename);
				if (ret == 0)
					rtiVC_StartDownload(); // API call.
			}
#endif
			break;

		case '5':
			DMSG("[LED Control] Request LED backlight\n");
			test_RequestLEDcontrol('0');
			break;

		case 'b':
			DMSG("[RDP] Temperature sensor calibration\n");
			test_TemperatureSensorCalibration(g_panel_name, '0');
			break;

		case 's':
			DMSG("[Full Download] Using file list <rdc200_download_file_list.txt> instead of user input\n");
#if 0
            sprintf(fname_buf, "%s%c%s", RDC200A_LUT_FILE_PATH, PathSeparator, "rdc200a_download_file_list.txt");
            test_Download_FileList(fname_buf, TRUE, LUT_DUMP_FILE_PATH, VC_LUT_DUMP_NONE);
#endif
			break;

		case 'c':
			DMSG("[Tuning] Brightness\n");
#if 1
			test_Brightness('+');
			test_Brightness('-');
#endif
			break;

		case 'd':
			DMSG("[Tuning] Gamma. (Enable test-pattern!)\n");

RETRY_SEL_GAMMA_VT_FILE:
			DMSG("\t\tChoose a gamma file of LOW polarity\n");

#if 0
			filename = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(gamma_vt_ext);
			if (filename) {
				DMSG("\t\tChoose a gamma file of HIGH polarity\n");

				filename_1 = GET_RDC200A_DOWNLOAD_FILE_FROM_USER(gamma_vt_ext);
				if (filename_1) {
					if (!strcmp(filename, filename_1)) {
						DMSG("File name is the same. Retry!\n");
						goto RETRY_SEL_GAMMA_VT_FILE;
					}

					if ((gamma_val = select_gamma_value()) > 0.) {
						ret = prepare_panel_gamma_vt(filename, filename_1, gamma_val);
						if (ret == 0) {
							rtiVC_StartDownload(); // API call.

							rtiVC_EnableTestPattern(TRUE);
							rtiVC_GenerateTestPattern(0x00, 0x80, 0x00);
#if 0
							{
								UINT_T lut_offset[2] = {0, 0};
								rtiVC_EnableGammaCorrection(VC_PANEL_CTRL_PORT_1, TRUE, lut_offset);
							}
#endif
						}
					}			
				}
			}
#endif
			break;

		case 'p':
			DMSG("[RDC] Change I2C slave address\n");
#if 0
			/*
			 * CH: IT1_CDCHID_20220809.elf section `.data' will not fit in region `FLASH'
			 * section .app_info LMA [000000000801ffd0,000000000801ffdf] overlaps section .data LMA [000000000801fabc,0000000008020103]
			 * region `FLASH' overflowed by 260 bytes
			 */
			test_ChangeI2cSlaveAddressRDC();
#endif
			break;

		case 'f':
			DMSG("[Tuning] Horizontal Pixel shift<Using RDC>\n");
#if 1
			test_ShiftHorizontalPixel('+');
			test_ShiftHorizontalPixel('-');
#endif
			break;

		case 'g':
			DMSG("[Tuning] Vertical Pixel shift<Using RDC>\n");
			test_ShiftVerticalPixel('+');
			test_ShiftVerticalPixel('-');
			break;

		case 'h':
			DMSG("[Tuning] Frame rate control\n");
			test_FrameRateControl('+');
			test_FrameRateControl('-');
			break;

		case 'i':
			DMSG("[Tuning] LED duty control\n");
#if 0
			/*
			 * CH: IT1_CDCHID_20220809.elf section `.rodata' will not fit in region `FLASH'
			 * section .app_info LMA [000000000801ffd0,000000000801ffdf] overlaps section .rodata LMA [000000000801dc40,0000000008020c9b]
			 * region `FLASH' overflowed by 4848 bytes
			 */
			test_LedDutyControl('R', '+');
			test_LedDutyControl('R', '-');
			test_LedDutyControl('G', '+');
			test_LedDutyControl('G', '-');
			test_LedDutyControl('B', '+');
			test_LedDutyControl('B', '-');
#endif
			break;

		case 'r':
			DMSG("[Tuning] LED current control\n");
#if 0
			/*
			 * CH: IT1_CDCHID_20220809.elf section `.data' will not fit in region `FLASH'
			 * section .app_info LMA [000000000801ffd0,000000000801ffdf] overlaps section .data LMA [000000000801fd5c,000000000802039f]
			 * region `FLASH' overflowed by 928 bytes
			 */
			test_LedCurrentControl('R', '+');
			test_LedCurrentControl('R', '-');
			test_LedCurrentControl('G', '+');
			test_LedCurrentControl('G', '-');
			test_LedCurrentControl('B', '+');
			test_LedCurrentControl('B', '-');
#endif
            break;
                
		case 'j':
			DMSG("Test Pattern\n");
			test_TestPattern("RDC200A", '0');
			break;

		case 'k':
			DMSG("Image display format (2D/3D)\n");
#if 1
			test_ImageDisplayFormat('0', 0);
#endif
			break;

		case 'l':
			DMSG("Generate CoKOA Lut\n");
			test_CoKOA();
			//test_KeystoneCoKOA();
			//test_AutoGenKeystoneCoKOA();
			break;

		case 'm':
			DMSG("[OSD] Bitmap\n");
#if 0
			/*
			 * CH: IT1_CDCHID_20220809.elf section `.rodata' will not fit in region `FLASH'
			 * section .app_info LMA [000000000801ffd0,000000000801ffdf] overlaps section .rodata LMA [000000000801d738,0000000008020743]
			 * region `FLASH' overflowed by 3616 bytes
			 */
			test_BitmapOSD('0');
#endif
			break;

		case 'n':
#if 0
			DMSG("[OSD] Font\n");
			test_FontOSD();
#else
			DMSG("[ATW] Pixel shift\n");
			test_ATW_pixel_shift();
#endif
			break;

		case 'o':
			DMSG("[Tuning] Factory Calibration\n");
#if 0
			test_FactoryCalibration();
#endif
			break;

		case 'x':
#if 0
			test_RegisterIO_RDC("RDC200A");
#endif
			break;
#if defined(_CFG_RDP_ATTACHED_TO_PORT_0)
		case 'y':
			sprintf(name_buf, "Panel 0: %s", g_panel_name);
#if 1
			test_RegisterIO_RDP(0, name_buf, '0', reg_addr, 10, write_data);
#endif
			break;
#endif
#if defined(_CFG_RDP_ATTACHED_TO_PORT_1)
		case 'z':
			sprintf(name_buf, "Panel 1: %s", g_panel_name);
#if 1
			test_RegisterIO_RDP(1, name_buf, '0', reg_addr, 10, write_data);
#endif
			break;
#endif
		case 'v':
			DMSG("Change Input Display Interface\n");
			test_ChangeInputDisplayInterface_RDC200A('0');
			break;

		case 't':
			DMSG("[Demo] CoKOA\n");
			demo_CoKOA();
			break;

		case 'u':
			DMSG("[Demo] OSD\n");
			demo_OSD();
			break;

		case 'w':
			DMSG("[FLASH] Test\n");
#if 0
			/*
			 * CH: IT1_CDCHID_20220809.elf section `.rodata' will not fit in region `FLASH'
			 * section .app_info LMA [000000000801ffd0,000000000801ffdf] overlaps section .rodata LMA [000000000801d858,0000000008020987]
			 * region `FLASH' overflowed by 4160 bytes
			 */
			test_FLASH("RDC200A", '0');
#endif
			//DMSG("Change Input resolution Test\n");
			//test_change_input_resol();
			break;

		case 'q':
		case 'Q':
			goto REG_IO_TEST_EXIT;
		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

REG_IO_TEST_EXIT:

	return 0;
}
#endif // #ifdef CFG_MDC_RDC200A

