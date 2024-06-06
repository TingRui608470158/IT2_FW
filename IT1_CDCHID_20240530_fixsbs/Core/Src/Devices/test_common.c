
#include "test.h"

#include "rti_vc_common.h"
#include "rdc_lut_patch.h"
#if defined(CFG_MDC_RDC200)
	#include "rdc200_flash_partitions.h"

#elif defined(CFG_MDC_RDC200A)	
	#include "rdc200a_flash_partitions.h"
	#include "rdc200a_internal.h"

#elif defined(CFG_MDC_RDC100)
#endif

#if defined(CFG_PANEL_RDP501H)
	const char *g_panel_name = "RDP501H";

#elif defined(CFG_PANEL_RDP360H)
	const char *g_panel_name = "RDP360H";
	
#elif defined(CFG_PANEL_RDP550F)
	const char *g_panel_name = "RDP550F";
	
#elif defined(CFG_PANEL_RDP700Q)	
	const char *g_panel_name = "RDP700Q";
	
#elif defined(CFG_PANEL_RDP551F)
	const char *g_panel_name = "RDP551F";

#elif defined(CFG_PANEL_RDP502H)	
	const char *g_panel_name = "RDP502H";

#elif defined(CFG_PANEL_RDP370F)
	const char *g_panel_name = "RDP370F";

#elif defined(CFG_PANEL_RDP250H)
	const char *g_panel_name = "RDP250H";

#else
	#error "Code not present"
#endif

char *trim_left(char *str)
{
	while ((*str != '\0') && isspace(*str))
		++str;

	return (char *)str;
}

char *trim_right(char *str)
{
	size_t size;
	char *end;

	size = strlen(str);
	if (!size)
		return str;

	// Removes trailing whitespace.
	end = str + size - 1;
	while (end >= str && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return str;
}

char *trim_string(char *str)
{
	return trim_left(trim_right(str));
}

#if (!defined(__KERNEL__) && defined(__linux__)  /* Linux application */ || defined(__APPLE__))
  
#include <termios.h>  

int _getch(void)
{  
	int ch;  
	struct termios buf;  
	struct termios save;  

	tcgetattr(0, &save);  
	buf = save;  
	buf.c_lflag &= ~(ICANON|ECHO);  
	buf.c_cc[VMIN] = 1;  
	buf.c_cc[VTIME] = 0;  
	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);
	return ch;
}
#endif

static UINT_T lut_dump_flag = VC_LUT_DUMP_NONE;

static FILE *fp_lut_dump_txt = NULL;
static FILE *fp_lut_dump_full_bin = NULL;

static char txt_dump_fname[1024];
static char full_bin_dump_fname[1024];
static char item_bin_dump_fname[1024];

static char vc_dump_dir_path[1024];
static char vc_lut_dir_path[1024];

void vc_SetLutDumpFlag(UINT_T flags)
{
	lut_dump_flag = flags;
}

void write_item_bin_output_file(U8_T *buf, U32_T size, char *var_name)
{
	size_t numwritten;
	FILE *fp_out = NULL;

	if (lut_dump_flag & VC_LUT_DUMP_ITEM_BIN) {
		sprintf(item_bin_dump_fname, "%s%c%s.bin", vc_GetDumpDirectoryPath(), PathSeparator, var_name);

		fp_out = fopen(item_bin_dump_fname, "wb");
		if (fp_out == NULL) {
			EMSG("Can't open %s to write\n", item_bin_dump_fname);
			return;
		}

		numwritten = fwrite(buf, sizeof(char), size, fp_out);
		if (numwritten != size) {
			EMSG("[DUMP item bin] File write error\n");
		}

		fclose(fp_out);
	}
}

// full bin
void write_lut2bin_output_file(long file_offset, U8_T *buf, U32_T size)
{
	size_t numwritten;

	if (lut_dump_flag & VC_LUT_DUMP_FULL_BIN) {
		if (fp_lut_dump_full_bin) {
			fseek(fp_lut_dump_full_bin, file_offset, SEEK_SET);

			numwritten = fwrite(buf, sizeof(char), size, fp_lut_dump_full_bin);
			if (numwritten != size) {
				EMSG("[DUMP full bin] File write error\n");
			}
		}
	}
}

void write_lut2hex_output_file(U8_T *buf, U32_T size, char *var_name)
{
	U32_T i;

	if (lut_dump_flag & VC_LUT_DUMP_HEX_ARRAY) {
		if (fp_lut_dump_txt) {
			fprintf(fp_lut_dump_txt, "const unsigned char %s[] = {\n\t", var_name);

			for (i = 0; i < size; i++) {
				if (i != (size-1))
					fprintf(fp_lut_dump_txt, "0x%02X, ", buf[i]);
				else
					fprintf(fp_lut_dump_txt, "0x%02X", buf[i]);

				if (i != 0) {
					if (((i+1)%10) == 0)
						fprintf(fp_lut_dump_txt, "\n\t");
				}
			}

			fprintf(fp_lut_dump_txt, "\n};\n\n");
		}
	}
}

int open_lut_dump_file(void)
{
	if (lut_dump_flag & VC_LUT_DUMP_HEX_ARRAY) {
#if defined(CFG_MDC_RDC200)
    sprintf(txt_dump_fname, "%s%c%s", vc_GetDumpDirectoryPath(), PathSeparator, "rdc200_lut_data.h");

#elif defined(CFG_MDC_RDC200A)
	sprintf(txt_dump_fname, "%s%c%s", vc_GetDumpDirectoryPath(), PathSeparator, "rdc200a_lut_data.h");

#else
	#error "Code not present"
#endif

		if (fp_lut_dump_txt == NULL) {
			fp_lut_dump_txt = fopen(txt_dump_fname, "wt");
			if (fp_lut_dump_txt == NULL) {
				EMSG("Can't open %s to write\n", txt_dump_fname);
				return -1;
			}

			DMSG("%s file opened\n", txt_dump_fname);
		}
	}

	if (lut_dump_flag & VC_LUT_DUMP_FULL_BIN) {
#if defined(CFG_MDC_RDC200)
	sprintf(full_bin_dump_fname, "%s%c%s", vc_GetDumpDirectoryPath(), PathSeparator, "rdc200_flash_data.bin");
	
#elif defined(CFG_MDC_RDC200A)
    sprintf(full_bin_dump_fname, "%s%c%s", vc_GetDumpDirectoryPath(), PathSeparator, "rdc200a_flash_data.bin");
#else
	#error "Code not present"
#endif

			if (fp_lut_dump_full_bin == NULL) {
				U8_T *init_buf;
				U32_T flash_erased_value_fill_size;

#if defined(CFG_MDC_RDC200)
				flash_erased_value_fill_size = FLASH_PART_OFFSET_BMP;
#elif defined(CFG_MDC_RDC200A)
				flash_erased_value_fill_size = FLASH_PART_OFFSET_BMP;
#else
	#error "Code not present"
#endif

				/*
				 BMP partition is the last partition.
				 So, we fill the 0xFF value front of BMP partition. 
				*/
				init_buf = (U8_T *)malloc(flash_erased_value_fill_size);
				if (init_buf == NULL) {
					EMSG("Full dump init allocation error\n");
					return -2;
				}

				fp_lut_dump_full_bin = fopen(full_bin_dump_fname, "wb");
				if (fp_lut_dump_full_bin == NULL) {
					EMSG("Can't open %s to write\n", full_bin_dump_fname);
					fclose(fp_lut_dump_txt);
					free(init_buf);
					return -1;
				}

				memset(init_buf, 0xFF, flash_erased_value_fill_size);
				fwrite(init_buf, sizeof(char), flash_erased_value_fill_size, fp_lut_dump_full_bin);
				free(init_buf);

				DMSG("%s file opened\n", full_bin_dump_fname);
			}
		}

	return 0;
}

