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

/* Device interface for RDC and RDP. */

#include "rti_vc_devif.h"

#if (!defined(ECLIPSE_RCP) && !defined(__KERNEL__) && defined(__linux__) /* Linux application */)
	U8_T vc_i2c_bus_number[MAX_NUM_VC_DEVICE_CH] = {
		_CFG_I2C_BUS_NR_RDC_RDP0, /* VC_DEVICE_CH_RDC_RDP0 */
		_CFG_I2C_BUS_NR_RDC_RDP1  /* VC_DEVICE_CH_RDP1 */
	};

	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	static int rtimd_dev_fd;
	#else
	static int vc_dev_fd[MAX_NUM_VC_DEVICE_CH];
	#endif // _CFG_USE_RTIMD_KERNEL_DRIVER
#else
    U8_T vc_i2c_bus_number[MAX_NUM_VC_DEVICE_CH] = {
        VC_DEVICE_CH_RDC_RDP0, /* VC_DEVICE_CH_RDC_RDP0 */
        VC_DEVICE_CH_RDP1  /* VC_DEVICE_CH_RDP1 */
	};
#endif /* #ifdef _CFG_USE_RTI_MD_LINUX_KERNEL_I2C_DRIVER */

/**
* External functions
*/
#ifndef ECLIPSE_RCP
#ifdef __cplusplus
extern "C"{
#endif
#endif
int rtiVC_prepare_controller(void);
int rtiVC_prepare_panel(void);
#ifndef ECLIPSE_RCP
#ifdef __cplusplus
}
#endif
#endif

#if 0
#if defined(ECLIPSE_RCP)
	#include "rti_rcp_api.h"
	CxUSBListener* m_pUsbI2C = NULL;
	static E_VC_DEVICE_CH_T vc_device_ch;

#elif defined(CFG_ANDRIOD_USB)
	#include "vc_android_usb.h"

	static E_VC_DEVICE_CH_T vc_device_ch;

#elif (!defined(__KERNEL__) && defined(__linux__) /* Linux application */)

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	#include "USBIIC_Driver.h"

	static USBIIC m_pUsbI2C;
	static E_VC_DEVICE_CH_T vc_device_ch;

	#ifdef __cplusplus
	extern "C"{
	#endif
	void rtiVC_DeinitDevice(void);
	void rtiVC_InitDevice(void);
	#ifdef __cplusplus
	}
	#endif

#elif defined(__APPLE__)
    #include "MacUsbDriver.h"

    static E_VC_DEVICE_CH_T vc_device_ch;

#else
	#error "Code not present"
#endif
#endif

/**
 * @brief Changes the I2C bus number connected with RDC or RDP device.
 *
 * @param[in] ch Channel port.
 * @param[in] i2c_bus_no I2C bus number on the specified channel.
 * @return 0 on success, otherwise returns a negative number.
 *
 */
void rtiVC_ChangeI2CBusNumber(E_VC_DEVICE_CH_T ch, int i2c_bus_no)
{
	vc_i2c_bus_number[ch] = i2c_bus_no;
}

