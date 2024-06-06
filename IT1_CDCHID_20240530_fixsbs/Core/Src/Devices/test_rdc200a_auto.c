
#if !defined(__KERNEL__) && defined(__linux__) /* Linux application */ 
#include <termios.h>  
#endif

#include "rdc_prepare_dload.h"

#include "rdc_lut_patch.h"
#ifdef CFG_MDC_RDC200A
#include "test.h"
#include "rdc200a_flash_partitions.h"
#include "rti_vc_common.h"

#define ERROR_RDC200_AUTO_INTERFACE -1
#define ERROR_RDC200_AUTO_ARGUMENT -2


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
	#define RDC200A_LUT_FILE_PATH		"C:\\home\\vclut\\rdc200a"
	//#define RDC200A_LUT_FILE_PATH		"C:\\home\\vclut\\rdc200a\\RDP550F-F"
	//#define RDC200A_LUT_FILE_PATH		"D:\\vclut\\rdc200_gen_flash_data"
	//#define RDC200A_LUT_FILE_PATH		"D:\\vclut\\rdc200a\\RDP700Q"
	//#define RDC200A_LUT_FILE_PATH		"C:\\RDP700Q"
   
#elif defined(__APPLE__)
    #define RDC200A_LUT_FILE_PATH        "."
    //#define RDC200A_LUT_FILE_PATH        "C:\\home\\vclut\\rdc200a"
    //#define RDC200A_LUT_FILE_PATH        "/Users/Maverick/Deskto/tmp"

#endif


#ifndef SWAP16
#define SWAP16(a)                \
((((U16_T)(a) >> 8) & 0x00ff) |    \
(((U16_T)(a) << 8) & 0xff00))
#endif

#ifndef SWAP32
#define SWAP32(a)                    \
((((U32_T)(a) >> 24) & 0x000000ff) |   \
(((U32_T)(a) >>  8) & 0x0000ff00) |   \
(((U32_T)(a) <<  8) & 0x00ff0000) |   \
(((U32_T)(a) << 24) & 0xff000000))
#endif

int rtiVC_GenerateCoKOALuto(VC_COKOA_PARAM_EX_T *param, const char *filename);

static void test_auto_terminate(int val_exit)
{
    rtiVC_CloseDevice();
    rtiVC_Denitialize();
    
    exit(val_exit);
}

static int test_auto_interface_setup(const char * test_param[])
{
    int slave_addr, bus_num;

#if 0
    sscanf(test_param[0], "%d" , &bus_num);
	if(bus_num < 0 ) return -1;
    
	rtiVC_ChangeI2CBusNumber((E_VC_DEVICE_CH_T)0, bus_num); //Set bus number into Channel array number 1.
#endif

	sscanf(test_param[1], "%x" , &slave_addr);
    if(slave_addr < 0) return -1;
	rtiVC_SetSlaveAddressRDC(slave_addr);

	return 0;
}

static int test_auto_RegisterIO_RDP(const char *test_dev_name, const char * test_param[])
{
    unsigned int reg_addr, write_data;
    U8_T reg_val;
	reg_addr = 0x0550;
	write_data = 0;

    int panel_port = atoi(*test_param);
    test_param++;
    
    switch(*test_param[0])
    {
#ifdef CFG_PANEL_RDP501H
        case '0':

            sscanf(test_param[1], "%x" , &reg_addr);
            sscanf(test_param[2], "%x" , &write_data);
            
            if (panel_port == 0)
                rtiVC_WriteRDP501H_0(reg_addr, write_data);
            else
                rtiVC_WriteRDP501H_1(reg_addr, write_data);
            break;
#else
        case '0':

#if 0
        	sscanf(test_param[1], "%x" , &reg_addr);
#endif

        	reg_val = RDP_REG_GET(panel_port, reg_addr);
        	DMSG("[%s] BYTE_READ: addr(0x%02X), val(0x%02X)\n",
        			test_dev_name, reg_addr, reg_val);
            
        	test_auto_terminate(reg_val); //terminated with return value.
            break;
            
        case '1':
#if 0
        	sscanf(test_param[1], "%x" , &reg_addr);
        	sscanf(test_param[2], "%x" , &write_data);
#endif
            
        	RDP_REG_SET(panel_port, reg_addr, write_data);
            break;
#endif
        default:
            DMSG("[%c]\n", *test_param[0]);
    }
	return 0;
}