void close_lut_dump_file(void)
{
	if (lut_dump_flag & VC_LUT_DUMP_HEX_ARRAY) {
		if (fp_lut_dump_txt) {
			fclose(fp_lut_dump_txt);
			fp_lut_dump_txt = NULL;
			//DMSG("%s file closed\n", txt_dump_fname);
		}
	}

	if (lut_dump_flag & VC_LUT_DUMP_FULL_BIN) {
		if (fp_lut_dump_full_bin) {
			fclose(fp_lut_dump_full_bin);
			fp_lut_dump_full_bin = NULL;
			//DMSG("%s file closed\n", full_bin_dump_fname);
		}
	}
}

const char *vc_GetDumpDirectoryPath(void)
{
	return (const char *)vc_dump_dir_path;
}

void vc_SetLutDumpInfomation(char *dump_path, UINT_T dump_flags)
{
	if (dump_path)
		strcpy(vc_dump_dir_path, dump_path);

	lut_dump_flag = dump_flags;
}

const char *vc_GetLutDirectoryPath(void)
{
	return (const char *)vc_lut_dir_path;
}

void vc_SetLutDirectoryPath(char *lut_path)
{
	strcpy(vc_lut_dir_path, lut_path);
}

E_VC_PANEL_CTRL_PORT_T test_InputSelection(int key)
{
//	int key;
	
    while(1) {
        DMSG("================ Target port Select ===============\n");
#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
        DMSG("\t0 : Port 0\n");
#endif
#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
        DMSG("\t1 : Port 1\n");
#endif
#if defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
        DMSG("\t2 : Port 0 & 1\n");
#endif
        DMSG("\tq or Q: Quit\n");

#if 0
        FFLUSH;
        key = getc(stdin);
        CLEAR_STDIN;
#endif
        
        switch(key)
        {
            case '0':
                return VC_PANEL_CTRL_PORT_0;
            case '1':
                return  VC_PANEL_CTRL_PORT_1;
            case '2':
                return  VC_PANEL_CTRL_PORT_ALL;
            case 'q':
            case 'Q':
                return VC_PANEL_CTRL_INVAILD;
			default:
				break;
		}
    }
	return VC_PANEL_CTRL_INVAILD;
}

void test_DumpPanelPatchRegister(E_VC_PANEL_CTRL_PORT_T ctrl_port)
{
	int ret;
	U8_T chk_port;
	int i, max_num_dump_lut_node, num_flash_node;
	RD_LUT_NODE_T *dump_lut_buf[MAX_NUM_VC_PANEL_PORT] = {NULL, NULL};
	RD_LUT_NODE_T *buf_ptr;

	max_num_dump_lut_node = FLASH_PART_SIZE_PANEL_PATCH / sizeof(RD_LUT_NODE_T);

	chk_port = (U8_T)ctrl_port;

	for (i = VC_PANEL_PORT_0; i < MAX_NUM_VC_PANEL_PORT; i++) {
		if (chk_port & 0x1) {
			dump_lut_buf[i] = (RD_LUT_NODE_T *)malloc(sizeof(RD_LUT_NODE_T) * max_num_dump_lut_node);
			if (dump_lut_buf[i] == NULL) {
				EMSG("Dump Panel_%d patch buffer malloc error!\n", i);
				ret = -1;
				goto exit_dump_panel;
			}

			buf_ptr = dump_lut_buf[i];
			
			num_flash_node
				= rtiVC_DumpPanelPatchLut(i, buf_ptr, max_num_dump_lut_node);
	
			if (num_flash_node <= 0) {
				ret = -2;
				goto exit_dump_panel;
			}
	
			DMSG("\t+---------------------+\n");
			DMSG("\t| %s (Port %d)  |\n", g_panel_name, i);
			DMSG("\t|-----------+---------|\n");
			DMSG("\t| Register  |  Value  |\n");
			DMSG("\t|-----------+---------|\n");
	
			for (i = 0; i < num_flash_node; i++) {
				DMSG("\t| 0x%04X    |  0x%02X   |\n",
					SWAP16(buf_ptr[i].reg), buf_ptr[i].val);
			}
	
			DMSG("\t+-----------+---------+\n");

		}		
		chk_port >>= 1;
	}

exit_dump_panel:
	if (dump_lut_buf[0])
		free(dump_lut_buf[0]);

	if (dump_lut_buf[1])
		free(dump_lut_buf[1]);
}

void test_DumpRdcPatchRegister(void)
{
	int i, max_num_dump_lut_node, num_flash_node;
	RD_LUT_NODE_T *dump_lut_buf = NULL;

	max_num_dump_lut_node = FLASH_PART_SIZE_RDC_PATCH / sizeof(RD_LUT_NODE_T);

	dump_lut_buf = (RD_LUT_NODE_T *)malloc(sizeof(RD_LUT_NODE_T) * max_num_dump_lut_node);
	if (dump_lut_buf == NULL) {
		EMSG("Dump RDC patch buffer malloc error!\n");
		return;
	}
	
	num_flash_node
		= rtiVC_DumpRdcPatchLut(dump_lut_buf, max_num_dump_lut_node);

	if (num_flash_node <= 0)
		return;

	DMSG("\t+---------------------+\n");
	DMSG("\t|        RDC200       |\n");
	DMSG("\t|-----------+---------|\n");
	DMSG("\t| Register  |  Value  |\n");
	DMSG("\t|-----------+---------|\n");

	for (i = 0; i < num_flash_node; i++) {
		DMSG("\t| 0x%04X    |  0x%02X   |\n",
			SWAP16(dump_lut_buf[i].reg), dump_lut_buf[i].val);
	}

	DMSG("\t+-----------+---------+\n");

	if (dump_lut_buf)
		free(dump_lut_buf);
}