/**
 * @brief Reads N bytes from the register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] w_buf Pointer to the buffer containing the data to write to
 *                  the device before READ command in I2C protocol.
 * @param[in] w_size Number of bytes to write to the device before READ command
 *                   in I2C protocol.
 * @param[out] r_buf Pointer to the buffer that receives the data read from the device.
 * @param[in] r_size Number of bytes to be read from the device.
 *
 * @return Return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_ReadBurstDeviceExt(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
							U8_T *w_buf, U16_T w_size,
							U8_T *r_buf, U16_T r_size)
{
#if defined(ECLIPSE_RCP)
	//DMSG("rtiVC_ReadBurstDeviceExt (0x%02X)<<\n", dev_slave_addr);

	m_pUsbI2C->setSlaveChipId(dev_slave_addr);

	//DMSG("rtiVC_ReadBurstDeviceExt (0x%02X)>>\n", dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}
	m_pUsbI2C->writeBurst(w_buf[0], &w_buf[1], w_size-1);
	m_pUsbI2C->readBurst(r_buf, (U16_T *)&r_size);

#elif defined(CFG_ANDRIOD_USB)
    ausb_set_slave_chipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        ausb_change_i2c_port(vc_device_ch);
    }
    
    ausb_write_burst(w_buf[0], &w_buf[1], w_size-1);
    ausb_read_burst(r_buf, (unsigned int)r_size);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_BURST_READ_REG_T br;

	SET_RTIMD(br, dev_ch, dev_slave_addr);
	br.wbuf_addr = (uint64_t)w_buf;
	br.wsize = w_size;
	br.rsize = r_size;
	br.rbuf_addr = (uint64_t)r_buf;

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_BURST_READ, &br)) < 0) {
		EMSG("[VC] Burst register read error (%d)\n", ret);\
		return -1;
	}

	#else
	int ret;
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msgs[2] = {
		{
			.addr = dev_slave_addr,
			.flags = 0,
			.buf = w_buf,
			.len = w_size
		},
		{
			.addr = dev_slave_addr,
			.flags = I2C_M_RD,
			.buf = r_buf,
			.len = r_size
		}
	};
	
	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 2;

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0) {
		EMSG("Extension burst register read error (%d)\n", ret);
		return -1;
	}	
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	m_pUsbI2C.writeBurst(w_buf[0], &w_buf[1], w_size-1);
	m_pUsbI2C.readBurst(r_buf, (UINT *)&r_size);

#elif defined(__APPLE__)
    setSlaveChipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        changeI2CPort(vc_device_ch);
    }
    
    writeBurst(w_buf[0], &w_buf[1], w_size-1);
    readBurst(r_buf, (unsigned int)r_size);
    
#else
//	#error "Code not present"

	// CH: Test if a value of an input variable passed one byte size
	if (w_size == 2) {
		I2CReadBytes16(&RDC200A_I2C1, dev_slave_addr, ((uint16_t)w_buf[0] << 8) | (uint16_t)w_buf[1], (uint8_t*)r_buf, r_size);
	}
	else if (w_size == 1) {
		I2CReadBytes(&RDC200A_I2C1, dev_slave_addr, (uint8_t)w_buf[0], (uint8_t*)r_buf, r_size);
	}
#endif

	return 0;
}

/**
 * @brief Writes N bytes to the register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] buf Pointer to the buffer containing the data to write to the device.
 * @param[in] size Number of bytes to write to the device.
 *
 * @return 0 on success, otherwise returns a negative number.
 */
int rtiVC_WriteBurstDeviceExt(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
								U8_T *buf, U16_T size)
{
#if defined(ECLIPSE_RCP)
	m_pUsbI2C->setSlaveChipId(dev_slave_addr);
	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch =  (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}
	m_pUsbI2C->writeBurst(buf[0], &buf[1], size-1);

#elif defined(CFG_ANDRIOD_USB)
    ausb_set_slave_chipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        ausb_change_i2c_port(vc_device_ch);
    }
    
    ausb_write_burst(buf[0], &buf[1], size-1);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_BURST_WRITE_REG_T bw;

	SET_RTIMD(bw, dev_ch, dev_slave_addr);
	bw.wsize = size;
	bw.wbuf_addr = (uint64_t)buf;	

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_BURST_WRITE, &bw)) < 0) {
		EMSG("[VC] Burst register write error (%d)\n", ret);
		return -1;
	}

	#else
	int ret;	
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msgs = {
		.addr = dev_slave_addr,
		.flags = 0,
		.buf = buf,
		.len = size
	};
	
	ioctl_data.msgs = &msgs;
	ioctl_data.nmsgs = 1;

//	DMSG("dev_slave_addr(0x%02X)\n", dev_slave_addr);

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0) {
		EMSG("Burst register write error (%d)\n", ret);
		return -1;
	}
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	m_pUsbI2C.writeBurst(buf[0], &buf[1], size-1);

#elif defined(__APPLE__)
    setSlaveChipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        changeI2CPort(vc_device_ch);
    }
    
    writeBurst(buf[0], &buf[1], size-1);
    
#else
//	#error "Code not present"
	uint16_t timeout = 0;

	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (dev_slave_addr << 1) | I2C_WRITE , (uint8_t*)buf, size, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
#endif

	return 0;
}

/**
 * @brief Read a byte from the 16-bit register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] reg Register address to be accessed.
 *
 * @return A byte read from device.
 *
 */