static int test_auto_RegisterIO_RDC(const char *test_dev_name, const char * argv[])
{
    int key=1;
    unsigned int i, reg_addr, read_cnt, write_data;
    U8_T reg_val, burst_buf[64], burst_wbuf[64];
    U16_T write_size = 0;
	reg_addr = 0x0550;
	write_data = 0;
    
    switch(*argv[0]) {
        case '0':
#if 0
            sscanf(argv[1], "%x" , &reg_addr);
#endif
            
            reg_val = RDC_REG_GET(reg_addr);
            DMSG("[RDC] S_READ: addr(0x%02X), val(0x%02X)\n", reg_addr, reg_val);
            
            test_auto_terminate(reg_val); //terminated with return value.
            break;
            
        case '1':
#if 0
            sscanf(argv[1], "%x" , &reg_addr);
            sscanf(argv[2], "%x" , &write_data);
#endif
            
            RDC_REG_SET(reg_addr, write_data);
            break;
            
        case '2':
            DMSG("[%s] Burst Read\n", "RDC");

#if 0
            sscanf(argv[1], "%x" , &reg_addr);
            sscanf(argv[2], "%x" , &read_cnt);
#endif
            
            if ((reg_addr + read_cnt) > 0xFF)
                EMSG("The count is out of register address\n");
            
            if ((reg_addr <= 0xFF) && ((reg_addr + read_cnt) <= 0xFF))
                break;

            burst_wbuf[0] = reg_addr;
            
            rtiVC_ReadBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr,
                                     burst_wbuf, 1,
                                     burst_buf, read_cnt);
            
            DMSG("Address\tValue\n");
            for (i = 0; i < read_cnt; i++)
                DMSG("0x%02X  \t 0x%02X\n", reg_addr+i, burst_buf[i]);
            break;
            
        case '3':
            DMSG("[%s] Burst Write\n", "RDC");

#if 0
            sscanf(argv[1], "%x" , &reg_addr);
#endif
            
            burst_buf[write_size++] = reg_addr;
            burst_buf[write_size++] = 0xAA;
            burst_buf[write_size++] = 0xBB;
            burst_buf[write_size++] = 0xCC;
            burst_buf[write_size++] = 0xDD;
            burst_buf[write_size++] = 0xEE;
            burst_buf[write_size++] = 0xA5;
            burst_buf[write_size++] = 0xBA;
            burst_buf[write_size++] = 0xCA;
            
            rtiVC_WriteBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr,
                                      burst_buf, write_size);
            break;
            
        default:
            DMSG("[%d]\n", key);
    }
    return 0;
}

