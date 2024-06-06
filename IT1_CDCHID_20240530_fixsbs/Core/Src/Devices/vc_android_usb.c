
#include <stdio.h>

#include "libusb.h"
#include "vc_android_usb.h"

#define VENDOR_ID	0x10C4 // RAONTECH EVB vendor id
#define PRODUCT_ID	0xEA61

#define INTERFACE	0

#define IN_ENDPOINT		0x82
#define OUT_ENDPOINT	0x02
#define INTERFACE		0

#define TIMEOUT			1000

#define USB_SILAB_PROTOCOL_CMD_LENGTH 0x0A

//I2C CONTROL COMMAND 0x30 ~ 0x3F
/* Align : Command,IICSpeed,0,0,0,0,0,0,0,0 */
#define I2C_SPEED_SET_COMMAND    0x34  /* I2C Speed Set Command Macro */
/* Align : Command,port_num,0,0,0,0,0,0,0,0 */
#define I2C_CHANGE_PORT_COMMAND  0x35  /* I2C Port Change Command Macro */
/* Align : Command,PortNum,0,0,0,0,0,0,0,0 */
#define I2C_PORT_CHANGE_COMMAND  0x35  /* I2C Port Change Command Macro */
/* Align : Command,ChipId,addrSize,addrH,addrL,dataSize,data0,data1,data2,data3 */
#define I2C_WRITE_COMMAND          0x30  /* I2C Write Command Macro */
/* Align : Command,ChipId,addrSize,addrH,addrL,dataSize,data0,data1,data2,data3 */
#define I2C_READ_COMMAND          0x31  /* I2C Read Command Macro */
/* Align : Command,ChipId,addrSize,addrH,addrL,dataSize,0,0,0,0 */
#define I2C_BURST_WRITE_COMMAND  0x32  /* I2C Burst Write Command Macro */
/* Align : Command,ChipId,addrSize,addrH,addrL,dataSize,0,0,0,0 */
#define I2C_BURST_READ_COMMAND      0x33  /* I2C Burst Read Command Macro */

//SPI CONTROL COMMAND 0x40 ~ 0x4F
/* Align : SPI_SPEED_IDX,SPI_MODE_IDX,NssManual,0,0,0,0,0,0,0 */  //NssManual 1: Manual 0: Auto
#define SPI_CONFIG_SET_COMMAND   0x44  /* SPI Configuration Set Command Macro */
/* Align : Command,0,0,0,0,dataSize,0,0,0,0 */
#define SPI_WRITE_COMMAND          0x40  /* SPI Write Command Macro */
/* Align : Command,0,0,0,0,dataSize,0,0,0,0 */
#define SPI_READ_COMMAND          0x41  /* SPI Read Command Macro */
/* Align : Command,0,writeSize,wrData0,wrData1,rdSize,rdData0,rdData1,rdData2,rdData3 */
#define SPI_WRITE_READ_COMMAND   0x42  /* SPI Write,Read Commbo Macro:MAX: Write 2Bytes, Read 4Bytes */

//GPIO CONTROL COMMAND 0x50 ~ 0x5F
/* Align : Command,GPIO_PORT_NUM_IDX,GPIO_PORT_SUB_NUM_IDX,GPIO_PORT_INOUT_MODE_IDX,0,0,0,0,0,0 */
#define GPIO_PORT_INIT_COMMAND   0x54  /* GPIO Port Initialize Command Macro */
/* Align : Command,GPIO_PORT_NUM_IDX,GPIO_PORT_SUB_NUM_IDX,GPIO_OUT_PORT_SET_IDX,0,0,0,0,0,0 */
#define GPIO_PORT_WRITE_COMMAND  0x50  /* GPIO Port Set Command Macro */
/* Align : Command,GPIO_PORT_NUM_IDX,GPIO_PORT_SUB_NUM_IDX,0,0,0,0,0,0,0 */
#define GPIO_PORT_READ_COMMAND   0x51  /* GPIO Port Read Command Macro */
//Version Number Check
/* Align : Command,0,0,0,0,0,0,0,0,0 */
#define VERSION_GET_VERSION   0xFE  /* Version Number Check Macro */