void test_AlignmentVerticalShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port)
{
	int shift_val;
	int read_shift_val[MAX_NUM_VC_PANEL_PORT];
	char read_buf[128];
	int ret;

#if 0
	while (1) {
#endif
		DMSG(" -- Current Vertical shift value -- \n");
		rtiVC_GetVerticalPixelShift(panel_ctrl_port, read_shift_val);
		if (panel_ctrl_port & VC_PANEL_CTRL_PORT_0) {
			DMSG("Port0: %d\n", read_shift_val[0]);
		}

		if (panel_ctrl_port & VC_PANEL_CTRL_PORT_1) {
			DMSG("Port1: %d\n", read_shift_val[1]);
		}

#if defined(CFG_PANEL_RDP550F)
		DMSG("Input shift value(-8 ~ 8). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP551F)
		DMSG("Input shift value(0 ~ 200, cropped if exceed 32). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP502H)
        DMSG("Input shift value(0 ~ 200, cropped if exceed 32). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP370F)
        DMSG("Input shift value(0 ~ 200, cropped if exceed 32). q(Quit), r(Retry): ");
#endif

#if 0
		fgets(read_buf, 128, stdin);
#endif
		if (read_buf[0] == 'q' || read_buf[0] == 'Q')
			return;
		
		if (read_buf[0] == 'r' || read_buf[0] == 'R')
			return;
//			continue;
		
		sscanf(read_buf, "%d", &shift_val);

		ret = rtiVC_SetVerticalPixelShift(panel_ctrl_port, shift_val);
#if 0
	}
#endif
}

void test_AlignmentHorizontalShift(E_VC_PANEL_CTRL_PORT_T panel_ctrl_port)
{
	int shift_val;
	int read_shift_val[MAX_NUM_VC_PANEL_PORT];
	char read_buf[128];
	int ret;

#if 0
	while (1) {
#endif
		DMSG(" -- Current Horizontal shift value -- \n");
		rtiVC_GetHorizontalPixelShift(panel_ctrl_port, read_shift_val);
		if (panel_ctrl_port & VC_PANEL_CTRL_PORT_0) {
			DMSG("Port0: %d\n", read_shift_val[0]);
		}

		if (panel_ctrl_port & VC_PANEL_CTRL_PORT_1) {
			DMSG("Port1: %d\n", read_shift_val[1]);
		}
		
#if defined(CFG_PANEL_RDP550F)
		DMSG("Input shift value(-32 ~ 31). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP551F)
		DMSG("Input shift value(0 ~ 200, cropped if exceed 144). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP502H)
        DMSG("Input shift value(0 ~ 200, cropped if exceed 32). q(Quit), r(Retry): ");
#elif defined(CFG_PANEL_RDP370F)
		DMSG("Input shift value(0 ~ 200, cropped if exceed 32). q(Quit), r(Retry): ");
#endif	

#if 0
		fgets(read_buf, 128, stdin);
#endif
		if (read_buf[0] == 'q' || read_buf[0] == 'Q')
			return;
		
		if (read_buf[0] == 'r' || read_buf[0] == 'R')
			return;
//			continue;
		
		sscanf(read_buf, "%d", &shift_val);

		ret = rtiVC_SetHorizontalPixelShift(panel_ctrl_port, shift_val);

#if 0
	}
#endif
}

void test_FactoryCalibration(int key, unsigned int reg_addr, unsigned int reg_val)
{
	E_VC_PANEL_CTRL_PORT_T panel_ctrl_port;
//	int key;
//	unsigned int reg_addr, reg_val;

	#if defined(CFG_PANEL_RDP550F)
	#if (_CFG_RDP_SLAVE_ADDR != 0x18)
		DMSG("Not support to alignment panel test\n");
		return;
	#endif
	#endif

#if 0
	while(1) {
#endif
		DMSG("================ [Factory Calibration] ===============\n");
		DMSG("\t0: [Pixel shift] Horizontal shift\n");
		DMSG("\t1: [Pixel shift] Vertical shift\n");
		//DMSG("\t2: [Pixel shift] Show shift register values\n");
		DMSG("\t3: [Pixel shift] Download Pixel shift LUT into FLASH\n");
		DMSG("\n");

		DMSG("\t4: [Dump] Dump RDC patch LUT from FLASH\n");
		DMSG("\t5: [Dump] Dump Panel patch LUT from FLASH\n");
		DMSG("\n");

		DMSG("\ta: Erease RDC patch LUT in FLASH\n");
		DMSG("\tb: Erease Panel_0 patch LUT in FLASH\n");
		DMSG("\tc: Erease Panel_1 patch LUT in FLASH\n");
		DMSG("\n");

		DMSG("\td: [Manual] Tune RDC patch LUT\n");		
		DMSG("\te: [Manual] Download RDC LUT into FLASH\n");
		DMSG("\tf: [Manual] Reset RDC patch items\n");
		DMSG("\n");

		DMSG("\tg: [Manual] Tune Panel patch LUT\n");
		DMSG("\th: [Manual] Download Panel LUT into FLASH\n");
		DMSG("\ti: [Manual] Reset Panel patch items\n");
		DMSG("\n");
		
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case '0':
            panel_ctrl_port = test_InputSelection(key);
            if(panel_ctrl_port == VC_PANEL_CTRL_INVAILD)
                return ;
			test_AlignmentHorizontalShift(panel_ctrl_port);
			break;

		case '1':
            panel_ctrl_port = test_InputSelection(key);
            if(panel_ctrl_port == VC_PANEL_CTRL_INVAILD)
                return ;
			test_AlignmentVerticalShift(panel_ctrl_port);
			break;

		case '2':
//			test_AlignmentShowShiftRegValue();
			break;

		case '3':
            panel_ctrl_port = test_InputSelection(key);
            if(panel_ctrl_port == VC_PANEL_CTRL_INVAILD)
                return ;
			rtiVC_DownloadPixelShiftPatchLUT(panel_ctrl_port);
			break;

		case '4':
			test_DumpRdcPatchRegister();			
			break;

		case '5':
#ifdef _CFG_RDP_ATTACHED_TO_PORT_0		
			test_DumpPanelPatchRegister(VC_PANEL_CTRL_PORT_0);
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1	
			test_DumpPanelPatchRegister(VC_PANEL_CTRL_PORT_1);
#endif
			break;

		case 'a':
			rtiVC_EraseSectorFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE);
			break;

		case 'b':
			rtiVC_EraseSectorFLASH(FLASH_PART_OFFSET_PANEL_0_PATCH_BASE);
			break;

		case 'c':
			rtiVC_EraseSectorFLASH(FLASH_PART_OFFSET_PANEL_1_PATCH_BASE);
			break;

		case 'd':
#if 0
			while (1) {
				DMSG("[Manual] Tune RDC patch LUT\n");
	            DMSG("Input Addr(hex) Data(hex) : ");
				scanf("%x %x" , &reg_addr, &reg_val);
				CLEAR_STDIN;
#endif

				if (reg_addr <= 0xFFFF)
					break;
				else
					DMSG("Invalid register address\n");
#if 0
			}
#endif

			rtiVC_TuneManualRdcPatch(reg_addr, reg_val);
			break;

		case 'e':
			rtiVC_DownloadRdcManualPatchLUT();
			break;

		case 'f':
			rtiVC_ResetManualRdcPatch();
			break;

		case 'g':
            panel_ctrl_port = test_InputSelection(key);
            if(panel_ctrl_port == VC_PANEL_CTRL_INVAILD)
                return ;
#if 0
			while (1) {
				DMSG("[Manual] Tune Panel patch LUT\n");		
	            DMSG("Input Addr(hex) Data(hex) : ");
				scanf("%x %x" , &reg_addr, &reg_val);
				CLEAR_STDIN;
#endif

				if (reg_addr <= 0xFFFF)
					break;
				else
					DMSG("Invalid register address\n");
#if 0
			}
#endif

			rtiVC_TuneManualPanelPatch(panel_ctrl_port, reg_addr, reg_val);
			break;

		case 'h':
            panel_ctrl_port = test_InputSelection(key);
            if(panel_ctrl_port == VC_PANEL_CTRL_INVAILD)
                return ;
			rtiVC_DownloadPanelManualPatchLUT(panel_ctrl_port);
			break;

		case 'i':
			rtiVC_ResetManualPanelPatch();
			break;

		case 'q':
		case 'Q':
			goto quit_test;
		}
#if 0
	}
#endif

quit_test:

	return;	
}

void test_ShiftVerticalPixel(char ch)
{
#if 0
	char ch;
#endif
	UINT_T ctrl_port = 0x0;

#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	ctrl_port = VC_PANEL_CTRL_PORT_0;
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	ctrl_port |= VC_PANEL_CTRL_PORT_1;
#endif

	DMSG("+ :Up, -: Down, q(Quit)\n");

#if 0
	while (1) {
		ch = _getch();
#endif

		if (ch == 'q' || ch == 'Q')
			return;

		if (ch == '+') {
			DMSG("Up pressed\n");
			rtiVC_ShiftVerticalPixel((E_VC_PANEL_CTRL_PORT_T)ctrl_port, VC_SHIFT_VERTICAL_UP);
		}
		else if (ch == '-') {
			DMSG("Down pressed\n");
			rtiVC_ShiftVerticalPixel((E_VC_PANEL_CTRL_PORT_T)ctrl_port, VC_SHIFT_VERTICAL_DOWN);
		}
#if 0
	}
#endif
}

void test_ShiftHorizontalPixel(char ch)
{
#if 0
	char ch;
#endif
	UINT_T ctrl_port = 0x0;

#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	ctrl_port = VC_PANEL_CTRL_PORT_0;
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	ctrl_port |= VC_PANEL_CTRL_PORT_1;
#endif

	DMSG("+ :Right, - :Left, q(Quit)\n");

#if 0
	while (1) {
		ch = _getch();
#endif

		if (ch == 'q' || ch == 'Q')
			return;

		if (ch == '+') {
			DMSG("Right pressed\n");
			rtiVC_ShiftHorizontalPixel((E_VC_PANEL_CTRL_PORT_T)ctrl_port, VC_SHIFT_HORIZONTAL_RIGHT);
		}
		else if (ch == '-') {
			DMSG("Left pressed\n");
			rtiVC_ShiftHorizontalPixel((E_VC_PANEL_CTRL_PORT_T)ctrl_port, VC_SHIFT_HORIZONTAL_LEFT);
		}
#if 0
	}
#endif
}

static BOOL_T is_string_hex(char *s)
{
	char ch;

	while (1) {
		ch = toupper(*s++);
		if (ch == 0) /* NULL char */
			break;

		if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F'))
			return FALSE;
	}

	return TRUE;
}

static int get_rgb_value_from_user(void)
{
	int color_val;
	char read_buf[100], *token, *stopstring;
	size_t str_len;

#if 0
	while (1) {
#endif
		DMSG("Input 24-bit RGB hex value (RRGGBB), q(Quit): 0x");
#if 0
		fgets(read_buf, 100, stdin);
#endif

		token = strtok(read_buf, " \n\r");
		if (token == NULL)
			return -1;
//			continue;

		if (token[0] == 'q' || token[0] == 'Q')
			return -50;

		str_len = strlen(token);
		if (str_len != 6) {
			EMSG("Invalid RGB length! NOT 6 characters!\n");
			return -1;
//			continue;
		}

		if (is_string_hex(token) == TRUE) {
			color_val = (int)strtol(token, &stopstring, 16);
			return color_val;
		}
		else {
			EMSG("Invalid RGB value! NOT hex value!\n");
		}
#if 0
	}
#endif

	return -1;
}

static int get_rgb_1color_value_from_user(void)
{
	int color_val;
	char read_buf[100], *token, *stopstring;
	size_t str_len;

#if 0
	while (1) {
#endif
		DMSG("Input 8-bit hex value , q(Quit): 0x");
#if 0
		fgets(read_buf, 100, stdin);
#endif

		token = strtok(read_buf, " \n\r");
		if (token == NULL)
			return -1;
//			continue;

		if (token[0] == 'q' || token[0] == 'Q')
			return -50;

		str_len = strlen(token);
		if (str_len != 2) {
			EMSG("Invalid length! NOT 2 hex!\n");
			return -1;
//			continue;
		}

		if (is_string_hex(token) == TRUE) {
			color_val = (int)strtol(token, &stopstring, 16);
			return color_val;
		}
		else {
			EMSG("Invalid value! NOT hex value!\n");
		}
#if 0
	}
#endif

	return -1;
}

static int get_brightness_value_from_user(void)
{
	int color_val;
	char read_buf[100], *token, *stopstring;

#if 0
	while (1) {
#endif
		DMSG("Input decimal value (0 ~ 255) , q(Quit): ");
#if 0
		fgets(read_buf, 100, stdin);
#endif

		token = strtok(read_buf, " \n\r");
		if (token == NULL)
			return -2;
//			continue;

		if (token[0] == 'q' || token[0] == 'Q')
			return -50;

		if (is_string_digit(token) == TRUE) {
			color_val = (int)strtol(token, &stopstring, 10);
			if ((color_val < 0) || (color_val > 255)) {
				EMSG("Invalid range!\n");
				return -2;
//				continue;
			}
			
			return color_val;
		}
		else {
			EMSG("Invalid value! NOT decimal value!\n");
		}
#if 0
	}
#endif

	return -2;
}

static void print_led_level(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							E_VC_LED_CTRL_COLOR_T color,
							VC_LED_CURRENT_INFO_T curr[MAX_NUM_VC_PANEL_PORT])
{
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		if (color & VC_LED_CTRL_COLOR_R)
			DMSG("Port 0: R LED current: %d\n", curr[0].R);

		if (color & VC_LED_CTRL_COLOR_G)
			DMSG("Port 0: G LED current: %d\n", curr[0].G);

		if (color & VC_LED_CTRL_COLOR_B)
			DMSG("Port 0: B LED current: %d\n", curr[0].B);
	}
	
	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		if (color & VC_LED_CTRL_COLOR_R)
			DMSG("Port 1: R LED current: %d\n", curr[1].R);

		if (color & VC_LED_CTRL_COLOR_G)
			DMSG("Port 1: G LED current: %d\n", curr[1].G);

		if (color & VC_LED_CTRL_COLOR_B)
			DMSG("Port 1: B LED current: %d\n", curr[1].B);
	}
}

#define LED_CURRENT_STEP		1

void test_LedCurrentControl(int key, char ch)
{
#if 0
	char ch;
	int ret, key;
#endif
	int ret;
	E_VC_PANEL_CTRL_PORT_T ctrl_port;
	U8_T min_level, max_level;
	VC_LED_CURRENT_INFO_T curr[MAX_NUM_VC_PANEL_PORT];
    
	ctrl_port = test_InputSelection(key);
	if(ctrl_port == VC_PANEL_CTRL_INVAILD)
		return;
	
	rtiVC_GetLedLevelRange(&min_level, &max_level);
	DMSG("LED current level range: %d ~ %d\n", min_level, max_level);

	rtiVC_GetLedCurrent(ctrl_port, curr);
	print_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);

/// Level Test
#if 0
	curr[0].R = max_level - 1;
	curr[0].G = max_level - 1;
	curr[0].B = max_level - 1;

	curr[1].R = max_level - 1;
	curr[1].G = max_level - 1;
	curr[1].B = max_level - 1;
	rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);
