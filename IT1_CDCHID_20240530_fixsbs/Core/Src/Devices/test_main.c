
/**
 * @file test_main.c
 * @author Kevin Ko
 * @brief Main entry File
 */

#include "test.h"
#include "rti_vc_common.h"

double select_gamma_value(void)
{
	double gamma_val;
	char read_buf[100], *token;
	
	while (1) {
		DMSG("Input gamma value (0.1 ~ 4.0), q(Quit):");
		fgets(read_buf, 100, stdin);

		token = strtok(read_buf, " \n\r");
		if (token == NULL)
			continue;

		if (token[0] == 'q' || token[0] == 'Q')
			return -100.;
		
		gamma_val = atof(token);
		
		if ((gamma_val >= 0.1) && (gamma_val <= 4.0))
			break;
		else {
			EMSG("Invalid gamma value! Retry\n");
		}
	}

	return gamma_val;
}

void test_RegisterIO_RDP(int panel_port, const char *test_dev_name, int key, unsigned int reg_addr, unsigned int read_cnt, unsigned int write_data)
{
//	int key;
//	unsigned int i, read_cnt, reg_addr, write_data;
	unsigned int i;
	U8_T reg_val;

#if 0
	while(1) {
#endif
		DMSG("================ [%s] Register IO Test ===============\n", test_dev_name);
#ifdef CFG_PANEL_RDP501H
		DMSG("\t0: Write a data to register\n");
#else
		DMSG("\t0: Single Read\n");		
		DMSG("\t1: Single Write\n");
		DMSG("\t2: Burst Read\n");
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
#ifdef CFG_PANEL_RDP501H
		case '0':
			DMSG("[%s] Write a data to register\n", test_dev_name);
			
			DMSG("Input Address(hex) :,  data(hex) : ");
			scanf("%x" , &reg_addr);	
			CLEAR_STDIN;
			
			scanf("%x" , &write_data);
			CLEAR_STDIN;

			if (panel_port == 0)
				rtiVC_WriteRDP501H_0(reg_addr, write_data);
			else
				rtiVC_WriteRDP501H_1(reg_addr, write_data);
			break;
#else
		case '0':
			DMSG("[%s] Single Read\n", test_dev_name);

			DMSG("Input Address(Hex) : ");
#if 0
			scanf("%x", &reg_addr);
			CLEAR_STDIN;
#endif

			reg_val = RDP_REG_GET(panel_port, reg_addr);
			DMSG("[%s] S_READ: addr(0x%02X), val(0x%02X)\n",
				test_dev_name, reg_addr, reg_val);
			break;

		case '1':
			DMSG("[%s] Single Write\n", test_dev_name);
			
            DMSG("Addr(hex) Data(hex) : ");
#if 0
			scanf("%x %x" , &reg_addr, &write_data);
			CLEAR_STDIN;
#endif

			RDP_REG_SET(panel_port, reg_addr, write_data);
			break;

		case '2':
			DMSG("[%s] Burst Read\n", test_dev_name);

#if 0
			while(1) {
#endif
				DMSG("Starting Address(hex)  Count(dec) : ");
#if 0
				scanf("%x %u" , &reg_addr, &read_cnt);
				CLEAR_STDIN;
#endif
				if ((reg_addr + read_cnt) > 0x3FF)
					EMSG("The count is out of register address\n");

				if ((reg_addr <= 0x3FF) && ((reg_addr + read_cnt) <= 0x3FF))
					break;
#if 0
			}
#endif

			DMSG("Address\tValue\n");
			for (i = reg_addr; i < (reg_addr + read_cnt); i++) {
				reg_val = RDP_REG_GET(panel_port, i);

				DMSG("0x%04X  \t 0x%02X\n", reg_addr+i, reg_val);
			}
			break;			
#endif

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
	return;
}

void test_RegisterIO_RDC(const char *test_dev_name, int key, unsigned int reg_addr, unsigned int write_data)
{
//	int key;
//	unsigned int i, reg_addr, read_cnt, write_data;
	unsigned int i, read_cnt;
	static U8_T reg_val, burst_buf[128], burst_wbuf[128];
	U16_T write_size = 0;
	U8_T val = 0;
	unsigned int burst_cnt = 20;

#if 0
	while(1) {
#endif
		DMSG("================ [%s] Register IO Test ===============\n", test_dev_name);
		DMSG("\t0: Single Read\n");		
		DMSG("\t1: Single Write\n");
		DMSG("\t2: Burst Read\n");
		DMSG("\t3: Burst Write\n");
		DMSG("\tq or Q: Quit\n");
		DMSG("================================================\n");

#if 0
		FFLUSH;
		key = getc(stdin);
		CLEAR_STDIN;
#endif
		
		switch(key) {
		case '0':
			DMSG("[RDC] Single Read\n");

			DMSG("Input Address(Hex) : ");
#if 0
			scanf("%x", &reg_addr);
			CLEAR_STDIN;
#endif

			reg_val = RDC_REG_GET(reg_addr);
			DMSG("[RDC] S_READ: addr(0x%04X), val(0x%02X)\n", reg_addr, reg_val);
			break;

		case '1':
			DMSG("[RDC Single Write\n");
			
            DMSG("Addr(hex) Data(hex) : ");
#if 0
			scanf("%x %x" , &reg_addr, &write_data);
			CLEAR_STDIN;
#endif
			
			RDC_REG_SET(reg_addr, write_data);
			break;

		case '2':
			DMSG("[%s] Burst Read\n", test_dev_name);

#if 0
			while(1) {
				DMSG("Starting Address(hex) :,	count(dec) : ");
				scanf("%x" , &reg_addr);	
				CLEAR_STDIN;

				scanf("%u" , &read_cnt);
				CLEAR_STDIN;

				if ((reg_addr + read_cnt) > 0x09C0)
					EMSG("The count is out of register address\n");

				if ((reg_addr <= 0x09C0) && ((reg_addr + read_cnt) <= 0x09C0))
					break;
			}
#else
			read_cnt = burst_cnt;
			reg_addr = 0x0550;
#endif

#if 1
			burst_wbuf[0] = reg_addr >> 8;
			burst_wbuf[1] = reg_addr & 0xFF;

			rtiVC_ReadBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr,
										burst_wbuf, 2,
										burst_buf, read_cnt);

			DMSG("Address\tValue\n");
			for (i = 0; i < read_cnt; i++)
				DMSG("0x%04X  \t 0x%02X\n", reg_addr+i, burst_buf[i]);
#else

			DMSG("Address\tValue\n");
			for (i = reg_addr; i < (reg_addr + read_cnt); i++) {
				reg_val = RDC_REG_GET(i);

				DMSG("0x%04X  \t 0x%02X\n", reg_addr+i, reg_val);
			}
#endif			
			break;

		case '3':
			DMSG("[%s] Burst Write\n", test_dev_name);
			
#if 0
			DMSG("Input the starting address of register(Hex) : ");
			scanf("%x", &reg_addr);
			CLEAR_STDIN;
#else
			reg_addr = 0x0550;
#endif
			val = 0;

			burst_buf[write_size++] = reg_addr >> 8;
			burst_buf[write_size++] = reg_addr & 0xFF;
			for (i = 0; i < burst_cnt; i++) {
				burst_buf[write_size++] = val++;
			}
			rtiVC_WriteBurstDeviceExt(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr,
									burst_buf, write_size);
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
	return;
}

/**
 * @brief Main Entry Function
 *        This function initialize Vista Cube then enter the test loop.
 */
int rdc200a_test_main(int test_num, const char * test_param[])
{
	int ret;
    E_VC_PANEL_DEVICE_T panel_type;
    
	ret = rtiVC_Initialize(RDC_I2C_SLAVE_ADDR);
	if (ret != 0) {
		EMSG("VC init error (%d)\n", ret);
		return -1;
	}

	rtiVC_RegisterDownloadCallback(vc_dload_erase_callback,
								vc_dload_program_callback,
								vc_dload_verify_callback,
								vc_dload_proc_callback);

#if defined(CFG_PANEL_RDP501H)
	panel_type = VC_PANEL_DEVICE_RDP501H;

#elif defined(CFG_PANEL_RDP360H)
	panel_type = VC_PANEL_DEVICE_RDP360H;

#elif defined(CFG_PANEL_RDP550F)
	panel_type = VC_PANEL_DEVICE_RDP550F;

#elif defined(CFG_PANEL_RDP700Q)
	panel_type = VC_PANEL_DEVICE_RDP700Q;

#elif defined(CFG_PANEL_RDP551F)
	panel_type = VC_PANEL_DEVICE_RDP551F;

#elif defined(CFG_PANEL_RDP502H)
	panel_type = VC_PANEL_DEVICE_RDP502H;

#elif defined(CFG_PANEL_RDP370F)
	panel_type = VC_PANEL_DEVICE_RDP370F;

#elif defined(CFG_PANEL_RDP250H)
	panel_type = VC_PANEL_DEVICE_RDP250H;

#else

	#error "Code not present"
#endif

#if defined(CFG_MDC_RDC200)
    if(argc == 1) {
        ret = test_RDC200(panel_type);
	} else if(argc > 3) {
        ret = test_RDC200_auto(panel_type, ++argv);
	} else {
        EMSG("The number of arguments is lack \n");
		ret = -1;
	}

#elif defined(CFG_MDC_RDC200A)
    if(test_num == 1) {
        ret = test_RDC200A(panel_type, '5');
	} else if(test_num > 2) {
        ret = test_RDC200A_auto(panel_type, test_param);
	} else {
        EMSG("The number of arguments is lack \n");
		ret = -1;
	}

#else
	#error "Code not present"
#endif

	rtiVC_CloseDevice();

	rtiVC_Denitialize();

	return ret;
}

