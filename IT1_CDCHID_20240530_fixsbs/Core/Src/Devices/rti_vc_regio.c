/**
* Commone register IO functions
*/

#include "rti_vc_common.h"

#define RDP_I2C_OP_WRITE	0
#define RDP_I2C_OP_READ		1

#define RDP_I2C_ADDR_S_1byte	0
#define RDP_I2C_ADDR_S_2byte	1
#define RDP_I2C_ADDR_S_3byte	2
#define RDP_I2C_ADDR_S_4byte	3

#if defined(CFG_MDC_RDC200)
void RDC_REG_MASK_SET(U8_T reg, U8_T mask, U8_T val)
{
	U8_T tmp;

	tmp = (RDC_REG_GET(reg)|(U8_T)(mask))
			& (U8_T)((~(mask))|(val));
	RDC_REG_SET(reg, tmp);
}

///////////// Panel I2C path via RDC200 ////////////////
void RDP_REG_SET(int panel_port, U16_T reg, U8_T val)
{
	U8_T intr_flag, retry_cnt = 200;
	U8_T reg_0x5E, done_flag;
	int err_flag;
	U8_T reg_size = 0;

#if (VC_RDP_REG_SIZE == 1)
	reg_size = RDP_I2C_ADDR_S_1byte;

#elif (VC_RDP_REG_SIZE == 2)
	reg_size = RDP_I2C_ADDR_S_2byte;

#else
	#error "Code not present"
#endif	
	
	RDC_REG_SET(0x6C, 0x80); // func = 1

	reg_0x5E = panel_port | (RDP_I2C_OP_WRITE<<2) | (reg_size<<3);
	RDC_REG_SET(0x5E, reg_0x5E);

	RDC_REG_SET(0x62, reg & 0xFF);
#if (VC_RDP_REG_SIZE == 2)	
	RDC_REG_SET(0x61, reg >> 8);
#endif
	
	RDC_REG_SET(0x63, val);

	intr_flag = RDC_REG_GET(0xC2);

	// set FW interrupt 
	RDC_REG_SET(0xC2, intr_flag | 0x08);

#if (VC_RDP_REG_SIZE == 1)
	VC_M_DELAY(1); //Minimum 1ms
	
#elif (VC_RDP_REG_SIZE == 2)
	VC_M_DELAY(2);	
#else
	#error "Code not present"
#endif

	// clear FW interrupt
	RDC_REG_SET(0xC2, intr_flag & 0xF7);

	do {
		done_flag = RDC_REG_GET(0x6B);
		if (done_flag) {
			RDC_REG_SET(0x6B, 0x00); /* Clear done_flag */

			err_flag = RDC_REG_GET(0x6A);
			if (err_flag)
				EMSG("[RDP_%d] Register write error (%d)\n", panel_port, -err_flag);

			return;
		}
		VC_M_DELAY(1);
	} while (--retry_cnt);

	EMSG("[RDP_%d] Register write timeout\n", panel_port);
}

U8_T RDP_REG_GET(int panel_port, U16_T reg)
{
	U8_T intr_flag;
	unsigned int retry_cnt = 200;
	U8_T reg_0x5E, done_flag, rdata = 0xFF;
	int err_flag;
	U8_T reg_size;

#if (VC_RDP_REG_SIZE == 1)
	reg_size = RDP_I2C_ADDR_S_1byte;

#elif (VC_RDP_REG_SIZE == 2)
	reg_size = RDP_I2C_ADDR_S_2byte;

#else
	#error "Code not present"
#endif	

	RDC_REG_SET(0x6C, 0x80); // func = 1

	reg_0x5E = panel_port | (RDP_I2C_OP_READ<<2) | (reg_size<<3);
	RDC_REG_SET(0x5E, reg_0x5E);

	RDC_REG_SET(0x62, reg & 0xFF);
#if (VC_RDP_REG_SIZE == 2)
	RDC_REG_SET(0x61, reg >> 8);
#endif

	intr_flag = RDC_REG_GET(0xC2);

	// set FW interrupt 
	RDC_REG_SET(0xC2, intr_flag | 0x08);

#if (VC_RDP_REG_SIZE == 1)
	VC_M_DELAY(1); //Minimum 1ms
	
#elif (VC_RDP_REG_SIZE == 2)
	VC_M_DELAY(2);	
#else
	#error "Code not present"
#endif	
    
	// clear FW interrupt
	RDC_REG_SET(0xC2, intr_flag & 0xF7);

	do {
		done_flag = RDC_REG_GET(0x6B);
		if (done_flag) {
			RDC_REG_SET(0x6B, 0x00); /* Clear done_flag */

			err_flag = RDC_REG_GET(0x6A);
			if (!err_flag)
				rdata = RDC_REG_GET(0x63);
			else
				EMSG("[RDP_%d] Register read error (%d)\n", panel_port, -err_flag);

			return rdata;
		}

		VC_M_DELAY(1);
	} while (--retry_cnt);

	EMSG("[RDP_%d] Register read timeout\n", panel_port);
	return 0xFF;
}

