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

#include <math.h>
#include "rti_vc_rdc.h"
#include "rti_vc_panel.h"

#if defined(CFG_MDC_RDC200A)
#include "rdc200a_flash_partitions.h"
#include "rdc200a_internal.h"

#if defined(_WIN32) || defined(WINDOWS)
#pragma pack(push, 1)
#endif

/* Except slave address */

/* FLASH Write Enable message hader. */
typedef struct {
	U8_T reg_addr_h;
	U8_T reg_addr_l;
	U8_T flash_cmd;
} GNUPACKED RDC200A_FLASH_WREN_MSG_HDR_T;

/* FLASH Read Data Bytes message header. */
typedef struct {
	U8_T reg_addr_h;
	U8_T reg_addr_l;
	U8_T flash_cmd;
	U8_T flash_addr_h;
	U8_T flash_addr_m;
	U8_T flash_addr_l;
} GNUPACKED RDC200A_FLASH_READ_MSG_HDR_T;

/* FLASH Page Program message header. */
typedef struct {
	U8_T reg_addr_h;
	U8_T reg_addr_l;
	U8_T flash_cmd;
	U8_T flash_addr_h;
	U8_T flash_addr_m;
	U8_T flash_addr_l;
} GNUPACKED RDC200A_FLASH_PP_MSG_HDR_T;

/* FLASH Erase Sector message header. */
typedef struct {
	U8_T reg_addr_h;
	U8_T reg_addr_l;
	U8_T flash_cmd;
	U8_T flash_addr_h;
	U8_T flash_addr_m;
	U8_T flash_addr_l;
} GNUPACKED RDC200A_FLASH_SE_MSG_HDR_T;

#if defined(_WIN32) || defined(WINDOWS) || defined(_WIN64)
#pragma pack(pop)
#endif

#define MAX_DEV_ACCESS_RETRY_CNT		240

#define RDC200_FLASH_SR_WIP	    0x01
#define RDC200_FLASH_SR_WEL     0x02
#define RDC200_FLASH_SR_BP0     0x04
#define RDC200_FLASH_SR_BP1     0x08
#define RDC200_FLASH_SR_BP2     0x10
#define RDC200_FLASH_SR_BP3     0x20
#define RDC200_FLASH_SR_QE      0x40
#define RDC200_FLASH_SR_SRWD    0x80
#define RDC200_FLASH_SR_BP3_0\
	(RDC200_FLASH_SR_BP0 | RDC200_FLASH_SR_BP1 | RDC200_FLASH_SR_BP2 | RDC200_FLASH_SR_BP3)

typedef enum {
    CMD_WRITE_STATUS_REG = 0x01,
    CMD_PROGRAM_PAGE = 0x02,
    CMD_READ = 0x03,
    CMD_WRITE_DISABLE = 0x04,

    CMD_READ_STATUS_REG1 = 0x05,

	CMD_WRITE_ENABLE = 0x06,

	CMD_SECTOR_ERASE_4K = 0x20,
	
	CMD_BLOCK_ERASE1_32K = 0x52,
	CMD_BLOCK_ERASE2_32K = 0x53,
	CMD_BLOCK_ERASE_64K = 0xD8,
    CMD_BULK_ERASE_0 = 0x60,

	CMD_READ_DEVICE_ID = 0x9F,
    CMD_RESUME_FROM_PWRDOWN = 0xAB,
    CMD_POWER_DOWN = 0xB9,
	CMD_BULK_ERASE_1 = 0xC7
} FLASH_COMMAND;

#define RDC200A_FLASH_REG_ADDR_H	0x00
#define RDC200A_FLASH_REG_ADDR_L	0x81

/* FLASH command positions. */
#define RDC200A_FCMD_POS_REG_ADDR_H		0
#define RDC200A_FCMD_POS_REG_ADDR_L		1
#define RDC200A_FCMD_POS_CMD			2