U8_T rtiVC_ReadDevice16(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr, U16_T reg)
{
	U8_T rbuf[1] = {0xFF};

#if defined(ECLIPSE_RCP)
	U16_T r_size = 1;
	U8_T wbuf[2] = {0xFF, 0xFF};

	m_pUsbI2C->setSlaveChipId(dev_slave_addr);
	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}

	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;

	m_pUsbI2C->writeBurst(wbuf[0], &wbuf[1], 1);
	m_pUsbI2C->readBurst(rbuf, (U16_T *)&r_size);

#elif defined(CFG_ANDRIOD_USB)
   int r_size = 1;
   uint8_t wbuf[2] = {0xFF, 0xFF};

   ausb_set_slave_chipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        ausb_change_i2c_port(vc_device_ch);
    }

    //rbuf[0] = singleRead16(reg);
	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	  
	ausb_write_burst(wbuf[0], &wbuf[1], 1);
	ausb_read_burst(rbuf, r_size);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_SINGLE_READ_REG_T sr;

	SET_RTIMD(sr, dev_ch, dev_slave_addr);
	sr.reg_size = 2;
	sr.reg_addr = reg;
	sr.rbuf_addr = (uint64_t)rbuf;

	//DMSG("rtiVC_ReadDevice: hdr.bus_num(%d) reg(0x%02X)\n", sr.hdr.bus_num, reg);

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_SINGLE_READ, &sr)) < 0) {	
		EMSG("[VC] ioctl Single register read error (%d)\n", ret);
		return 0xFF;
	}
	
	#else
	int ret;
	U8_T wbuf[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msgs[2] = {
		{
			.addr = dev_slave_addr,
			.flags = 0,
			.buf = wbuf,
			.len = 2
		},
		{
			.addr = dev_slave_addr,
			.flags = I2C_M_RD,
			.buf = rbuf,
			.len = 1
		}
	};

	wbuf[0] = (reg >> 8) & 0xFF;
	wbuf[1] = reg & 0xFF;

	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 2;

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0) {	
		EMSG("[VC] Single register read error (%d)\n", ret);
		return 0xFF;
	}	
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	UINT r_size = 1;
	U8_T wbuf[2] = {0xFF, 0xFF};

	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	//rbuf[0] = m_pUsbI2C.read16(reg);
	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	  
	m_pUsbI2C.writeBurst(wbuf[0], &wbuf[1], 1);
	m_pUsbI2C.readBurst(rbuf, &r_size);

#elif defined(__APPLE__)
   UInt32 r_size = 1;
   UInt8 wbuf[2] = {0xFF, 0xFF};

   setSlaveChipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        changeI2CPort(vc_device_ch);
    }

    //rbuf[0] = singleRead16(reg);
	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	  
	writeBurst(wbuf[0], &wbuf[1], 1);
	readBurst(rbuf, (unsigned int)r_size);

#else
//	#error "Code not present"
	I2CReadBytes16(&RDC200A_I2C1, (uint16_t)dev_slave_addr, (uint16_t)reg, (uint8_t*)rbuf, 1);
#endif

	return rbuf[0];
}

/**
 * @brief Read a byte from the 8-bit register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] reg Register address to be accessed.
 *
 * @return A byte read from device.
 *
 */
U8_T rtiVC_ReadDevice(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr, U8_T reg)
{
	U8_T rbuf[1] = {0xFF};

#if defined(ECLIPSE_RCP)
	m_pUsbI2C->setSlaveChipId(dev_slave_addr);
	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}

	rbuf[0] = m_pUsbI2C->read(reg);

#elif defined(CFG_ANDRIOD_USB)
   ausb_set_slave_chipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        ausb_change_i2c_port(vc_device_ch);
    }
    
    rbuf[0] = ausb_single_read(reg);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_SINGLE_READ_REG_T sr;

	SET_RTIMD(sr, dev_ch, dev_slave_addr);
	sr.reg_size = 1; // RDC200 has 1 byte width register.
	sr.reg_addr = reg;
	sr.rbuf_addr = (uint64_t)rbuf;

	//DMSG("rtiVC_ReadDevice: hdr.bus_num(%d) reg(0x%02X)\n", sr.hdr.bus_num, reg);

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_SINGLE_READ, &sr)) < 0) {	
		EMSG("[VC] ioctl Single register read error (%d)\n", ret);
		return 0xFF;
	}
	
	#else
	int ret;
	U8_T wbuf[1] = {reg};
	struct i2c_rdwr_ioctl_data ioctl_data;
	struct i2c_msg msgs[2] = {
		{
			.addr = dev_slave_addr,
			.flags = 0,
			.buf = wbuf,
			.len = 1
		},
		{
			.addr = dev_slave_addr,
			.flags = I2C_M_RD,
			.buf = rbuf,
			.len = 1
		}
	};

	ioctl_data.msgs = msgs;
	ioctl_data.nmsgs = 2;

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0) {	
		EMSG("[VC] Single register read error (%d)\n", ret);
		return 0xFF;
	}	
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	rbuf[0] = m_pUsbI2C.read(reg);

