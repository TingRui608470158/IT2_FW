
#ifndef __TEST_H__
#define __TEST_H__

#if (!defined(__KERNEL__) && defined(__linux__) /* Linux application */ || defined(__APPLE__))
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>

#elif defined(WINCE) || defined(WINDOWS) || defined(_WIN32)
#include <io.h>
#include <errno.h>
#include <conio.h>
#include <time.h>
#endif

#include "main.h"
#include "rti_vc_config.h"
#include "rti_vc_api.h"
#include <time.h>

#ifdef __cplusplus
extern "C"{
#endif

#if defined(CFG_MDC_RDC200)
	#define RDC_I2C_SLAVE_ADDR		0x44

#elif defined(CFG_MDC_RDC200A)
	#define RDC_I2C_SLAVE_ADDR		0x4A
#else
	#error "Code not present"
#endif

#if defined(CFG_MDC_RDC200) || defined(CFG_MDC_RDC200A)
#define LUT_DUMP_FILE_PATH\
	"C:\\home\\src\\LCoS\\Vista-Cube\\examples\\mcu_example"
#endif

#if 0
#define CLEAR_STDIN\
	do {\
		while (getc(stdin) != '\n');\
	} while (0)
#endif

#if  defined(WINCE) || defined(WINDOWS) || defined(_WIN32)
#define PathSeparator '\\'

#define FFLUSH	fflush(stdin)

#else
#define PathSeparator '/'

#define FFLUSH	((void)0)
#endif
    
typedef struct
{
	E_VC_MDC_DEVICE_T rdc;
	const char *str;
} RDC_DEV_TABLE_T;

typedef struct
{
	E_VC_PANEL_DEVICE_T panel;
	const char *str;
} PANEL_DEV_TABLE_T;

typedef struct
{
	RDC_DEV_TABLE_T *rdc_tbl_ptr;
	PANEL_DEV_TABLE_T *panel_tbl_ptr;
} VC_DEVICE_SELECT_INFO_T;
   
// return: msec
static double time_elapse(void)
{
#if (!defined(__KERNEL__) && defined(__linux__) /* Linux application */) || defined(__APPLE__)
	static struct timeval st, sp;

    struct timeval tt;
    gettimeofday(&sp, NULL);

    tt.tv_sec = sp.tv_sec - st.tv_sec;
    tt.tv_usec = sp.tv_usec - st.tv_usec;

    gettimeofday(&st, NULL);

    if (tt.tv_usec < 0) {
        tt.tv_usec += 1000000;
        tt.tv_sec--;
    }

    return (double)tt.tv_usec/1000000 + (double)tt.tv_sec;
#elif defined(_WIN32)
	static DWORD old_tick = 0, tick_val;
	DWORD time_elapsed = 0;

	tick_val = GetTickCount();

	if (old_tick == 0)
		old_tick = tick_val;
	else {
		time_elapsed = tick_val - old_tick;
		old_tick = 0;
	}

	return (double)time_elapsed;

#else
	static uint32_t old_tick = 0, tick_val;
	uint32_t time_elapsed = 0;

	tick_val = HAL_GetTick();

	if (old_tick == 0)
		old_tick = tick_val;
	else {
		time_elapsed = tick_val - old_tick;
		old_tick = 0;
	}

	return (double)time_elapsed;
//	#error "Code not present"
#endif
}
///

static BOOL_T is_string_double(char *str)
{
	size_t i = 0;
	int comma_cnt = 0;
	size_t len = strlen(str);

	if (len == 0)
		return FALSE;

	if (str[0] == '-') // minus notation
		i = 1; // skip

	for ( ; i < len; i++) {
		if (str[i] != '.') {
			if (!isdigit(str[i]))
				return FALSE;
		}
		else
			comma_cnt++;
	}

	if (comma_cnt > 1)
		return FALSE;

	return TRUE;
}

static BOOL_T is_string_digit(const char *str)
{
	size_t i = 0;

	if (str[0] == '-') // minus notation
		i = 1; // skip

	for ( ; i < strlen(str); i++) {
		if (!isdigit(str[i]))
			return FALSE;
	}

	return TRUE;
}

void vc_dload_proc_callback(E_VC_DLOAD_STATUS_T status,
							E_VC_LUT_CLASS_T lut_class,
							U8_T *pgm_data_buf_ptr,
							U16_T processed_item_cnt, U16_T total_item);

void vc_dload_verify_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_verify_addr,
								U16_T verified_bytes, U16_T total_verify_bytes);