#endif
/////////

#if 0
	while(1) {
#endif
		DMSG("================ LED current control ===============\n");
		DMSG("\tr or R: Red\n");
		DMSG("\tg or G: Blue\n");
		DMSG("\tb or B: Green\n");\
		DMSG("\ta or A: All(RGB)\n");
		DMSG("\tx or X: All(RGB) Maximum\n");
		DMSG("\tn or N: All(RGB) Minimum\n");
#if defined(CFG_MDC_RDC200)		
        DMSG("\t3: [Patch] Download LED current LUT into FLASH\n");
#endif        
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");
		
#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case 'r':
		case 'R':			
			while (1) {
				DMSG("[R] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].R += LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].R += LED_CURRENT_STEP;
					
					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_R);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].R -= LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].R -= LED_CURRENT_STEP;

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_R);
				}
				if (ret == 0)
					print_led_level(ctrl_port, VC_LED_CTRL_COLOR_R, curr);
			}
			break;

		case 'g':
		case 'G':			
#if 0
			while (1) {
#endif
				DMSG("[G] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].G += LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].G += LED_CURRENT_STEP;

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_G);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].G -= LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].G -= LED_CURRENT_STEP;

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_G);
				}
				if (ret == 0)
					print_led_level(ctrl_port, VC_LED_CTRL_COLOR_G, curr);
#if 0
			}
#endif
			break;

		case 'b':
		case 'B':			
#if 0
			while (1) {
#endif
				DMSG("[B] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].B += LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].B += LED_CURRENT_STEP;

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_B);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0)
						curr[0].B -= LED_CURRENT_STEP;

					if (ctrl_port & VC_PANEL_CTRL_PORT_1)
						curr[1].B -= LED_CURRENT_STEP;

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_B);
				}
				if (ret == 0)
					print_led_level(ctrl_port, VC_LED_CTRL_COLOR_B, curr);
#if 0
			}
#endif
			break;

		case 'a':
		case 'A':			
