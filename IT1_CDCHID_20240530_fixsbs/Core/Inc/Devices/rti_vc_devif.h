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

#ifndef __RTI_VC_DEVIF_H__
#define __RTI_VC_DEVIF_H__

#include "rti_vc_api.h"
#include "main.h"
#include "i2c.h"

#if (!defined(ECLIPSE_RCP) && !defined(__KERNEL__) && defined(__linux__) /* Linux application */)
#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
/*
 * NOTE: Force align to 64-bit to compat 32-bit application.
 */
typedef struct {
	uint32_t reg_addr;
	uint8_t bus_num;
	uint8_t slave_addr;
	uint8_t reg_size;
	uint8_t data;
} RTIMD_SINGLE_WRITE_REG_T;

typedef struct {
	uint64_t wbuf_addr;

	uint8_t bus_num;
	uint8_t slave_addr;
	uint16_t wsize;
	uint32_t pad;
} RTIMD_BURST_WRITE_REG_T;

typedef struct {
	uint64_t rbuf_addr;

	uint32_t reg_addr;
	uint8_t bus_num;
	uint8_t slave_addr;
	uint8_t reg_size;
	uint8_t pad;
} RTIMD_SINGLE_READ_REG_T;

typedef struct {
	uint64_t wbuf_addr;
	uint64_t rbuf_addr;

	uint16_t wsize;
	uint16_t rsize;
	uint8_t bus_num;
	uint8_t slave_addr;
	uint16_t pad;
} RTIMD_BURST_READ_REG_T;


#define RTIMD_IOC_MAGIC	'R'

#define RTI_MD_DEV_NAME 	"/dev/rtimd-i2c"

#define SET_RTIMD(st, dev_ch, slave_dev_addr)	\
	do {										\
		st.bus_num = vc_i2c_bus_number[dev_ch];	\
		st.slave_addr = slave_dev_addr; 		\
	} while (0)

#define IOCTL_RTIMD_SINGLE_READ		_IOWR(RTIMD_IOC_MAGIC, 1, RTIMD_SINGLE_READ_REG_T)
#define IOCTL_RTIMD_BURST_READ		_IOWR(RTIMD_IOC_MAGIC, 2, RTIMD_BURST_READ_REG_T)
#define IOCTL_RTIMD_SINGLE_WRITE	_IOWR(RTIMD_IOC_MAGIC, 3, RTIMD_SINGLE_WRITE_REG_T)
#define IOCTL_RTIMD_BURST_WRITE		_IOWR(RTIMD_IOC_MAGIC, 4, RTIMD_BURST_WRITE_REG_T)

#else // !defined(_CFG_USE_RTIMD_KERNEL_DRIVER)
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define VC_LINUX_DEVICE_PATH_PREFIX	"/dev/i2c-"

#endif // _CFG_USE_RTIMD_KERNEL_DRIVER
#endif // #if (!defined(ECLIPSE_RCP) && !defined(__KERNEL__) && defined(__linux__)

#endif // __RTI_VC_DEVIF_H__