void vc_dload_program_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_pgm_addr,
								U16_T programmed_bytes, U16_T total_pgm_bytes);

void vc_dload_erase_callback(E_VC_DLOAD_STATUS_T status,
								E_VC_LUT_CLASS_T lut_class,
								U32_T curr_erase_addr,
								U16_T erased_sector_cnt, U16_T total_sector);

int test_Download_FileList(const char *filename, BOOL_T dload_activate,
							char *dump_path, UINT_T dump_flags);

int vc_download_all_lut_rdc100(const char *file_ext_list[], int num_file_ext);

char *get_download_file_from_user(const char *path, const char *file_ext_list[], int num_file_ext);

double select_gamma_value(void);

void test_LedCurrentControl(int key, char ch);
void test_LedDutyControl(int key, char ch);
void test_FrameRateControl(char ch);

void test_ImageDisplayFormat(int key, unsigned int swap_image_3d_mode);

void test_Brightness(char ch);
void test_ShiftVerticalPixel(char ch);
void test_ShiftHorizontalPixel(char ch);
void test_TestPattern(const char *rdc_name, int key);

void test_DumpFlashData(void);

void test_PanelPowerOnOff(int key);

void test_ChangeI2cSlaveAddressRDC(int key);

void test_RegisterIO_RDP(int panel_port, const char *test_dev_name, int key, unsigned int reg_addr, unsigned int read_cnt, unsigned int write_data);
void test_RegisterIO_RDC(const char *test_dev_name, int key, unsigned int reg_addr, unsigned int write_data);

int test_RDC200(E_VC_PANEL_DEVICE_T panel_type);
int test_RDC200A(E_VC_PANEL_DEVICE_T panel_type, int key);

int test_RDC200_auto(E_VC_PANEL_DEVICE_T panel_type, const char * argv[]);
int test_RDC200A_auto(E_VC_PANEL_DEVICE_T panel_type, const char * argv[]);

#define VC_LUT_DUMP_NONE		0x0
#define VC_LUT_DUMP_HEX_ARRAY	0x1
#define VC_LUT_DUMP_ITEM_BIN	0x2
#define VC_LUT_DUMP_FULL_BIN	0x4
#define VC_LUT_DUMP_ALL\
	(VC_LUT_DUMP_HEX_ARRAY|VC_LUT_DUMP_ITEM_BIN|VC_LUT_DUMP_FULL_BIN)

const char *vc_GetDumpDirectoryPath(void);
void vc_SetLutDumpInfomation(char *dump_path, UINT_T dump_flags);

const char *vc_GetLutDirectoryPath(void);
void vc_SetLutDirectoryPath(char *lut_path);

void write_item_bin_output_file(U8_T *buf, U32_T size, char *var_name);
void write_lut2bin_output_file(long file_offset, U8_T *buf, U32_T size);
void write_lut2hex_output_file(U8_T *buf, U32_T size, char *var_name);
int open_lut_dump_file(void);
void close_lut_dump_file(void);
void vc_SetLutDumpFlag(UINT_T flags);

char *trim_left(char *str);
char *trim_right(char *str);
char *trim_string(char *str);

void test_FactoryCalibration(int key, unsigned int reg_addr, unsigned int reg_val);

void test_RequestLEDcontrol(int key);

void test_FLASH(const char *rdc_name, int key);

void test_TemperatureSensorCalibration(const char *panel_name, int key);

extern const char *g_panel_name;

#ifdef __cplusplus
}
#endif

#endif /* __TEST_H__ */