#define RDC200A_FCMD_POS_FADDR_H		3
#define RDC200A_FCMD_POS_FADDR_M		4
#define RDC200A_FCMD_POS_FADDR_L		5
#define RDC200A_FCMD_POS_WDATA			6

#define MAX_PAGE_PROGRAM_TIMEOUT		3 // ms
#define MAX_ERASE_SECTOR_TIMEOUT		250
#define MAX_ERASE_CHIP_TIMEOUT			60*1000
#define MAX_WRITE_STATUS_REG_TIMEOUT	50

static U8_T flash_cmdb[264];

static int wait_until_spi_master_ready(void)
{
	U16_T reg_0x0092;
	UINT_T retry_cnt = MAX_DEV_ACCESS_RETRY_CNT;

	while (1) {
		reg_0x0092 = RDC_REG_GET(0x0092);		
		if (reg_0x0092 & 0x40) { /* MON_SPI_MASTER_RDY_NBUSY */
			//DMSG("OK");
			return 0;
		}

		if (--retry_cnt == 0)
			break;		
	}

	EMSG("FLASH: SPI master ready Timeout!\n"); 

	return -1;	
}

static int write_status_register(U8_T new_sr_val)
{
	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_WRITE_STATUS_REG;
	flash_cmdb[3] = new_sr_val;
	
	RDC200A_REG_BURST_SET(flash_cmdb, 4);

	return 0;
}

static U8_T read_status_register(void)
{
	U8_T sr_buf[2];

	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_READ_STATUS_REG1;
	RDC200A_REG_BURST_SET(flash_cmdb, 4); // Include "DummyForRead"

	RDC200A_REG_BURST_GET(flash_cmdb, 2, sr_buf, 2);

	//DMSG("RDSR: 0x%02X\n", sr_buf[1]);

	return sr_buf[1];
}

static int read_identification(void)
{
	U8_T id_buf[4];

	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_READ_DEVICE_ID;

	RDC200A_REG_BURST_SET(flash_cmdb, 6); // Include DummyForRead0 ~ DummyForRead2

	RDC200A_REG_BURST_GET(flash_cmdb, 2, id_buf, 4); /* Dummy(CMD) + 3 bytes */

	DMSG("Flash Device ID : 0x%02X 0x%02X 0x%02X\n",
	                    id_buf[1], id_buf[2], id_buf[3]);
    
    return 0;
}

static int read_data_bytes(U32_T addr, U8_T *buf, UINT_T len)
{	
	static U8_T temp[260];	

	/* Direct control Mode. */
	int ret = 0;
	UINT_T retry_cnt = 100;
	U8_T reg_0x0061;
	U8_T temp_reg_val;
	U8_T direct_ctrl_reg_buf[8]; /* 0x005A ~ 0x005F */
	U16_T FLASH_TX_DATA_NUM;

	FLASH_TX_DATA_NUM = 4/*cmd + 3 addr*/ + len;

	direct_ctrl_reg_buf[0] = 0x00;
	direct_ctrl_reg_buf[1] = 0x5A;
	direct_ctrl_reg_buf[2] = (1<<7) | (0<<6) | (FLASH_TX_DATA_NUM>>8); /* 0x005A */
	direct_ctrl_reg_buf[3] = (U8_T)(FLASH_TX_DATA_NUM & 0xFF); /* 0x005B */
	direct_ctrl_reg_buf[4] = CMD_READ; /* 0x005C */
	direct_ctrl_reg_buf[5] = (addr >> 16) & 0xFF; /* 0x005D */
	direct_ctrl_reg_buf[6] = (addr >> 8) & 0xFF; /* 0x005E */
	direct_ctrl_reg_buf[7] = addr & 0xFF; /* 0x005F */

	temp_reg_val &= ~0x07; /* Clear [2:0] bits. */

	/* SPI_BRIDGE_EN	FLASH_MEM_CTRL_EN	FLASH_TO_LUT_EN */
	temp_reg_val = ((1<<2) | (1<<1) | (0));	
	RDC_REG_SET(0x0051, temp_reg_val);

	RDC200A_REG_BURST_SET(direct_ctrl_reg_buf, 8);

	reg_0x0061 = RDC_REG_GET(0x0061);
	reg_0x0061 &= ~(1<<3); // FLASH_DIRECT_START
	RDC_REG_SET(0x0061, reg_0x0061);

	reg_0x0061 |= (1<<3); // FLASH_DIRECT_START
	RDC_REG_SET(0x0061, reg_0x0061);
	
	while (1) {
		ret = wait_until_spi_master_ready();
		if (ret == 0) 
			break;

		if (--retry_cnt == 0) {
			EMSG("FLASH: Read data bytes timeout!\n");
			ret = -2;
			break;
		}
	}

	/* I2C to SPI bridge Mode */
	/* SPI_BRIDGE_EN	FLASH_MEM_CTRL_EN	FLASH_TO_LUT_EN */
	temp_reg_val = ((1<<2) | (0<<1) | (0));	
	RDC_REG_SET(0x0051, temp_reg_val);

	if (ret)
		return ret;

	RDC200A_REG_BURST_GET(flash_cmdb, 2, temp, 4/*DummyData*/ + len);

	memcpy(buf, &temp[4], len);

	return 0;
}

