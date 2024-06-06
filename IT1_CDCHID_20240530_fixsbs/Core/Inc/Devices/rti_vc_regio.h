/**
* Commone register IO macros
*/


#ifndef __RTI_VC_REGIO_H__
#define __RTI_VC_REGIO_H__

#include "rti_vc_api.h"
#include "rti_vc_common.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * RDC register IO macros.
 */
#if defined(CFG_MDC_RDC200)
	#define RDC_REG_GET(reg)\
		rtiVC_ReadDevice(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr, (U8_T)reg)

	#define RDC_REG_SET(reg, val)\
		rtiVC_WriteDevice(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr, (U8_T)reg, val)

	void RDC_REG_MASK_SET(U8_T reg, U8_T mask, U8_T val);

#elif defined(CFG_MDC_RDC200A)
	#define RDC_REG_GET(reg)\
		rtiVC_ReadDevice16(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr, reg)

	#define RDC_REG_SET(reg, val)\
		rtiVC_WriteDevice16(VC_DEVICE_CH_RDC_RDP0, vc_rdc_i2c_slave_addr, reg, val)

	void RDC_REG_MASK_SET(U16_T reg, U8_T mask, U8_T val);
#else
	#error "Code not present"
#endif

U8_T RDP_REG_GET(int panel_port, U16_T reg);
void RDP_REG_SET(int panel_port, U16_T reg, U8_T val);
void RDP_REG_MASK_SET(int panel_port, U16_T reg, U8_T mask, U8_T val);

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_REGIO_H__ */