static void print_auto_led_level(E_VC_PANEL_CTRL_PORT_T ctrl_port,
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

#define LED_CURRENT_STEP        1

int test_auto_LedCurrentControl(const char * argv[])
{
    char ch;
    int ret = -1;
    E_VC_PANEL_CTRL_PORT_T ctrl_port;
    U8_T min_level, max_level;
    VC_LED_CURRENT_INFO_T curr[MAX_NUM_VC_PANEL_PORT];
	
    ctrl_port = (E_VC_PANEL_CTRL_PORT_T)(atoi(*argv)+1);
    argv++;
    
    rtiVC_GetLedLevelRange(&min_level, &max_level);
    DMSG("LED current range: %d ~ %d\n", min_level, max_level);
    
    if((ret = rtiVC_GetLedCurrent(ctrl_port, curr)) < 0)
       return ret;
	
	if(argv[0] == NULL) {
		DMSG("unknown \n");
		return -1;
	}
    switch (*argv[0]) {
        case 'r':
        case 'R':
            sscanf(argv[1], "%c", &ch);
			if (ch == '+') {
				if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].R += LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].R += LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_R);
            }
            else if (ch == '-') {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].R -= LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].R -= LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_R);
            } else {
				DMSG("unknown - [%c]\n", ch);
			}
            if (ret == 0)
                print_auto_led_level(ctrl_port, VC_LED_CTRL_COLOR_R, curr);
        break;
            
        case 'g':
        case 'G':
            sscanf(argv[1], "%c", &ch);
            if (ch == '+') {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].G += LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].G += LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_G);
            }
            else if (ch == '-') {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].G -= LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].G -= LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_G);
            }
			else {
				DMSG("unknown - [%c]\n", ch);
			}
            if (ret == 0)
                print_auto_led_level(ctrl_port, VC_LED_CTRL_COLOR_G, curr);
        break;
        case 'b':
        case 'B':
            sscanf(argv[1], "%c", &ch);
            if (ch == '+') {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].B += LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].B += LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_B);
            }
            else if (ch == '-') {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0)
                    curr[0].B -= LED_CURRENT_STEP;
                    
                if (ctrl_port & VC_PANEL_CTRL_PORT_1)
                    curr[1].B -= LED_CURRENT_STEP;
                    
                ret = rtiVC_SetLedCurrent(ctrl_port, curr, VC_LED_CTRL_COLOR_B);
            }
			else {
				DMSG("unknown - [%c]\n", ch);
			}
            if (ret == 0)
                print_auto_led_level(ctrl_port, VC_LED_CTRL_COLOR_B, curr);
            break;
        case 'a':
        case 'A':
                sscanf(argv[1], "%c", &ch);
                if (ch == '+') {
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
				else {
					DMSG("unknown - [%c]\n", ch);
				}
                if (ret == 0)
                    print_auto_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
            
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
            
            ret = rtiVC_SetLedCurrent((E_VC_PANEL_CTRL_PORT_T)ctrl_port, curr, VC_LED_CTRL_COLOR_ALL);
            
            print_auto_led_level((E_VC_PANEL_CTRL_PORT_T)ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
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
            
            print_auto_led_level(ctrl_port, VC_LED_CTRL_COLOR_ALL, curr);
            break;
        case '3':
            ret = rtiVC_DownloadLedCurrentPatchLUT(ctrl_port);
            break;
        default:
            DMSG("unknown - [%c]\n", *argv[0]);
            ret = -1;
    }
    return ret;
}

static int test_auto_LedDutyControl(const char * test_param[])
{
    char ch;
    int ret = -1;
    VC_LED_DUTY_INFO_T duty[MAX_NUM_VC_PANEL_PORT];

    E_VC_PANEL_CTRL_PORT_T ctrl_port = (E_VC_PANEL_CTRL_PORT_T)(atoi(*test_param)+1);
    test_param++;

    rtiVC_GetLedDuty(ctrl_port, duty);

	if(test_param[0] == NULL) {
		DMSG("unknown \n");
		return -1;
	}
    switch (*test_param[0]) {
        case 'r':
        case 'R':
#if 0
        	sscanf(argv[1], "%c", &ch);
#endif
			if (*test_param[1] == '+')
                ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_R);
            else if (*test_param[1] == '-')
                ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_R);
            else
				DMSG("unknown - [%c]\n", *test_param[1]);

            if (ret == 0)
                DMSG("R LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].R)); // temp
            break;

        case 'g':
        case 'G':
#if 0
        	sscanf(argv[1], "%c", &ch);
#endif
			if (*test_param[1] == '+')
                ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_G);
            else if (*test_param[1] == '-')
                ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_G);
			else 
				DMSG("unknown - [%c]\n", *test_param[1]);
            if (ret == 0)
                DMSG("G LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].G));
            break;
           
        case 'b':
        case 'B':
#if 0
        	sscanf(argv[1], "%c", &ch);
#endif
			if (*test_param[1] == '+')
                ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_B);
            else if (*test_param[1] == '-')
                ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_B);
			else
				DMSG("unknown - [%c]\n", *test_param[1]);
            if (ret == 0)
                DMSG("B LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].B));
            break;
            
        case 'a':
        case 'A':
#if 0
        	sscanf(argv[1], "%c", &ch);
#endif
            if (*test_param[1] == '+')
                ret = rtiVC_StepUpLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_ALL);
            else if (*test_param[1] == '-')
                ret = rtiVC_StepDownLedDuty((E_VC_PANEL_CTRL_PORT_T)ctrl_port, duty, VC_LED_CTRL_COLOR_ALL);
			else 
				DMSG("unknown - [%c]\n", *test_param[1]);
                
            if (ret == 0) {
                DMSG("R LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].R));
                DMSG("G LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].G));
                DMSG("B LED duty: %.1f %%\n", VC_LED_DUTY_TO_PERCENTAGE(duty[0].B));
            }
            break;
        case '3':
#if 0
        	ret = rtiVC_DownloadLedDutyPatchLUT();
#endif
        	break;
        default:
        	DMSG("unknown - [%c]\n", *test_param[0]);
    }
	return ret;
}