// len: Should less than the size of page.
//Program a page of data to the flash whithin the same page.
static int program_page(U32_T flash_addr, const void *pgm_buf, UINT_T len)
{
	U16_T burst_size = sizeof(RDC200A_FLASH_PP_MSG_HDR_T) + len;
	
	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_PROGRAM_PAGE; 
	flash_cmdb[RDC200A_FCMD_POS_FADDR_H] = (flash_addr >> 16) & 0xFF;
	flash_cmdb[RDC200A_FCMD_POS_FADDR_M] = (flash_addr >> 8) & 0xFF; 
	flash_cmdb[RDC200A_FCMD_POS_FADDR_L] = (flash_addr & 0xFF);

	memcpy(&flash_cmdb[sizeof(RDC200A_FLASH_PP_MSG_HDR_T)], pgm_buf, len);

	RDC200A_REG_BURST_SET(flash_cmdb, burst_size);

	return len;
}

static int sector_erase(U32_T sector_addr)
{
	U16_T burst_size = sizeof(RDC200A_FLASH_SE_MSG_HDR_T);

	// SE (Sector Erase)		
	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_SECTOR_ERASE_4K; 
	flash_cmdb[RDC200A_FCMD_POS_FADDR_H] = (sector_addr >> 16) & 0xFF;
	flash_cmdb[RDC200A_FCMD_POS_FADDR_M] = (sector_addr >> 8) & 0xFF; 
	flash_cmdb[RDC200A_FCMD_POS_FADDR_L] = (sector_addr & 0xFF);

	RDC200A_REG_BURST_SET(flash_cmdb, burst_size);

	return 0;
}

static int write_enable(void)
{
	U8_T sr;
	UINT_T retry_cnt = 100;
//	int ret;

	while (1) {
		// WREN (Write ENable)
		flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_WRITE_ENABLE; 
		RDC200A_REG_BURST_SET(flash_cmdb, 3);

#if 0
		ret = wait_until_spi_master_ready();
		if (ret)
			return ret;
#endif

		sr = read_status_register();
		if (sr & RDC200_FLASH_SR_WEL) {
			//DMSG("OK");
			return 0;
		}
	
		if (--retry_cnt == 0) {
			EMSG("FLASH: WREN Timeout!\n"); 
			goto write_enable_err;
		}
	}

write_enable_err:

	return -1;
}