#define RDP_REG_GET_0(reg)\
	RDP_REG_GET(0, reg)

#define RDP_REG_SET_0(reg, val)\
	RDP_REG_SET(0, reg, val)

#define RDP_REG_GET_1(reg)\
	RDP_REG_GET(1, reg)
	
#define RDP_REG_SET_1(reg, val)\
	RDP_REG_SET(1, reg, val)

#elif defined(CFG_MDC_RDC200A)
void RDC_REG_MASK_SET(U16_T reg, U8_T mask, U8_T val)
{
	U8_T tmp;

	tmp = (RDC_REG_GET(reg)|(U8_T)(mask))
			& (U8_T)((~(mask))|(val));
	RDC_REG_SET(reg, tmp);
}

///////////// Panel I2C path via RDC200A ////////////////
#if 1
void RDP_REG_SET(int panel_port, U16_T reg, U8_T val)
{
	unsigned int retry_cnt = 200;
	U8_T reg_0x550, result_flags;
	int err_flag;
	U8_T addr_size = 0;

#if (VC_RDP_REG_SIZE == 1)
	addr_size = RDP_I2C_ADDR_S_1byte;

#elif (VC_RDP_REG_SIZE == 2)
	addr_size = RDP_I2C_ADDR_S_2byte;

#else
	#error "Code not present"
#endif	
	
	reg_0x550 = panel_port | (RDP_I2C_OP_WRITE<<2) | (addr_size<<3);
	RDC_REG_SET(0x0550, reg_0x550);

	RDC_REG_SET(0x0552, reg & 0xFF);
#if (VC_RDP_REG_SIZE == 2)	
	RDC_REG_SET(0x0551, reg >> 8);
#endif
	
	RDC_REG_SET(0x0553, val);

	// set FW interrupt 
	RDC_REG_SET(0x05A4, 0x01); // INTR_SET<31:24> reg, [24]: button 0

	/* Wait until done_flag is set. */
	VC_M_DELAY(1); // Minimum 1ms
	
	do {
		result_flags = RDC_REG_GET(0x0554);
		if (result_flags & 0x01) { /* Check done_flag bit */
			RDC_REG_SET(0x0554, 0x00); /* Clear done_flag */

			err_flag = (int)(result_flags >> 1);
			if (err_flag)
				EMSG("[RDP_%d] Register write error (%d)\n", panel_port, -err_flag);

			return;
		}

		VC_U_DELAY(500);
	} while (--retry_cnt);

	EMSG("[RDP_%d] Register write timeout\n", panel_port);
}