#define USB_CMD_HANDLED   0x55  /* Command Completed return syntax */
#define USB_CMD_HANDLED_ERROR   0xAA  /* Command Completed return error syntax */

//FW UpGrade COMMAND 0x80 ~ 0x8F
/* Align : Command,0,0,0,0,0,0,0,0,0 */
#define FLASH_READ_LOCK_BYTE_COMMAND 0x80
/* Align : Command,Block Index,0,0,0,0,0,0,0,0 */
#define FLASH_WRITE_BLOCK_COMMAND    0x81
/* Align : Command,Block Index,0,0,0,0,0,0,0,0 */
#define FLASH_READ_BLOCK_COMMAND     0x82
/* Align : Command,Page Index,0,0,0,0,0,0,0,0 */
#define FLASH_WRITE_PAGE_COMMAND     0x83
/* Align : Command,Page Index,0,0,0,0,0,0,0,0 */
#define FLASH_READ_PAGE_COMMAND      0x84
/* Align : Command,0,0,0,0,0,0,0,0,0 */
#define RAON_EVB_RESET_COMMAND       0x85

static libusb_device **devs = NULL;
static libusb_context *ctx = NULL;
static libusb_device_handle *handle = NULL;

static uint8_t mSlaveChipId;

static const int EP_MAX_SIZE = 34;
	
static int bulk_read(uint8_t *buf, int len)
{
	int ret= 0;
	int size;
	
	ret = libusb_bulk_transfer(handle, IN_ENDPOINT, buf, len, &size, TIMEOUT);
	if (ret == LIBUSB_SUCCESS) {
		if (len != size) {
			printf("receive ==>  unexpected length returned : Target Length = %d Read Length = %d\n", len, size);
			return -1;
		}
	}
	else {
		printf("USB error : Unable to send data: Operation timed out\n");
		return -2;
	}

	return 0;
}

static int bulk_write(uint8_t *buf, int len)
{
	int ret= 0;
	int size;
	
	ret = libusb_bulk_transfer(handle, OUT_ENDPOINT, buf, len, &size, TIMEOUT);
	if (ret == LIBUSB_SUCCESS) {
		if (len != size) {
			printf("receive ==>  unexpected length returned : Target Length = %d Read Length = %d\n", len, size);
			return -1;
		}
	}
	else {
		printf("USB error : Unable to send data: Operation timed out\n");
		return -2;
	}

	return 0;
}

/**
 * @brief  i2c single read function : register address size is 16 bits
 * @details Send I2C read command then get I2C read data
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr_Upper_8 + ACK + regAddr_Lower_8 + ACK + STOP + START + Chip_ID(7bit) + Read 1 (1bit) + ACK + rdData + NACK + STOP
 */
uint8_t ausb_single_read16(uint16_t regAddr)
{
    uint8_t rdData[4] = {0xFF,};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_READ_COMMAND, mSlaveChipId,
        2/*addrSize*/, 0/*AddrH*/,0/*AddrL*/,
        0x01/*dataSize*/,0, 0,0,0}; //Data Size Max 255Bytes. recommand is under 64Bytes.

	cmd[3] = regAddr >> 8; /*AddrH*/
	cmd[4] = regAddr & 0xFF; /*AddrL*/

    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("Read Error : write size : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return (rdData[0]);
    }
    if(!bulk_read(rdData, sizeof(rdData))) {
        printf("Read Data  0x%02X 0x%02X 0x%02X 0x%02X\n",rdData[0],rdData[1],rdData[2],rdData[3]);
        return (rdData[0]);
    }
    
    return (rdData[0]);
}

/**
 * @brief  i2c singleRead function : register address size is 8 bits
 * @details Send I2C read command then get I2C read data
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + STOP + START + Chip_ID(7bit) + Read 1 (1bit) + ACK + rdData + NACK + STOP
 */
uint8_t ausb_single_read(uint8_t regAddr)
{
    uint8_t rdData[4] = {0xFF,};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_READ_COMMAND, mSlaveChipId,
        0x01/*addrSize*/, 0/*AddrH*/,(uint8_t)regAddr/*AddrL*/,
        0x01/*dataSize*/,0, 0,0,0}; //Data Size Max 255Bytes. recommand is under 64Bytes.

    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("Read Error : write size : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return (rdData[0]);
    }
    if(!bulk_read(rdData, sizeof(rdData))) {
        printf("Read Data  0x%02X 0x%02X 0x%02X 0x%02X\n",rdData[0],rdData[1],rdData[2],rdData[3]);
        return (rdData[0]);
    }
    
    return (rdData[0]);
}