// Wait until the device is not in progress of program/erase/write status.
static int wait_until_write_ready(UINT_T milliseconds)
{
	int ret;
	U8_T fsr;

	ret = wait_until_spi_master_ready();
	if (ret)
		return ret;

	while (1) {
		fsr = read_status_register();
		if ((fsr & RDC200_FLASH_SR_WIP) == 0x0)
			return 0;

		if (--milliseconds == 0)
			break;

		VC_M_DELAY(1);
	}

	EMSG("FLASH: WIP Timeout!\n"); 

	return -1;
}

static int wait_until_write_latch(void)
{
	U8_T sr;
	UINT_T retry_cnt = 100;

	while (1) {
		// WREN (Write ENable)
		flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_WRITE_ENABLE; 
		RDC200A_REG_BURST_SET(flash_cmdb, 3);

		sr = read_status_register();
		if (sr & RDC200_FLASH_SR_WEL) {
			//DMSG("OK");
			return 0;
		}
	
		if (--retry_cnt == 0) {
			EMSG("FLASH: WREN Timeout!\n"); 
			goto write_enable_err;
		}
	}

write_enable_err:

	return -1;
}

static int rdc200a_flash_read(U32_T addr, U8_T *buf, UINT_T len)
{
	int ret;
	unsigned int bytes_to_read = 0;

	if (buf == NULL || len == 0) {
		EMSG("Read: Invalid parameters\n");
		return -1;
	}

	while (len) {
		// by RDC200A spec. Max FLASH_TX_DATA_NUM is 260. (cmd - 260 = 256)
		bytes_to_read = MIN(len, 256);

		ret = read_data_bytes(addr, buf, bytes_to_read);
		if (ret)
			return -2;

		buf += bytes_to_read;
		addr += bytes_to_read;
		len -= bytes_to_read;
	}

	return 0;
}

static int program_data_bytes(U32_T flash_addr, const U8_T *buf, UINT_T len)
{
	int written;
	int ret = 0;
	size_t retlen = 0;	
	unsigned int page_offset, page_remain, i;

	for (i = 0; i < len; ) {
		page_offset = (flash_addr + i) & (FLASH_PAGE_SIZE - 1);

		/* The size of data remaining on the first page */
		page_remain = MIN(FLASH_PAGE_SIZE - page_offset, len - i);

		ret = write_enable();
		if (ret)
			goto write_err;

		ret = program_page(flash_addr + i, buf + i, page_remain);
		//DMSG("flash_addr + i: 0x%X, page_remain: %u\n", flash_addr + i, page_remain);
		if (ret < 0)
			goto write_err;
		written = ret;

		ret = wait_until_write_ready(MAX_PAGE_PROGRAM_TIMEOUT);
		if (ret)
			goto write_err;

		retlen += written;
		i += written;
		if (written != page_remain) {
			EMSG("While writing %u bytes written %d bytes\n", page_remain, written);
			ret = -2;
			goto write_err;
		}
	}

write_err:

	return ret;
}

static int rdc200a_flash_write(U32_T flash_addr, const U8_T *buf, UINT_T len)
{
	int ret;

	if (buf == NULL || len == 0) {
		EMSG("Write: Invalid parameters\n");
		return -1;
	}

	RDC_REG_SET(0x0051, 0x04);

	ret = program_data_bytes(flash_addr, buf, len);
	if (ret)
		return -2;

	return 0;
}

static int rdc200a_erase_sector(U32_T sector_addr)
{
	int ret;
	U8_T sr;

	RDC_REG_SET(0x0051, 0x04);

	sr = read_status_register();
	if (sr & (RDC200_FLASH_SR_SRWD | RDC200_FLASH_SR_BP3_0)) {
		EMSG("[FLASH] Protect enabled 0x%02X\n", sr);
		return -1;
	}

	ret = write_enable();
	if (ret)
		goto erase_err;

	ret = sector_erase(sector_addr);
	if (ret)
		goto erase_err;

	ret = wait_until_write_ready(MAX_ERASE_SECTOR_TIMEOUT);
	if (ret)
		goto erase_err;

	return 0;

erase_err:
	return ret;
}