U8_T RDP_REG_GET(int panel_port, U16_T reg)
{
	unsigned int retry_cnt = 200;
	U8_T reg_0x550, result_flags, rdata = 0xFF;
	int err_flag;
	U8_T reg_size;

#if (VC_RDP_REG_SIZE == 1)
	reg_size = RDP_I2C_ADDR_S_1byte;

#elif (VC_RDP_REG_SIZE == 2)
	reg_size = RDP_I2C_ADDR_S_2byte;

#else
	#error "Code not present"
#endif	

	reg_0x550 = panel_port | (RDP_I2C_OP_READ<<2) | (reg_size<<3);
	RDC_REG_SET(0x0550, reg_0x550);

	RDC_REG_SET(0x0552, reg & 0xFF);
#if (VC_RDP_REG_SIZE == 2)
	RDC_REG_SET(0x0551, reg >> 8);
#endif

	// set FW interrupt
	RDC_REG_SET(0x05A4, 0x01); // INTR_SET<31:24> reg, [24]: button 0

	/* Wait until done_flag is set. */
	VC_M_DELAY(1); // Minimum 1ms
	
	do {
		result_flags = RDC_REG_GET(0x0554);
		if (result_flags) {
			RDC_REG_SET(0x0554, 0x00); /* Clear done_flag bit */

			err_flag = (int)(result_flags >> 1);
			if (!err_flag)
				rdata = RDC_REG_GET(0x0553);
			else
				EMSG("[RDP_%d] Register read error (%d)\n", panel_port, -err_flag);

			return rdata;
		}

		VC_U_DELAY(500);
	} while (--retry_cnt);

	EMSG("[RDP_%d] Register read timeout\n", panel_port);
	return 0xFF;
}

#else
#if (VC_RDP_REG_SIZE == 1)
	U8_T RDP_REG_GET(int panel_port, U8_T reg)
	{
		/* TX0_SLV_I2C_BYPASS or TX1_SLV_I2C_BYPASS */
		RDC_REG_SET(0x0006, 1<<panel_port);

		if (panel_port == 0)
			return rtiVC_ReadDevice(VC_DEVICE_CH_RDC_RDP0, _CFG_RDP_SLAVE_ADDR, (U8_T)reg);
		else
			return rtiVC_ReadDevice(VC_DEVICE_CH_RDP1, _CFG_RDP_SLAVE_ADDR, (U8_T)reg);
	}

	void RDP_REG_SET(int panel_port, U8_T reg, U8_T val)
	{
		/* TX0_SLV_I2C_BYPASS or TX1_SLV_I2C_BYPASS */
		RDC_REG_SET(0x0006, 1<<panel_port);

		if (panel_port == 0)
			rtiVC_WriteDevice(VC_DEVICE_CH_RDC_RDP0, _CFG_RDP_SLAVE_ADDR, (U8_T)reg, val);
		else
			rtiVC_WriteDevice(VC_DEVICE_CH_RDP1, _CFG_RDP_SLAVE_ADDR, (U8_T)reg, val);
	}

#elif (VC_RDP_REG_SIZE == 2)
	U8_T RDP_REG_GET(int panel_port, U16_T reg)
	{
		if (vc_prev_panel_port != panel_port) {
			vc_prev_panel_port = panel_port;
			RDC_REG_SET(0x0006, 1<<panel_port); /* TX0_SLV_I2C_BYPASS or TX1_SLV_I2C_BYPASS */
		}
			
		if (panel_port == 0)
			return rtiVC_ReadDevice16(VC_DEVICE_CH_RDC_RDP0, _CFG_RDP_SLAVE_ADDR, reg);
		else
			return rtiVC_ReadDevice16(VC_DEVICE_CH_RDP1, _CFG_RDP_SLAVE_ADDR, reg);
	}

	void RDP_REG_SET(int panel_port, U16_T reg, U8_T val)
	{
		if (vc_prev_panel_port != panel_port) {
			vc_prev_panel_port = panel_port;
			RDC_REG_SET(0x0006, 1<<panel_port); /* TX0_SLV_I2C_BYPASS or TX1_SLV_I2C_BYPASS */
		}
			
		if (panel_port == 0)
			rtiVC_WriteDevice16(VC_DEVICE_CH_RDC_RDP0, _CFG_RDP_SLAVE_ADDR, reg, val);
		else
			rtiVC_WriteDevice16(VC_DEVICE_CH_RDP1, _CFG_RDP_SLAVE_ADDR, reg, val);
	}
#else
	#error "Code not present"
#endif

#endif // backdoor

#endif

void RDP_REG_MASK_SET(int panel_port, U16_T reg, U8_T mask, U8_T val)
{
	U8_T tmp;

	tmp = (RDP_REG_GET(panel_port, reg)|(U8_T)(mask))
			& (U8_T)((~(mask))|(val));
	RDP_REG_SET(panel_port, reg, tmp);
}