static int test_auto_TestPatternDisable()
{
	FLASH_FW_CFG_INFO_T cfg;

	int ret = rtiVC_ReadFLASH(FLASH_PART_OFFSET_FW_CFG,
							(U8_T *)&cfg, sizeof(FLASH_FW_CFG_INFO_T));
	if (ret != 0) {
		EMSG("Fail to read the RDC patch header in FLASH\n");
		return -1;
	}

	if(cfg.dif_path < 6)
	{
		RDC_REG_MASK_SET(0x200, 0x07, cfg.dif_path);
	}
	else {	
		EMSG("Cfg havs wrong value\n");
		return -1;
	}

	return 0;
}

static int test_auto_TestPattern(const char *rdc_name, const char * argv[])
{
    int ret = -1;
    int R, G, B;
    R = 0xF3;	// initial value
    G = 0x33;	// initial value
    B = 0x93;	// initial value
    
    switch(*argv[0])
    {
        case '0':
#if 0
            sscanf(argv[1], "%d" , &R);
            sscanf(argv[2], "%d" , &G);
            sscanf(argv[3], "%d" , &B);
#endif
                
            ret = rtiVC_EnableTestPattern(TRUE);
            if (ret == 0) {
                rtiVC_GenerateTestPattern(R, G, B);
                DMSG("Enabled\n");
            }
            
        break;
            
        case '1':
            ret = test_auto_TestPatternDisable();
			DMSG("Disabled\n");
            break;

        default:
            DMSG("unknown - [%c]\n", *argv[0]);
    }
    return ret;
}

static void test_auto_Brightness(char *ch)
{
//	char ch;

#if 0
	sscanf(test_param[0], "%c", &ch);
#endif
    //DMSG("+ :Increase, - :Decrease, q(Quit)\n");
    
    if (*ch == '+') {
        DMSG("Increase pressed\n");
        rtiVC_IncreaseBrightness();
    }
    else if (*ch == '-') {
        DMSG("Decrease pressed\n");
        rtiVC_DecreaseBrightness();
    }
}

static int test_auto_AlignmentVerticalShift(E_VC_PANEL_CTRL_PORT_T ctrl_port, const char * argv[])
{
    int shift_val;
    shift_val = 0;
    
#if defined(CFG_PANEL_RDP550F)
    DMSG("Vertical Shift Range : -8 ~ +8\n");
#elif defined(CFG_PANEL_RDP551F) || defined(CFG_PANEL_RDP502H) || defined(CFG_PANEL_RDP370F)
    DMSG("Vertical Shift Range : 0 ~ 200, cropped if exceed 32\n");
#endif
#if 0
	sscanf(argv[0], "%d", &shift_val);
#endif
	DMSG("V-shift : %d\n", shift_val);
    
	return rtiVC_SetVerticalPixelShift(ctrl_port, shift_val); 
}

static int test_auto_AlignmentHorizontalShift(E_VC_PANEL_CTRL_PORT_T ctrl_port, const char * argv[])
{
    int shift_val;
    shift_val = 0;

#if defined(CFG_PANEL_RDP550F)
    DMSG("IHorizontal Shift Range : -32 ~ +31\n");
#elif defined(CFG_PANEL_RDP551F)
    DMSG("Horizontal Shift Range : 0 ~ 200, cropped if exceed 144\n");
#elif defined(CFG_PANEL_RDP502H) || defined(CFG_PANEL_RDP370F)
    DMSG("Horizontal Shift Range : 0 ~ 200, cropped if exceed 32\n");
#endif
    
#if 0
    sscanf(argv[0], "%d", &shift_val);
#endif
	DMSG("H-shift : %d\n", shift_val);
    
	return rtiVC_SetHorizontalPixelShift(ctrl_port, shift_val);
}