#if 0
			while (1) {
#endif
				DMSG("[All] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
						curr[0].R += LED_CURRENT_STEP;
						curr[0].G += LED_CURRENT_STEP;
						curr[0].B += LED_CURRENT_STEP;
					}

					if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
						curr[1].R += LED_CURRENT_STEP;
						curr[1].G += LED_CURRENT_STEP;
						curr[1].B += LED_CURRENT_STEP;
					}					

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");

					if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
						curr[0].R -= LED_CURRENT_STEP;
						curr[0].G -= LED_CURRENT_STEP;
						curr[0].B -= LED_CURRENT_STEP;
					}

					if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
						curr[1].R -= LED_CURRENT_STEP;
						curr[1].G -= LED_CURRENT_STEP;
						curr[1].B -= LED_CURRENT_STEP;
					}

					ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);
				}

				if (ret == 0)
					print_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
#if 0
			}
#endif
			break;

		case 'x':
		case 'X':
			if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
				curr[0].R = max_level;
				curr[0].G = max_level;
				curr[0].B = max_level;
			}

			if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
				curr[1].R = max_level;
				curr[1].G = max_level;
				curr[1].B = max_level;
			}

			ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);

			print_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
			break;

		case 'n':
		case 'N':
			if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
				curr[0].R = min_level;
				curr[0].G = min_level;
				curr[0].B = min_level;
			}

			if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
				curr[1].R = min_level;
				curr[1].G = min_level;
				curr[1].B = min_level;
			}

			ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);

			print_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
			break;

#if defined(CFG_MDC_RDC200)
        case '3':
            rtiVC_DownloadLedCurrentPatchLUT(ctrl_port);
        break;
#endif

		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("unknown -[%c]\n", key);
		}
#if 0
	}
#endif

TEST_TP_EXIT:

	return;
}

