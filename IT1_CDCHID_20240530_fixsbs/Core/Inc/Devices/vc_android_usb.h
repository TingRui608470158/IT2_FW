

#ifndef __VC_ANDRIOD_USB_H__
#define __VC_ANDRIOD_USB_H__

#include "libusb.h"

#ifdef __cplusplus
extern "C"{
#endif

enum IICSpeed {
    kIIC_SPEED_400k = 0,
    kIIC_SPEED_100k,
    kIIC_SPEED_200k
};

uint8_t ausb_single_read16(uint16_t regAddr);

/**
 * @brief  i2c singleRead function : register address size is 8 bits
 * @details Send I2C read command then get I2C read data
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + STOP + START + Chip_ID(7bit) + Read 1 (1bit) + ACK + rdData + NACK + STOP
 */
uint8_t ausb_single_read(uint8_t regAddr);

/**
 * @brief  i2c readBurst function
 * @details Send I2C read command then get I2C read data as len count.
 *          I2C protocol : START + Chip_ID(7bit) + Read 1(1bit) + ACK + read rdData_0 + ACK + read rdData_1 + ACK + ..... + rdData_N + NACK + STOP
 */
void ausb_read_burst(uint8_t* pData, int len);

/**
 * @brief  i2c singleWrite function : register address size is 16 bits
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr_Upper_8 + ACK + regAddr_Lower_8 + ACK + write data + ACK + STOP
 */
void ausb_single_write16(uint16_t regAddr, uint8_t data);

/**
 * @brief  i2c singleWrite function : register address size is 8 bits
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + write data + ACK + STOP
 */
void ausb_single_write(uint8_t regAddr, uint8_t data);

/**
 * @brief  i2c writeBurst function
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + write pdata_0 + ACK + write pdata_1 + ACK + ....... + ACK + STOP
 */
void ausb_write_burst(uint8_t regAddr, uint8_t* pData, int len);

void ausb_set_i2c_speed(enum IICSpeed speed);

void ausb_set_slave_chipId(uint8_t slave_chipId);

void ausb_change_i2c_port(int port_num);

void ausb_close(void);

int ausb_open(void);
	
#ifdef __cplusplus
}
#endif

#endif /* __VC_ANDRIOD_USB_H__ */