static int rdc200a_erase_chip(void)
{
	int ret;

	RDC_REG_SET(0x0051, 0x04);

	ret = write_enable();
	if (ret)
		goto erase_chip_err;

	flash_cmdb[RDC200A_FCMD_POS_CMD] = CMD_BULK_ERASE_0;	
	RDC200A_REG_BURST_SET(flash_cmdb, 3);

	ret = wait_until_write_ready(MAX_ERASE_CHIP_TIMEOUT);
	if (ret)
		goto erase_chip_err;

#if 0 // check erased data
{
	int i;
	static U8_T tmp[100];

	if (rdc200a_flash_read(0x0, tmp, 100) == 0) {
		DMSG("Chip erase result\n");

		for (i = 0; i < 100; i++) {
			if (((i+1) % 16) == 0)
				DMSG("\n");
			else
				DMSG("0x%02X ", tmp[i]);
		}
	}
}
#endif

	return 0;

erase_chip_err:
	return ret;
}

static int rdc200a_flash_disable_protect(void)
{
	int ret;
    
	DMSG("rdc200a_flash_disable_protect : 0x%02X\n", read_status_register());

	RDC_REG_SET(0x0051, 0x04);

	ret = write_enable();
	if (ret)
		goto disable_protect_err;

	ret = write_status_register(0x00);
	if (ret)
		goto disable_protect_err;

	ret = wait_until_write_ready(MAX_WRITE_STATUS_REG_TIMEOUT);
	if (ret)
		goto disable_protect_err;

	return 0;

disable_protect_err:

	return ret;
}

static int rdc200a_flash_enable_protect(void)
{
	int ret;

	RDC_REG_SET(0x0051, 0x04);

	ret = write_enable();
	if (ret)
		goto enable_protect_err;

	ret = write_status_register(RDC200_FLASH_SR_SRWD | RDC200_FLASH_SR_BP3_0);
	if (ret)
		goto enable_protect_err;

	ret = wait_until_write_ready(MAX_WRITE_STATUS_REG_TIMEOUT);
	if (ret)
		goto enable_protect_err;

	return 0;

    DMSG("rdc200a_flash_enable_protect : 0x%02X\n", read_status_register());
    
	return 0;

enable_protect_err:

	return ret;
}

static int rdc200a_flash_prepare(void)
{
	int ret;

	RDC_REG_SET(0x0051, 0x04);

	ret = read_identification();
	if (ret)
		return ret;

#if 0
{
	int i;
	static U8_T rbuf[128];
	
	rdc200a_flash_read(0x010000, rbuf, 64);

	printf("");
	printf("");
	printf("");
	printf("");
}
#endif

	ret = rdc200a_flash_disable_protect();
	if (ret)
		return ret;

	return 0;
}

static int rdc200a_flash_finish(void)
{
	rdc200a_flash_enable_protect();

	return 0;
}

int RDC200A_ProbeFLASH(VC_RDC_DEV_INFO_T *dev)
{
	dev->sector_size_shift = 12;
	//DMSG("SECTOR SIZE: 0x%X\n", 1 << dev->sector_size_shift);

	dev->flash_prepare = rdc200a_flash_prepare;
	dev->flash_finish = rdc200a_flash_finish;

	dev->flash_erase_sector = rdc200a_erase_sector;
	dev->flash_erase_chip = rdc200a_erase_chip;
	dev->flash_write = rdc200a_flash_write;
	dev->flash_read = rdc200a_flash_read;

	/* Pre assigned the register address */
	flash_cmdb[0] = RDC200A_FLASH_REG_ADDR_H;
	flash_cmdb[1] = RDC200A_FLASH_REG_ADDR_L;

	return 0;
}
#endif // #if defined(CFG_MDC_RDC200A)