static int test_auto_FactoryCalibration(const char * argv[])
{
    int ret  =  0;
#if defined(CFG_PANEL_RDP550F) || defined(CFG_PANEL_RDP551F) || defined(CFG_PANEL_RDP502H)
    E_VC_PANEL_CTRL_PORT_T ctrl_port = (E_VC_PANEL_CTRL_PORT_T)(atoi(*argv)+1);
    argv++;
    
	if(argv[0] == NULL) {
		 DMSG("unknown\n");
		 return -1;
	}

    switch(*argv[0]) {
        case '0':
            ret = test_auto_AlignmentHorizontalShift(ctrl_port, ++argv);
            break;
        case '1':
            ret = test_auto_AlignmentVerticalShift(ctrl_port, ++argv);
            break;
        case '3':
            ret = rtiVC_DownloadPixelShiftPatchLUT(ctrl_port);
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
			rtiVC_TuneManualRdcPatch(*argv[1], *argv[2]);
			break;
		case 'e':
			rtiVC_DownloadRdcManualPatchLUT();
			break;
		case 'f':
			rtiVC_ResetManualRdcPatch();
			break;
		case 'g':
            rtiVC_TuneManualPanelPatch(ctrl_port, *argv[1], *argv[2]);
			break;
		case 'h':
            rtiVC_DownloadPanelManualPatchLUT(ctrl_port);
			break;
		case 'i':
			rtiVC_ResetManualPanelPatch();
			break;

		default:
			 DMSG("unknown - [%c]\n", *argv[0]);
			break;
    }
    
#else
    DMSG("Not support to alignment panel test\n");
    return -1;
#endif
	return ret;
}

static int test_auto_patch_manage(const char* argv[])
{
    char mode = (*argv)[0];
	int param;
    int base_patch_addr = 0;

	argv++;
    param = atoi(*argv);
    argv++;
    
    if(param == 0)
        base_patch_addr  =FLASH_PART_OFFSET_RDC_PATCH_BASE;
    else if(param == 1)
        base_patch_addr  =FLASH_PART_OFFSET_PANEL_0_PATCH_BASE;
    else if(param == 2)
        base_patch_addr  =FLASH_PART_OFFSET_PANEL_1_PATCH_BASE;
    else
        return -1;
    
    switch(mode)
    {
        case 'd':
        {
            rtiVC_EraseSectorFLASH(base_patch_addr);
            IMSG("Erase Done.\n");
        }
        break;
        case 'v':
        {
            int j;
            U8_T buffer[256];

			rtiVC_ReadFLASH(base_patch_addr, buffer, 256);
            for(j = 0; j< 256; j++)
            {
                IMSG("%2X ", buffer[j]);
                if((j+1) % 16 == 0) IMSG("\n");
            }
            IMSG("\n");
        }
        break;
    }
	return 0;
}

