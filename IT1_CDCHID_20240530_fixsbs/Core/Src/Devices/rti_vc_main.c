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
 * @file rti_vc_main.c
 * @author Kevin Ko
 * @brief The entry file
 */

#include "rti_vc_api.h"
#include "rti_vc_common.h"

void rtiVC_DeinitRDC(void);
int rtiVC_InitRDC(void);

void rtiVC_DeinitPanel(void);
void rtiVC_InitPanel(void);

//void rtiVC_DeinitDevice(void);
//void rtiVC_InitDevice(void);

static const char *vc_build_date = __DATE__;
static const char *vc_build_time = __TIME__;

U8_T vc_rdc_i2c_slave_addr = 0xFF;

int vc_prev_panel_port; /* Previous panel port */
U8_T vc_prev_rdc_slave_addr = 0xFF; /* Previous RDC slave address */

/**
	Bit mask of attached panel port.
		0x01: Port 0 attached
		0x02: Port 1 attached
		0x03: Port 0 and Port 1 attached
 */
UINT_T vc_panel_attached_port_mask = 0x0;

VC_MUTEX_DECLARE(vc_dev_access_lock)

/**
 * @brief Set the 7-bit I2C slave address of RDC.
 *
 * @param[in] rdc_i2c_slave_addr 7-bit I2C slave address of RDC.
 * @return 0 on success, otherwise returns a negative number.
 *
 */
void rtiVC_SetSlaveAddressRDC(U8_T rdc_i2c_slave_addr)
{
#if defined(CFG_MDC_RDC200A)
	if (vc_prev_rdc_slave_addr != rdc_i2c_slave_addr) {
		vc_prev_rdc_slave_addr = rdc_i2c_slave_addr;

		vc_rdc_i2c_slave_addr = rdc_i2c_slave_addr;

		/* Disable all panel backdoor path on the previous RDC. */
#if 0
		RDC_REG_SET(0x0006, 0x00);
#endif
	}

#endif // #if defined(CFG_MDC_RDC200A)

	vc_rdc_i2c_slave_addr = rdc_i2c_slave_addr;

#if 0// (!defined(ECLIPSE_RCP) && !defined(__KERNEL__) && defined(__linux__) /* Linux application */)
	i2c_bus_number[0] = i2c_bus_no;
#endif	
}

void rtiVC_Denitialize(void)
{
	rtiVC_DeinitPanel();

	rtiVC_DeinitRDC();

	rtiVC_DeinitDevice();

	VC_MUTEX_DEINIT(&vc_dev_access_lock);
}

static void config_panel_port_mask(void)
{
	vc_panel_attached_port_mask = 0x0;

#ifdef _CFG_RDP_ATTACHED_TO_PORT_0
	vc_panel_attached_port_mask = (1 << VC_PANEL_PORT_0);
#endif

#ifdef _CFG_RDP_ATTACHED_TO_PORT_1
	vc_panel_attached_port_mask |= (1 << VC_PANEL_PORT_1);
#endif
}

/**
 * @brief Initializes components of Vista Cube.
 *
 * @param[in] rdc_i2c_slave_addr 7-bit I2C slave address of RDC.
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_Initialize(U8_T rdc_i2c_slave_addr)
{
	int ret = 0;

	vc_prev_panel_port = -1; /* Previous panel port */
	vc_prev_rdc_slave_addr = 0xFF; /* Previous RDC slave address */

	vc_rdc_i2c_slave_addr = rdc_i2c_slave_addr;

	config_panel_port_mask();

	VC_MUTEX_INIT(&vc_dev_access_lock);

	rtiVC_InitDevice();

	ret = rtiVC_InitRDC();
	if (ret != 0)
		return ret;

	rtiVC_InitPanel();

	return 0;
}
