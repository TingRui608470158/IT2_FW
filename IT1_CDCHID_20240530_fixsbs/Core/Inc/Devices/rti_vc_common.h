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

#ifndef __RTI_VC_COMMON_H__
#define __RTI_VC_COMMON_H__

#include "rti_vc_api.h"
#include "rti_vc_regio.h"

#ifdef __cplusplus
extern "C"{
#endif

extern U8_T vc_rdc_i2c_slave_addr;

extern int vc_prev_panel_port; /* Previous panel port */

extern U8_T vc_prev_rdc_slave_addr; /* Previous RDC slave address */

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_COMMON_H__ */