static int test_auto_cokoa(const char* argv[])
{
#if 0
    int ret = -1;
	VC_COKOA_PARAM_EX_T param;
	char filename_buf[256] = {0};
	char cokoa_cmd = *argv[0];
	char cokoa_enable = '4';
	UINT_T lut_offset[MAX_NUM_VC_PANEL_PORT];

    E_VC_PANEL_CTRL_PORT_T ctrl_port = (E_VC_PANEL_CTRL_PORT_T)(atoi(*argv)+1);
    argv++;

	if(argv[0] == NULL) {
		 DMSG("unknown arg\n");
		 return -1;
	}
    
    if(cokoa_cmd == '0' || cokoa_cmd == '1')
    {
        U32_T width = 0, height = 0;
        rtiVC_GetOutputResolution(&width, &height);
        param.pix_w = width;
        param.pix_h = height;
        param.lut_id = atoi(argv[1]);
        param.k1_x = atof(argv[2]);
        param.k2_x = atof(argv[3]);
        param.k3_x = atof(argv[4]);
        param.k1_y = atof(argv[5]);
        param.k2_y = atof(argv[6]);
        param.k3_y = atof(argv[7]);
        param.chrm_r = atoi(argv[8]);
        param.chrm_g = atoi(argv[9]);
        param.chrm_b = atoi(argv[10]);
        if(cokoa_cmd == '0' || cokoa_cmd == '1')
        {
            param.rotation = atof(argv[11]);
            param.interp_x = atof(argv[12]);
            param.interp_y = atof(argv[13]);
            param.sbys_3d = FALSE;
            if(cokoa_cmd == '1') cokoa_enable = *argv[14];
        }
        sprintf(filename_buf, "%s%ccokoa.lut", RDC200A_LUT_FILE_PATH, PathSeparator);
        
        ret = rtiVC_GenerateCoKOALuto(&param, filename_buf);
        if(ret == 0)
        {
            DMSG("CoKOA LUT file generated\n");
        }
        else
        {
            DMSG("Can't Generate CoKOA LUT (%d)\n", ret);
            return ret;
        }
        
        ret = prepare_optic_dc_lut(filename_buf);
        if (ret == 0)
            rtiVC_StartDownload();

		// temp
	#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
		lut_offset[0] = param.lut_id;
	#endif

	#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
		lut_offset[1] = param.lut_id;
	#endif

        if(cokoa_enable == '2')
            ret = rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);
        else if(cokoa_enable == '3')
            ret = rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset, TRUE);
        else if(cokoa_enable != '4')
            DMSG("unknown - CoKOA command(%c)\n", cokoa_cmd);
        
    }
    else if(cokoa_cmd ==  '2')
    {
        DMSG("CoKOA Enabled\n");

		// temp
	#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
		lut_offset[0] = atoi(argv[1]);
	#endif

	#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
		lut_offset[1] = atoi(argv[1]);
	#endif

		ret = rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, TRUE, lut_offset, TRUE);
    }
    else if(cokoa_cmd == '3')
    {        
        DMSG("CoKOA Disabled\n");
		ret = rtiVC_EnableDistortionCorrection((E_VC_PANEL_CTRL_PORT_T)ctrl_port, FALSE, lut_offset, TRUE);
    }
    else
    {
		DMSG("unknown - CoKOA command(%c)\n", cokoa_cmd);
	}
	return ret;
#endif
}

int test_auto_TemperatureSensorCalibration(const char *panel_name, const char* argv[])
{
    int ret;
    VC_PANEL_TEMPERATURE_INFO_T tinfo[MAX_NUM_VC_PANEL_PORT];
    E_VC_PANEL_CTRL_PORT_T ctrl_port;
    float curr_tpe;
    VC_PANEL_TEMPERATURE_INFO_T *t;
    
    ctrl_port = (E_VC_PANEL_CTRL_PORT_T)(atoi(*argv)+1);
    argv++;
    
    switch (*argv[0]) {
        case '0':
            DMSG("Get Temperature\n");
            ret = rtiVC_GetTemperature(ctrl_port, tinfo);
            if (ret == 0) {
                if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
                    t = &tinfo[0];
                    curr_tpe = (float)t->temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
                    
                    DMSG("P0: otp_applied(%d) curr[0x%03X, %.3f oC]\n", t->otp_applied, t->tsensor_code, curr_tpe);
                    ret = (int)curr_tpe;
                }
                
                if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
                    t = &tinfo[1];
                    curr_tpe = (float)t->temperature/(float)VC_TEMPERATURE_DEGREE_DIV;
                    
                    DMSG("P1: otp_applied(%d) curr[0x%03X, %.3f oC]\n",
                         t->otp_applied, t->tsensor_code, curr_tpe);
                    ret = (int)curr_tpe;
                }
            }
            break;
        default:
            DMSG("[%c]\n", *argv[0]);
            ret = -1;
    }
    
    return ret;
}