#elif defined(__APPLE__)
   setSlaveChipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        changeI2CPort(vc_device_ch);
    }
    
    rbuf[0] = singleRead(reg);
#else
//	#error "Code not present"
	I2CReadBytes(&RDC200A_I2C1, (uint16_t)dev_slave_addr, (uint8_t)reg, (uint8_t*)rbuf, 1);
#endif

	return rbuf[0];
}

/**
 * @brief Writes a byte to the 16-bit register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] reg 16-bit Register address to be accessed.
 * @param[in] val Data value to be written.
 */
void rtiVC_WriteDevice16(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
						U16_T reg, U8_T val)
{
#if defined(ECLIPSE_RCP)
	U8_T wbuf[3] = {0xFF,0xFF,0xFF};

	m_pUsbI2C->setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}

	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	wbuf[2] = val;
	m_pUsbI2C->writeBurst(wbuf[0], &wbuf[1], 2);

#elif defined(CFG_ANDRIOD_USB)
	U8_T wbuf[3] = {0xFF,0xFF,0xFF};

	ausb_set_slave_chipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		ausb_change_i2c_port(vc_device_ch);
	}

	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	wbuf[2] = val;
	ausb_write_burst(wbuf[0], &wbuf[1], 2);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_SINGLE_WRITE_REG_T sw;

	SET_RTIMD(sw, dev_ch, dev_slave_addr);
	sw.reg_size = 2;
	sw.reg_addr = reg;
	sw.data = val;

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_SINGLE_WRITE, &sw)) < 0)
		EMSG("[VC] Single register write error (%d)\n", ret);

#else
	int ret;
	struct i2c_rdwr_ioctl_data ioctl_data;
	U8_T wbuf[3];
	struct i2c_msg msg = {
		.addr = dev_slave_addr, .flags = 0, .len = 3, .buf = wbuf
	};

	wbuf[0] = (reg >> 8) & 0xFF;
	wbuf[1] = reg & 0xFF;
	wbuf[2] = val;

	ioctl_data.msgs = &msg;
	ioctl_data.nmsgs = 1;

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0)
		EMSG("[VC] Single register write error (%d)\n", ret);

//	DMSG("[RDC] reg wr: reg(0x%02X), val(0x%02X)\n", reg, val); 
#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	U8_T wbuf[3] = {0xFF,0xFF,0xFF};

	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	//m_pUsbI2C.write16(reg, val);

	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	wbuf[2] = val;
	m_pUsbI2C.writeBurst(wbuf[0], &wbuf[1], 2);

#elif defined(__APPLE__)
	UInt8 wbuf[3] = {0xFF,0xFF,0xFF};

	setSlaveChipId(dev_slave_addr);
	
	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		changeI2CPort(vc_device_ch);
	}
	
    //singleWrite16(reg, val);
    
	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	wbuf[2] = val;
	writeBurst(wbuf[0], &wbuf[1], 2);

#else
//	#error "Code not present"
	uint16_t timeout = 0;
#if 0
	if(HAL_I2C_Mem_Write(&RDC200A_I2C1, (dev_slave_addr << 1)&0xFE, (uint16_t)reg, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&val, sizeof(val), I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
#else
	uint8_t data[3];
	uint16_t size = 3;

	data[0] = (uint8_t)((reg&0xff00)>>8);
	data[1] = (uint8_t)reg&0xff;
	data[2] = (uint8_t) (val & 0xFF);

	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (dev_slave_addr << 1) | I2C_WRITE , (uint8_t*)&data, size, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
#endif
#endif
}

/**
 * @brief Writes a byte to the 8-bit register of the device.
 *
 * @param[in] dev_ch Channel for the path to access the devices (RDC, RDP0 or RDP1).
 * @param[in] dev_slave_addr 7-bit I2C slave address.
 * @param[in] reg Register address to be accessed.
 * @param[in] val Data value to be written.
 */
void rtiVC_WriteDevice(E_VC_DEVICE_CH_T dev_ch, U8_T dev_slave_addr,
						U8_T reg, U8_T val)
{
#if defined(ECLIPSE_RCP)
	m_pUsbI2C->setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C->changeI2CPort(vc_device_ch);
	}
	m_pUsbI2C->write(reg, val);

#elif defined(CFG_ANDRIOD_USB)
	ausb_set_slave_chipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		ausb_change_i2c_port(vc_device_ch);
	}
	ausb_single_write(reg, val);