#if defined(CFG_MDC_RDC200A)
static void dump_rdc200a_led_timing(UINT_T ctrl_port)
{
	U16_T val;
	U8_T led_lsb, led_msb;

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		led_msb = RDC_REG_GET(LED_P0_ON_R_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_ON_R_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_on_red=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_R_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_OFF_R_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_off_red=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P1_ON_R_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_ON_R_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_on_red=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_R_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_OFF_R_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_off_red=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P0_ON_G_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_ON_G_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_on_green=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_G_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_OFF_G_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_off_green=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P1_ON_G_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_ON_G_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_on_green=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_G_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_OFF_G_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_off_green=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P0_ON_B_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_ON_B_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_on_blue=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_B_msb(0));
		led_lsb = RDC_REG_GET(LED_P0_OFF_B_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p0_off_blue=0x%04X\n\n", val);
		    
		led_msb = RDC_REG_GET(LED_P1_ON_B_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_ON_B_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_on_blue=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_B_msb(0));
		led_lsb = RDC_REG_GET(LED_P1_OFF_B_lsb(0));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx0_led_p1_off_blue=0x%04X\n\n", val);			
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		led_msb = RDC_REG_GET(LED_P0_ON_R_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_ON_R_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_on_red=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_R_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_OFF_R_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_off_red=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P1_ON_R_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_ON_R_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_on_red=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_R_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_OFF_R_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_off_red=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P0_ON_G_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_ON_G_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_on_green=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_G_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_OFF_G_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_off_green=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P1_ON_G_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_ON_G_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_on_green=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_G_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_OFF_G_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_off_green=0x%04X\n\n", val);

		led_msb = RDC_REG_GET(LED_P0_ON_B_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_ON_B_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_on_blue=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P0_OFF_B_msb(1));
		led_lsb = RDC_REG_GET(LED_P0_OFF_B_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p0_off_blue=0x%04X\n\n", val);
		    
		led_msb = RDC_REG_GET(LED_P1_ON_B_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_ON_B_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_on_blue=0x%04X\n", val);

		led_msb = RDC_REG_GET(LED_P1_OFF_B_msb(1));
		led_lsb = RDC_REG_GET(LED_P1_OFF_B_lsb(1));
		val = ((U16_T)led_msb<<8) | led_lsb;
		DMSG("tx1_led_p1_off_blue=0x%04X\n\n", val);		
	}
}
#endif // #if defined(CFG_MDC_RDC200A)

void test_LedDutyControl(int key, char ch)
{
#if 1
#if 0
	char ch;
	int ret, key;
#endif
	int ret;
	VC_LED_DUTY_INFO_T duty[MAX_NUM_VC_PANEL_PORT];
	UINT_T ctrl_port = 0x0; // temp

    ctrl_port = test_InputSelection(key);
    if(ctrl_port == VC_PANEL_CTRL_INVAILD)
        return ;

	rtiVC_GetLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty);

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		DMSG("P0: R LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].R));
		DMSG("P0: G LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].G));
		DMSG("P0: B LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].B));
	}
	
	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		DMSG("P1: R LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].R));
		DMSG("P1: G LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].G));
		DMSG("P1: B LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].B));
	}

	while(1) {
		DMSG("================ LED duty control ===============\n");
		DMSG("\tr or R: Red\n");
		DMSG("\tg or G: Blue\n");
		DMSG("\tb or B: Green\n");
		DMSG("\ta or A: All(RGB)\n");
        DMSG("\t3: [LED duty] Download LED duty LUT into FLASH\n");
#if defined(CFG_MDC_RDC200A)		
		DMSG("\t4: [LED duty] Dump register (Can be use in firmware configuration file)\n");
#endif
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");
		
#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case 'r':
		case 'R':			
#if 0
			while (1) {
#endif
				DMSG("[R] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");
					ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_R);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");
					ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_R);
				}
                if (ret == 0) {
                    if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                        DMSG("P0: R duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].R));
                    if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                        DMSG("P1: R duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].R));
                }
#if 0
			}
#endif
			break;

		case 'g':
		case 'G':
#if 0
			while (1) {
#endif
				DMSG("[G] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");
					ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_G);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");
					ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_G);
				}
                if (ret == 0) {
                    if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                        DMSG("P0: G duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].G));
                    if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                        DMSG("P0: G duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].G));
                }

#if 0
			}
#endif
			break;

		case 'b':
		case 'B':
#if 0
			while (1) {
#endif
				DMSG("[B] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");
					ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_B);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");
					ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_B);
				}
                if (ret == 0) {
                    if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                        DMSG("P0: B duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].B));
                    if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                        DMSG("P1: B duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].B));
                }
#if 0
			}
#endif
			break;

		case 'a':
		case 'A':
#if 0
			while (1) {
#endif
				DMSG("[All] + :Up, -: Down, q(Quit)\n");
#if 0
				ch = _getch();
#endif
				if (ch == 'q' || ch == 'Q')
					break;

				ret = -1;
				if (ch == '+') {
					DMSG("Up pressed\n");
					ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_ALL);					
				}
				else if (ch == '-') {
					DMSG("Down pressed\n");
					ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_ALL);
				}

				if (ret == 0) {
					if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
						DMSG("P0: R duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].R));
						DMSG("P0: G duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].G));
						DMSG("P0: B duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].B));
					}

					if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
						DMSG("P1: R duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].R));
						DMSG("P1: G duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].G));
						DMSG("P1: B duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[1].B));
					}
				}
#if 0
			}
#endif
			break;

        case '3':
            rtiVC_DownloadLedDutyPatchLUT();
        break;

#if defined(CFG_MDC_RDC200A)
		case '4':
			dump_rdc200a_led_timing(ctrl_port);
			break;
#endif // #if defined(CFG_MDC_RDC200A)

		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("unknown - [%c]\n", key);
		}
	}

TEST_TP_EXIT:

	return;
#else
	char read_buf[100], *token;
	U8_T percentage;
	U8_T min_led_duty, max_led_duty;
	
	rtiVC_GetLedDutyRange(&min_led_duty, &max_led_duty);

	while (1) {
		DMSG("Input LED duty (%d ~ %d)%%, q(Quit): ", min_led_duty, max_led_duty);
		fgets(read_buf, 100, stdin);

		token = strtok(read_buf, " \n\r");
		if (token == NULL)
			continue;

		if (token[0] == 'q' || token[0] == 'Q')
			return;

		if (is_string_digit(token) == TRUE) {
			percentage = (U8_T)atoi(token);

			if ((percentage < min_led_duty) || (percentage > max_led_duty)) {
				EMSG("Invaild LED duty range!\n");
			}
			else {
				rtiVC_SetLedDuty(percentage);
			}
		}
		else {
			EMSG("Invalid LED duty value!\n");
		}
	}
#endif
}

void test_FrameRateControl(char ch)
{
#if 0
	char ch;
#endif
	int curr_frame_rate;
	int applied_frame_rate;
	double frame_rate_hz;

	curr_frame_rate = rtiVC_GetFrameRate();
	frame_rate_hz = VC_FRAME_RATE_TO_HZ(curr_frame_rate);
	DMSG("Current Frame Rate: %.3f Hz\n", frame_rate_hz);
	
	DMSG("+ :Up, -: Down, q(Quit)\n");

#if 0
	while (1) {
		ch = _getch();
#endif

		if (ch == 'q' || ch == 'Q')
			return;

		if (ch == '+') {
			DMSG("Up pressed\n");
			applied_frame_rate = rtiVC_StepUpFrameRate();
			if (applied_frame_rate > 0) {
				frame_rate_hz = VC_FRAME_RATE_TO_HZ(applied_frame_rate);
				DMSG("Changed Frame Rate: %.3f Hz\n", frame_rate_hz);
			}
		}
		else if (ch == '-') {
			DMSG("Down pressed\n");
			applied_frame_rate = rtiVC_StepDownFrameRate();
			if (applied_frame_rate > 0) {
				frame_rate_hz = VC_FRAME_RATE_TO_HZ(applied_frame_rate);
				DMSG("Changed Frame Rate: %.3f Hz\n", frame_rate_hz);
			}
		}
#if 0
	}
#endif
}

void test_TestPattern(const char *rdc_name, int key)
{
//	int key, ret;
	int ret;
	U8_T R, G, B;

#if 0
	while(1) {
#endif
		DMSG("================ [%s] Test Pattern ===============\n", rdc_name);
		DMSG("\t0: Enable\n");
		DMSG("\t1: Disable\n");
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
			DMSG("Enable\n");
			ret = get_rgb_value_from_user();			
			if (ret >= 0) {
				R = (ret >> 16) & 0xFF;
				G = (ret >> 8) & 0xFF;
				B = ret & 0xFF;

				ret = rtiVC_EnableTestPattern(TRUE);
				if (ret == 0) {
					rtiVC_GenerateTestPattern(R, G, B);
					DMSG("Enabled\n");
				}
			}
			break;

		case '1':
			rtiVC_EnableTestPattern(FALSE);
			DMSG("Disabled\n");
			break;
	
		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_TP_EXIT:
	//rtiVC_EnableTestPattern(FALSE);

	return;
}

void test_Brightness(char ch)
{
#if 0
	char ch;
#endif
	
	DMSG("+ :Increase, - :Decrease, q(Quit)\n");

#if 0
	while (1) {
		ch = _getch();
#endif

		if (ch == 'q' || ch == 'Q')
			return;

		if (ch == '+') {
			DMSG("Increase pressed\n");
			rtiVC_IncreaseBrightness();
		}
		else if (ch == '-') {
			DMSG("Decrease pressed\n");
			rtiVC_DecreaseBrightness();
		}
#if 0
	}
#endif
}

void test_RequestLEDcontrol(int key) // using Interrupt
{
#if defined(CFG_PANEL_RDP502H) || defined(CFG_PANEL_RDP551F)
//	int key, ret, color;
	int ret, color;
	VC_LED_CTRL_INFO_T led_ctrl_info;
	U8_T brightness;
	E_VC_PANEL_CTRL_PORT_T ctrl_port;
	E_VC_RDC_INDEX_T rdc_idx;

	rdc_idx = VC_RDC_INDEX_PRIMARY; // temp! Assume that 1 RDC existed
    
	ctrl_port = test_InputSelection(key);
	if(ctrl_port == VC_PANEL_CTRL_INVAILD)
		return;

#if 0
	while(1) {
#endif
		DMSG("================ Test Request LED control ==============\n");
		DMSG("\t0: Brightness mode\n");
		DMSG("\t1: Color mode: Red\n");
		DMSG("\t2: Color mode: Green\n");
		DMSG("\t3: Color mode: Blue\n");
		DMSG("\t4: Color mode: All\n");		
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		switch (key) {
		case '0':
			DMSG("Brightness mode\n");
			color = get_brightness_value_from_user();			
			if (color >= 0) {
				brightness = color;

				led_ctrl_info.mode = VC_LED_CTRL_MODE_BRIGHTNESS;
				led_ctrl_info.param.brightness = brightness;

				ret = rtiVC_ReqeustLedControl(rdc_idx, ctrl_port, &led_ctrl_info);

				VC_M_DELAY(2);
				rtiVC_ConfirmLedControl(rdc_idx);

				//rtiVC_ConfirmLedControl(VC_RDC_INDEX_SECONDARY); // if present

				DMSG("Result: %d\n", ret);
			}
			break;


		case '1':
			DMSG("Color mode: Red\n");
			color = get_rgb_1color_value_from_user();			
			if (color >= 0) {
				led_ctrl_info.mode = VC_LED_CTRL_MODE_COLOR;
				led_ctrl_info.param.color.R = color;
				led_ctrl_info.param.color.ctrl = VC_LED_CTRL_COLOR_R;

				ret = rtiVC_ReqeustLedControl(rdc_idx, ctrl_port, &led_ctrl_info);
				VC_M_DELAY(2);
				rtiVC_ConfirmLedControl(rdc_idx);				
				DMSG("Result: %d\n", ret);
			}
			break;

		case '2':
			DMSG("Color mode: Green\n");
			color = get_rgb_1color_value_from_user();			
			if (color >= 0) {
				led_ctrl_info.mode = VC_LED_CTRL_MODE_COLOR;
				led_ctrl_info.param.color.G = color;
				led_ctrl_info.param.color.ctrl = VC_LED_CTRL_COLOR_G;

				ret = rtiVC_ReqeustLedControl(rdc_idx, ctrl_port, &led_ctrl_info);
				VC_M_DELAY(2);
				rtiVC_ConfirmLedControl(rdc_idx);				
				DMSG("Result: %d\n", ret);
			}
			break;

		case '3':
			DMSG("Color mode: Blue\n");
			color = get_rgb_1color_value_from_user();			
			if (color >= 0) {
				led_ctrl_info.mode = VC_LED_CTRL_MODE_COLOR;
				led_ctrl_info.param.color.B = color;
				led_ctrl_info.param.color.ctrl = VC_LED_CTRL_COLOR_B;

				ret = rtiVC_ReqeustLedControl(rdc_idx, ctrl_port, &led_ctrl_info);
				VC_M_DELAY(2);
				rtiVC_ConfirmLedControl(rdc_idx);				
				DMSG("Result: %d\n", ret);
			}
			break;

		case '4':
			DMSG("Color mode: All\n");
			color = get_rgb_value_from_user();			
			if (color >= 0) {
				led_ctrl_info.mode = VC_LED_CTRL_MODE_COLOR;
				led_ctrl_info.param.color.R = (color >> 16) & 0xFF;
				led_ctrl_info.param.color.G = (color >> 8) & 0x000000ff;
				led_ctrl_info.param.color.B = color & 0x000000ff;
				led_ctrl_info.param.color.ctrl = VC_LED_CTRL_COLOR_ALL;

				ret = rtiVC_ReqeustLedControl(rdc_idx, ctrl_port, &led_ctrl_info);
				VC_M_DELAY(2);
				rtiVC_ConfirmLedControl(rdc_idx);				
				DMSG("Result: %d\n", ret);
			}
			break;

		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_TP_EXIT:
	//rtiVC_EnableTestPattern(FALSE);

	return;

#else
	EMSG("Unsupported!\n");
#endif
}

void test_ImageDisplayFormat(int key, unsigned int swap_image_3d_mode)
{
//	int key;
//	unsigned int swap_image_3d_mode = 0;

#if 0
	while(1) {
#endif
		DMSG("================ Image display format ===============\n");
		DMSG("\t0: 2D\n");
		DMSG("\t1: 3D (Side By Side)\n");
#if defined(CFG_MDC_RDC200A)
		DMSG("\t2: 3D (Top and Bottom)\n");
		DMSG("\t3: 3D (Lossless Side by Side)\n");
		DMSG("\t4: 3D (Frame By Frame)\n");
		DMSG("\t5: 3D (Select pi_rsvd1 pad for Frame by frame indicator)\n");
#endif
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
			DMSG("2D\n");
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_2D, 0);
			break;

		case '1':
			DMSG("3D (Side By Side)\n");
#if defined(CFG_MDC_RDC200A)
#if 0
			while(1) {
				DMSG("Swap select (0: None, 1: Swap) : ");
				scanf("%u" , &swap_image_3d_mode);
                CLEAR_STDIN;
#endif
				if ((swap_image_3d_mode != 0) && (swap_image_3d_mode != 1)) {
					EMSG("Invalid swap selection!\n");
					break;
				}
//					break;

//				EMSG("Invalid swap selection!\n");
#if 0
			}
#endif
#endif
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_3D_SBS,
										(BOOL_T)swap_image_3d_mode);
			break;

#if defined(CFG_MDC_RDC200A)
		case '2':
			DMSG("3D (Top and Bottom)\n");
#if 0
			while(1) {
				DMSG("Swap select (0: None, 1: Swap) : ");
				scanf("%u" , &swap_image_3d_mode);
                CLEAR_STDIN;
#endif
				if ((swap_image_3d_mode == 0) || (swap_image_3d_mode == 1))
					break;

				EMSG("Invalid swap selection!\n");
#if 0
			}
#endif
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_3D_TAB,
										(BOOL_T)swap_image_3d_mode);
			break;

		case '3':
			DMSG("3D (Lossless Side by Side)\n");
#if 0
			while(1) {
				DMSG("Swap select (0: None, 1: Swap) : ");
				scanf("%u" , &swap_image_3d_mode);
                CLEAR_STDIN;
#endif
				if ((swap_image_3d_mode == 0) || (swap_image_3d_mode == 1))
					break;

				EMSG("Invalid swap selection!\n");
#if 0
			}
#endif
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_3D_SBS_LL,
										(BOOL_T)swap_image_3d_mode);
			break;

		case '4':
			DMSG("3D (Frame By Frame)\n");
#if 0
			while(1) {
				DMSG("Swap select (0: None, 1: Swap) : ");
				scanf("%u" , &swap_image_3d_mode);
                CLEAR_STDIN;
#endif
				if ((swap_image_3d_mode == 0) || (swap_image_3d_mode == 1))
					break;

				EMSG("Invalid swap selection!\n");
#if 0
			}
#endif
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_3D_FBF,
										(BOOL_T)swap_image_3d_mode);
			break;

		case '5':
			DMSG("3D (Select pi_rsvd1 pad for Frame by frame indicator)\n");
#if 0
			while(1) {
				DMSG("Swap select (0: None, 1: Swap) : ");
				scanf("%u" , &swap_image_3d_mode);
                CLEAR_STDIN;
#endif
				if ((swap_image_3d_mode == 0) || (swap_image_3d_mode == 1))
					break;

				EMSG("Invalid swap selection!\n");
#if 0
			}
#endif
			rtiVC_SetImageDisplayFormat(VC_IMAGE_DISPLAY_FORMAT_3D_FBF_EX_SIG,
										(BOOL_T)swap_image_3d_mode);
			break;			
#endif
	
		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_TP_EXIT:

	return;
}

void test_ChangeI2cSlaveAddressRDC(int key)
{
	int slave_addr, bus_num, ch;
	char read_buf[100], *token, *stopstring;
	size_t str_len;
//	int key;

#if 0
	while (1) {
#endif
		DMSG("================ [RDC I2C] Slave address/Bus number ===============\n");
		DMSG("\t0: Change I2C slave address\n");
		DMSG("\t1: Change I2C bus number\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");		

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		switch (key) {
		case '0':
RETRY_SLAVE_ADDR:		
			DMSG("Input RDC I2C slave address(Hex value): 0x");
#if 0
			fgets(read_buf, 100, stdin);
#endif

			token = strtok(read_buf, " \n\r");
			if (token == NULL)
				goto RETRY_SLAVE_ADDR;

			str_len = strlen(token);
			if (str_len != 2) {
				EMSG("Invalid input length! NOT 2 characters!\n");
				goto RETRY_SLAVE_ADDR;
			}

			if (is_string_hex(token) == TRUE) {			
				slave_addr = (int)strtol(token, &stopstring, 16);
				rtiVC_SetSlaveAddressRDC(slave_addr);
			}
			else {
				EMSG("Invalid input value value! NOT hex value!\n");
				goto RETRY_SLAVE_ADDR;
			}
			break;

		case '1':
RETRY_CH:
			DMSG("Input RDC channel(0 or 1): ");
#if 0
			fgets(read_buf, 100, stdin);
#endif
			token = strtok(read_buf, " \n\r");
			if (token == NULL)
				goto RETRY_CH;

			if (is_string_digit(token) == TRUE) { 		
				ch = (int)strtol(token, &stopstring, 10);
				if ((ch < 0) || (ch > 1)) {
					EMSG("Invalid channel! Must be 0 or 1\n");
					goto RETRY_CH;
				}

			}
			else {
				EMSG("Invalid input value value! NOT decimal value!\n");
				goto RETRY_CH;
			}				

RETRY_BUS_NUM:
			DMSG("Input RDC I2C bus number(Decimal value): ");
#if 0
			fgets(read_buf, 100, stdin);
#endif
			token = strtok(read_buf, " \n\r");
			if (token == NULL)
				goto RETRY_BUS_NUM;

			if (is_string_digit(token) == TRUE) { 		
				bus_num = (int)strtol(token, &stopstring, 10);
				rtiVC_ChangeI2CBusNumber((E_VC_DEVICE_CH_T)ch, bus_num);
			}
			else {
				EMSG("Invalid input value value! NOT decimal value!\n");
				goto RETRY_BUS_NUM;
			}
			break;

		case 'q':
		case 'Q':
			goto CHG_EXIT;

		default:
			DMSG("[%c]\n", key);			
		}
#if 0
	}
#endif

CHG_EXIT:
	return;
}

void test_PanelPowerOnOff(int key)
{
//	int key;

#if 0
	while(1) {
#endif
		DMSG("================ Panel Power On/Off ===============\n");
		DMSG("\t0: Power Off\n");
		DMSG("\t1: Power On\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case '0':
			DMSG("Power Off\n");
			rtiVC_PowerOnPanel(FALSE);
			break;

		case '1':
			DMSG("Power On\n");
			rtiVC_PowerOnPanel(TRUE);
			break;
	
		case 'q':
		case 'Q':
			goto TEST_TP_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_TP_EXIT:
	//rtiVC_EnableTestPattern(FALSE);

	return;
}

#define MAX_FLASH_DUMP_SIZE		(4* 1024*1024) // 4 MB bytes
#define FLASH_DUMP_UINT_SIZE	256 // bytes

static void make_flash_dump_filename(char *filename_buf)
{
	struct tm *t;
	time_t timer;

	timer = time(NULL);
	t = localtime(&timer);

#if defined(CFG_MDC_RDC200)
    sprintf(filename_buf, "%s%crdc200_flash_dump_%04d%02d%02d_%02d%02d%02d.bin",
    vc_GetLutDirectoryPath(),PathSeparator,
    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
	
#elif defined(CFG_MDC_RDC200A)
    sprintf(filename_buf, "%s%crdc200a_flash_dump_%04d%02d%02d_%02d%02d%02d.bin",
    vc_GetLutDirectoryPath(),PathSeparator,
    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);

#else
	#error "Code not present"
#endif
}

static int input_flash_dump_parameters(U32_T *addr, UINT_T *read_size, int key)
{
//	int key;
	char read_buf[128];

	DMSG("Input FLASH dump parameters. Hex or q/r\n");
RETYPE_DUMP_PARAM:
	DMSG("Input the base address of FLASH to be read. q(Quit), r(Retry)\n");
	DMSG("> ");
#if 0
	fgets(read_buf, 128, stdin);
#endif
	if (read_buf[0] == 'q' || read_buf[0] == 'Q')
		return -1000;
	
	if (read_buf[0] == 'r' || read_buf[0] == 'R')
		goto RETYPE_DUMP_PARAM;
	
	sscanf(read_buf, "%X", addr);
	if (*addr >= MAX_FLASH_DUMP_SIZE) {
		EMSG("Invalid the read address! Must be less than 0x%X\n", MAX_FLASH_DUMP_SIZE);
		goto RETYPE_DUMP_PARAM;
	}

	DMSG("Input the read size. q(Quit), r(Retry)\n");
	DMSG("> ");
#if 0
	fgets(read_buf, 128, stdin);
#endif
	if (read_buf[0] == 'q' || read_buf[0] == 'Q')
		return -1000;
	
	if (read_buf[0] == 'r' || read_buf[0] == 'R')
		goto RETYPE_DUMP_PARAM;
	
	sscanf(read_buf, "%X", read_size);
	if ((*addr + *read_size) >= MAX_FLASH_DUMP_SIZE) {
		EMSG("Invalid the read size! Must be less than 0x%X\n", MAX_FLASH_DUMP_SIZE);
		goto RETYPE_DUMP_PARAM;
	}

	DMSG("------------------- Input parametes ---------------------------\n");
	DMSG("Address: 0x%X\n", *addr);
	DMSG("Size: 0x%X\n", *read_size);
#if 0
	while (1) {
#endif
		DMSG("Use this input parameters(y) or retype(n)?");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif

		if (key == 'y' || key == 'Y')
			return 0;
//			break;

		if (key == 'n' || key == 'N')
			goto RETYPE_DUMP_PARAM;
#if 0
	}
#endif

	return 0;
}

#define PROGRESSVALUE(a, b) (int)((double)((a) * 100 ) / (double)(b))

void test_DumpFlashData(void)
{
	int ret;
	U32_T addr;
	UINT_T read_size, dump_len, total_dump_size, dumped_size = 0;
	static char filename[1024];
	U8_T *buf = NULL, *buf_ptr;
	FILE *out = NULL;
	int progress_val;
    size_t numwritten;
    
	make_flash_dump_filename(filename);

#if 1
	// input: flash addr, length
	ret = input_flash_dump_parameters(&addr, &read_size, 'y');
	if (ret == -1000)
		return;
#else
	addr = 0;
	read_size = 0x100;
#endif

	buf = (U8_T *)malloc(read_size);
	if (buf == NULL) {
		EMSG("[FLASH] Dump memory allocation error\n");
		return;
	}

	out = fopen(filename, "wb");
	if (out == NULL) {
		EMSG("[FLASH] Can't open file: %s\n", filename);
		goto err2;
	}

	buf_ptr = buf;

	total_dump_size = read_size;
	DMSG("Dumping... 0%%\n");

	while (read_size > 0) {
		dump_len = MIN(read_size, FLASH_DUMP_UINT_SIZE);

		ret = rtiVC_ReadFLASH(addr, buf_ptr, dump_len);
		if (ret != 0) {
			goto err1;
		}

		numwritten = fwrite(buf_ptr, sizeof(char), dump_len, out);
		if (numwritten != dump_len) {
			EMSG("[FLASH] File write error\n");
			goto err1;
		}

		buf_ptr += dump_len;
		addr += dump_len; 
		read_size -= dump_len;
		dumped_size += dump_len;

		progress_val = PROGRESSVALUE(dumped_size, total_dump_size);
		DMSG("Dumping... %d%% (%d/%u bytes)\n",
			progress_val, dumped_size, total_dump_size);		
	}

	DMSG("Dump completed 100%% (%u/%u bytes)\n", dumped_size, total_dump_size);

err1:
	if (out)
		fclose(out);

err2:
	if (buf)
		free(buf);
}

void test_FLASH(const char *rdc_name, int key)
{
//	int key;

#if 0
	while (1) {
#endif
		DMSG("================ [%s] Test FLASH ===============\n", rdc_name);
		DMSG("\t0: Dump data\n");
		DMSG("\t1: Chip Erase\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case '0':
			DMSG("Dump data\n");
			test_DumpFlashData();
			break;

		case '1':
			DMSG("Chip Erase\n");
			rtiVC_EraseFlashChip();
			break;
	
		case 'q':
		case 'Q':
			goto TEST_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_EXIT:

	return;
}

void test_TemperatureSensorCalibration(const char *panel_name, int key)
{
//	int key, ret;
	int ret;
	VC_PANEL_TEMPERATURE_INFO_T tinfo[MAX_NUM_VC_PANEL_PORT];
	E_VC_PANEL_CTRL_PORT_T ctrl_port;
	float prev_tpe, curr_tpe;
	VC_PANEL_TEMPERATURE_INFO_T *t;

#if defined(_CFG_RDP_ATTACHED_TO_PORT_0) && !defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_0;
#elif !defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_1;
#elif defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	ctrl_port = VC_PANEL_CTRL_PORT_ALL;
#else
	#error "Code not present"
#endif

#if 0
	while (1) {
#endif
		DMSG("\n======= [%s] Test Temperature Sensor =====\n", panel_name);
		DMSG("\t0: Get Temperature\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("=====================================================\n");
#if 0
		FFLUSH;
		key = getc(stdin);				
		CLEAR_STDIN;
#endif
		
		switch (key) {
		case '0':
			DMSG("Get Temperature\n");
			ret = rtiVC_GetTemperature(ctrl_port, tinfo);
			if (ret == 0) {
				if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
					t = &tinfo[0];
					prev_tpe = (float)t->prev_temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
					curr_tpe = (float)t->temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
						
					DMSG("P0: otp_applied(%d) prev[0x%03X, %.3f oC] => curr[0x%03X, %.3f oC]\n",
						t->otp_applied,
						t->prev_tsensor_code, prev_tpe,
						t->tsensor_code, curr_tpe);
				}

				if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
					t = &tinfo[1];
					prev_tpe = (float)t->prev_temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
					curr_tpe = (float)t->temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
						
					DMSG("P1: otp_applied(%d) prev[0x%03X, %.3f oC] => curr[0x%03X, %.3f oC]\n",
						t->otp_applied,
						t->prev_tsensor_code, prev_tpe,
						t->tsensor_code, curr_tpe);
				}
			}
			break;

		case 'q':
		case 'Q':
			goto TEST_EXIT;

		default:
			DMSG("[%c]\n", key);
		}
#if 0
	}
#endif

TEST_EXIT:

	return;
}