/**
 * @brief  i2c readBurst function
 * @details Send I2C read command then get I2C read data as len count.
 *          I2C protocol : START + Chip_ID(7bit) + Read 1(1bit) + ACK + read rdData_0 + ACK + read rdData_1 + ACK + ..... + rdData_N + NACK + STOP
 */
void ausb_read_burst(uint8_t* pData, int len)
{
    uint8_t rdData[1] ={0xFF};
    int nDevide = 0, nRemainder = 0;
    
    //printf("readBurst Size : %d \n",len);
    
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_BURST_READ_COMMAND, mSlaveChipId,
        0x00/*addrSize*/,0x00/*AddrH*/,0x00/*AddrL*/,
        len/*dataSize*/,0,0,0,0};
    
    if (len == 0 || pData == NULL) {
        printf("[Error] length or data pointer is wrong.\n");
        return;
    }
    
    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("readBurst Error : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    nDevide = (int)(len / EP_MAX_SIZE);
    nRemainder = (int)len % EP_MAX_SIZE;
    if(len  > 0 && len <= EP_MAX_SIZE) {
        bulk_read(pData, len);
    }
    else if(len > EP_MAX_SIZE) {
        
        for(int i=0;i < EP_MAX_SIZE * nDevide;i += EP_MAX_SIZE) {
            bulk_read(pData + i, EP_MAX_SIZE);
        }
        if(nRemainder)
            bulk_read(pData + (EP_MAX_SIZE * nDevide), nRemainder);
    }

#if 0
    for(int tempcnt = 0; tempcnt < *len; tempcnt++)
        printf("readBurst Index : %d data: 0x%02X\n",tempcnt,pData[tempcnt]);
#endif

    bulk_read(rdData, sizeof(rdData)); //Get completed message.
}

/**
 * @brief  i2c singleWrite function : register address size is 16 bits
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr_Upper_8 + ACK + regAddr_Lower_8 + ACK + write data + ACK + STOP
 */
void ausb_single_write16(uint16_t regAddr, uint8_t data)
{
    uint8_t rdData[1] = {0x00};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_WRITE_COMMAND, mSlaveChipId,
        2/*addrSize*/,0/*AddrH*/,0x00/*AddrL*/,
        0x01/*dataSize*/,data,0,0,0};

	cmd[3] = regAddr >> 8; /*AddrH*/
	cmd[4] = regAddr & 0xFF; /*AddrL*/

    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("Write Error :write size : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    if(!bulk_read(rdData, sizeof(rdData)))
        printf("Hand shake Error 0x%02X\n",rdData[0]);   
}

/**
 * @brief  i2c singleWrite function : register address size is 8 bits
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + write data + ACK + STOP
 */
void ausb_single_write(uint8_t regAddr, uint8_t data)
{
    uint8_t rdData[1] = {0x00};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_WRITE_COMMAND, mSlaveChipId,
        0x01/*addrSize*/,0/*AddrH*/,regAddr/*AddrL*/,
        0x01/*dataSize*/,data,0,0,0};
    
    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("Write Error :write size : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    if(!bulk_read(rdData, sizeof(rdData))) //Get completed message.
        printf("Hand shake Error 0x%02X\n",rdData[0]);   
}

/**
 * @brief  i2c writeBurst function
 * @details Send I2C write command then get completed message.
 *          I2C protocol : START + Chip_ID(7bit) + Write 0(1bit) + ACK + regAddr + ACK + write pdata_0 + ACK + write pdata_1 + ACK + ....... + ACK + STOP
 */