#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	int ret;
	RTIMD_SINGLE_WRITE_REG_T sw;

	SET_RTIMD(sw, dev_ch, dev_slave_addr);
	sw.reg_size = 1;
	sw.reg_addr = reg;
	sw.data = val;

	if ((ret = ioctl(rtimd_dev_fd, IOCTL_RTIMD_SINGLE_WRITE, &sw)) < 0)
		EMSG("[VC] Single register write error (%d)\n", ret);

	#else
	int ret;
	struct i2c_rdwr_ioctl_data ioctl_data;
	U8_T wbuf[2] = {reg, val};
	struct i2c_msg msg = {
		.addr = dev_slave_addr, .flags = 0, .len = 2, .buf = wbuf
	};

	ioctl_data.msgs = &msg;
	ioctl_data.nmsgs = 1;

	if ((ret = ioctl(vc_dev_fd[dev_ch], I2C_RDWR, &ioctl_data)) < 0)
		EMSG("[VC] Single register write error (%d)\n", ret);

//	DMSG("[RDC] reg wr: reg(0x%02X), val(0x%02X)\n", reg, val);	
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	m_pUsbI2C.setSlaveChipId(dev_slave_addr);

	if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
		vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
		m_pUsbI2C.changeI2CPort(vc_device_ch);
	}

	m_pUsbI2C.write(reg, val);

#elif defined(__APPLE__)
    setSlaveChipId(dev_slave_addr);
    
    if (vc_device_ch != (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch]) {
        vc_device_ch = (E_VC_DEVICE_CH_T)vc_i2c_bus_number[dev_ch];
        changeI2CPort(vc_device_ch);
    }
    
    singleWrite(reg, val);
#else
//	#error "Code not present"
	uint16_t timeout = 0;

	uint8_t data[2];
	uint16_t size = 2;

	data[0] = (uint8_t)reg&0xff;
	data[1] = (uint8_t) (val & 0xFF);

	if(HAL_I2C_Master_Transmit(&RDC200A_I2C1, (dev_slave_addr << 1) | I2C_WRITE , (uint8_t*)&data, size, I2C_TIMEOUT) != HAL_OK)
	{
		Error_Handler();
	}
	while (HAL_I2C_GetState(&RDC200A_I2C1) != HAL_I2C_STATE_READY && timeout++<I2C_TIMEOUT) HAL_Delay(10);
#endif
}

static void devif_close_device(void)
{
#if 0
#if defined(ECLIPSE_RCP)
	m_pUsbI2C->closeUSB();

#elif defined(CFG_ANDRIOD_USB)
	ausb_close();

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	if (rtimd_dev_fd >= 0) {
		close(rtimd_dev_fd);
		rtimd_dev_fd = -1;		
		DMSG("[VC] %s device closed\n", RTI_MD_DEV_NAME);
	}
	#else
	int dev_ch;

	for (dev_ch = 0; dev_ch < MAX_NUM_VC_DEVICE_CH; dev_ch++) {
		if (vc_dev_fd[dev_ch] >= 0) {
			close(vc_dev_fd[dev_ch]);
			vc_dev_fd[dev_ch] = -1;		
			DMSG("[VC] %s%d device closed\n", VC_LINUX_DEVICE_PATH_PREFIX, vc_i2c_bus_number[dev_ch]);
		}
	}	
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	m_pUsbI2C.closeUSB();

#elif defined(__APPLE__)
    IoKitCloseUSB();
#else
	#error "Code not present"
#endif
#endif
}

/**
 * @brief Close the interface for the RDC and RDP devices.
 *
 */
void rtiVC_CloseDevice(void)
{
	devif_close_device();
}

/**
 * @brief Open the interface for the RDC and RDP devices.
 *
 * @return 0 on success, otherwise returns a negative number.
 *
 */