int test_RDC200A_auto(E_VC_PANEL_DEVICE_T panel_type, const char * test_param[])
{
    int ret = 0;
    static char name_buf[64];
    char filename[1024];
    U8_T major_ver, minor_ver, rel_ver;

	// Open device
    if((ret = rtiVC_OpenDevice()) < 0)
		return ret;

    if((ret = test_auto_interface_setup(test_param)) < 0)
        return ret;

    // consume two argument
    test_param++;
    test_param++;

#if 0
    // temp. flag and file path are will be recv from UI
    vc_SetLutDirectoryPath(RDC200A_LUT_FILE_PATH);
#endif
  
    if(test_param[0] == NULL) {
        DMSG("unknown\n");
        return -1;
    }

    switch(*test_param[0])
    {
        case '0':
            DMSG("[Download] Firmware Hex file\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_firmware_hex_file(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case '1':
            DMSG("[Download] Firmware Configuration\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_firmware_config_file(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case '2':
            DMSG("[Download] RDC200A Init LUT\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_rdc_init_lut(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case '3':
            DMSG("[Download] Panel Init LUT\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_panel_init_lut(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case '4':
            DMSG("[Download] Panel Gamma LUT\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_panel_gamma_lut(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case '7':
            DMSG("[Download] Optic CoKOA LUT\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = prepare_optic_dc_lut(filename);
                if (ret == 0)
                    rtiVC_StartDownload(); // API call.
            }
#endif
            break;
            
        case 'b':
            DMSG("[RDP] Temperature sensor \n");
            ret = test_auto_TemperatureSensorCalibration(g_panel_name, ++test_param);
            break;
            
        case 's':
            DMSG("[Full Download] Using file list <rdc200_download_file_list.txt> instead of user input\n");
#if 0
            test_param++;
            memcpy(filename, test_param[0], strlen(test_param[0])+1);
            if (*filename) {
                ret = test_Download_FileList(filename, TRUE, LUT_DUMP_FILE_PATH, VC_LUT_DUMP_NONE);
            }
#endif
            break;
            
        case 'c':
            DMSG("[Tuning] Brightness\n");
//			test_auto_Brightness(++test_param);
            break;
            
        case 'p':
//        	test_PanelPowerOnOff('1');
            break;
            
        case 'i':
            DMSG("[Tuning] LED duty control\n");
            ret = test_auto_LedDutyControl(++test_param);
            break;
            
        case 'r':
            DMSG("[Tuning] LED current control\n");
#if 0
            ret = test_auto_LedCurrentControl(++test_param);
#endif
            break;
            
        case 'j':
            DMSG("Test Pattern\n");
            ret = test_auto_TestPattern("RDC200A",++test_param);
            break;
            
        case 'k':
            DMSG("Image display format (2D/3D_SBS)\n");
            test_ImageDisplayFormat('0', 1);
            break;
            
        case 'o':
            DMSG("[Tuning] Alignment Panel\n");
#if 0
            ret = test_auto_FactoryCalibration(++test_param);
#endif
            break;
       	
		case 'l':
			DMSG("Distortion Correction(CoKOA)\n");
#if 0
			test_auto_cokoa(++test_param);
#endif
			break;

        case 'v':
            rtiVC_GetFirmwareVersion(&major_ver, &minor_ver, &rel_ver);
            DMSG("RDC200A firmware version: %d.%d.%d\n", major_ver, minor_ver, rel_ver);
            ret = (major_ver << 8 | minor_ver);
            test_auto_terminate(ret);
            break;
            
        case 'w':
            DMSG("[FLASH] Test\n");
            //rtiVC_EraseFlashChip();
            break;
            
        case 'x':
            ret = test_auto_RegisterIO_RDC("RDC200A",++test_param);
            break;
        
        case 'y':
            sprintf(name_buf, "Panel : %s", g_panel_name);
            ret = test_auto_RegisterIO_RDP(name_buf, ++test_param);
            break;
            
        case 'z':
            ret = test_auto_patch_manage(++test_param);
            break;
        
		default:
            DMSG("Wrong input : [%c]\n", *test_param[0]);
            ret = -1;
    }

	return ret;
}

#endif // #ifdef CFG_MDC_RDC200