void ausb_write_burst(uint8_t regAddr, uint8_t* pData, int len)
{
    uint8_t rdData[1];
    int targetLen = (int)len;
    
    //printf("writeBurst Size : %d \n",len);
    
    int nDevide = 0, nRemainder = 0;
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_BURST_WRITE_COMMAND, mSlaveChipId,
        0x01/*addrSize*/,0/*AddrH*/,regAddr/*AddrL*/,
        len/*dataSize*/,0,0,0,0};
    
    if (targetLen > EP_MAX_SIZE) {
        printf("[Error] burst length is wrong.\n");
        return;
    }

    if(!bulk_write(cmd, sizeof(cmd))) { //Send Command
        printf("writeBurst Error : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd),cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    nDevide = (int)(targetLen / EP_MAX_SIZE);
    nRemainder = (int)targetLen % EP_MAX_SIZE;
    if(targetLen > 0 && targetLen <= EP_MAX_SIZE) {
        bulk_write(pData, targetLen); //Send Write Data
    }
    else if(targetLen > EP_MAX_SIZE) {
        for(int i=0;i < EP_MAX_SIZE * nDevide;i += EP_MAX_SIZE) {
            bulk_write(pData + i, EP_MAX_SIZE);
        }
        if(nRemainder)
            bulk_write(pData + (EP_MAX_SIZE * nDevide), nRemainder);
    }

    bulk_read(rdData, sizeof(rdData)); //Get completed message.
}

void ausb_set_i2c_speed(enum IICSpeed speed)
{
    uint8_t rdData[1] = {0x00};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_SPEED_SET_COMMAND,speed,0,0,0,0,0,0,0,0};
    
    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("setI2cSpeed Error : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd), cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    if(!bulk_read(rdData, sizeof(rdData))) { //Get completed message.
        printf("Hand shake Error 0x%02X\n",rdData[0]);
        return;
    }	
}

void ausb_set_slave_chipId(uint8_t slave_chipId)
{
    mSlaveChipId = slave_chipId;
}

/**
 * @brief  changeI2CPort
 * @details USB command for I2C port changing.
 */
void ausb_change_i2c_port(int port_num)
{
    uint8_t rdData[1] = {0x00};
    uint8_t cmd[USB_SILAB_PROTOCOL_CMD_LENGTH] = {I2C_CHANGE_PORT_COMMAND,port_num,0,0,0,0,0,0,0,0};
    
    if(!bulk_write(cmd, sizeof(cmd))) {
        printf("changeI2CPort  : %i : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              (int)sizeof(cmd), cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8],cmd[9]);
        return;
    }
    
    if(!bulk_read(rdData, sizeof(rdData))) { //Get completed message.
        printf("Hand shake Error 0x%02X\n",rdData[0]);
        return;
    }
}

void ausb_close(void)
{
	int ret = 0;
	uint8_t buffer[1];

	///////////////////////// \ Special Close Control Command for SiLab C8051F320 USBXpress Driver //////////////////////
	ret = libusb_control_transfer(handle,
							LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
							0x02,
							0x0004,
							0,
							buffer,
							sizeof(buffer),
							0);
	if (ret < 0)
		printf("Control Transfer Failed : %d\n", ret);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(NULL);

	handle = NULL;
}

int ausb_open(void)
{
	int ret = 0;
	ssize_t cnt;
	uint8_t buffer[1];

	if (handle) {
		printf("Already USB openrd\n");
		return 0;
	}

	ret = libusb_init(&ctx);
	if (ret < 0) {
		printf("Error initializing libusb: %s\n", libusb_error_name(ret));
		return ret;
	}

	handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if (handle == NULL) {
		printf("Finding USB device failed.\n");
		return -1;
	}

	/////////////////////////Special Start Control Command for SiLab C8051F320 USBXpress Driver //////////////////////
	libusb_control_transfer(handle,
							LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_IN,
							0xFF,
							0x370B,
							0,
							buffer,
							sizeof(buffer),
							0);

	ret = libusb_control_transfer(handle,
								LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
								0x02,
								0x0002,
								0,
								buffer,
								sizeof(buffer),
								0);
	if (ret < 0)
		printf("Control Transfer Failed : %d\n", ret);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ret = libusb_claim_interface(handle, INTERFACE);
	if (ret != LIBUSB_SUCCESS) {
		printf("Unable to claim USB interface (%d)\n", ret);
		goto out;		
	}

	return 0;

out:
	if (handle) {
		libusb_close(handle);
		handle = NULL;
	}

	libusb_exit(NULL);

	return ret;
}