int rtiVC_OpenDevice(void)
{
#if 0
#if defined(ECLIPSE_RCP)
	if (m_pUsbI2C->openUSB(1) == FALSE) { //USBIIC::SILAB_C8051
		EMSG("Can't open IIC USB interface!\n Check USB connector or USB voltage\n");
		return -1;
	}

	m_pUsbI2C->setSpeed(1); //USBIIC::kIIC_SPEED_100k

#elif defined(CFG_ANDRIOD_USB)
	if (ausb_open() != 0) {
		EMSG("Can't Open IIC USB interface!\n Check USB connector or USB voltage\n");
		return -1;
	}

	ausb_set_i2c_speed(kIIC_SPEED_400k);

#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	if (rtimd_dev_fd >= 0) {
		DMSG("[VC] Already device opened: %s\n", RTI_MD_DEV_NAME);
		return 0;
	}

	rtimd_dev_fd = open(RTI_MD_DEV_NAME, O_RDWR);
	if (rtimd_dev_fd < 0) {
		EMSG("[VC] Can't open device: %s\n", RTI_MD_DEV_NAME);
		return -1;
	}

	DMSG("[VC] %s device opened\n", RTI_MD_DEV_NAME);

	#else
	int dev_ch;
	char devname[64];

	for (dev_ch = 0; dev_ch < MAX_NUM_VC_DEVICE_CH; dev_ch++) {
		if (vc_dev_fd[dev_ch] < 0) {
			sprintf(devname, "%s%d", VC_LINUX_DEVICE_PATH_PREFIX, vc_i2c_bus_number[dev_ch]);
		
			vc_dev_fd[dev_ch] = open(devname, O_RDWR);
			if (vc_dev_fd[dev_ch] < 0) {
				EMSG("[VC] Can't open device: %s\n", devname);
				devif_close_device();
				return -1;
			}
			else
				DMSG("[VC] %s device opened\n", devname);
		}
		else
			DMSG("[VC] Already device opened: %s\n", devname);
	}	
	#endif
	
#elif (defined(_WIN32)||defined(_WINDOWS) | defined(__APPLE__)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	if (m_pUsbI2C.openUSB() == FALSE) {
		EMSG("Can't Open IIC USB interface!\n Check USB connector or USB voltage\n");
		return -1;
	}

	m_pUsbI2C.setSpeed(USBIIC::kIIC_SPEED_100k);

#elif defined(__APPLE__)
    if (IoKitOpenUSB() != 0) {
        EMSG("MAC : Can't Open IIC USB interface!\n Check USB connector or USB voltage\n");
        return -1;
    }
    
    setI2cSpeed(kIIC_SPEED_400k);

#else
	#error "Code not present"
#endif

#if !defined(ECLIPSE_RCP)
	if(rtiVC_prepare_controller() < 0) return -1;
	if(rtiVC_prepare_panel() < 0 ) return -1;
#endif
#endif

	return 0; /* On success. */
}

/**
 * @brief Deinitializes components of device interface.
 */
void rtiVC_DeinitDevice(void)
{
	devif_close_device();
}

/**
 * @brief Initializes components of device interface.
 */
void rtiVC_InitDevice(void)
{
#if 0
#if defined(ECLIPSE_RCP)
	vc_device_ch = VC_DEVICE_CH_INVALID;

#elif defined(CFG_ANDRIOD_USB)


#elif !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#ifdef _CFG_USE_RTIMD_KERNEL_DRIVER
	rtimd_dev_fd = -1;
	#else
	int dev_ch;

	for (dev_ch = 0; dev_ch < MAX_NUM_VC_DEVICE_CH; dev_ch++)
		vc_dev_fd[dev_ch] = -1;
	#endif

#elif (defined(_WIN32)||defined(_WINDOWS) || defined(__APPLE__)) && !defined(CFG_CONSOLE_APP)

#elif defined(_WIN32)
	vc_device_ch = VC_DEVICE_CH_INVALID; // default;
    
#elif defined(__APPLE__)
    vc_device_ch = VC_DEVICE_CH_INVALID; // default;
#else
	#error "Code not present"
#endif
#endif
}

#ifdef ECLIPSE_RCP
#include "rti_rcp_api.h"
void rtiVC_SetListener(CxUSBListener* lplistener)
{
	m_pUsbI2C = (CxUSBListener *)lplistener;
}

#endif

