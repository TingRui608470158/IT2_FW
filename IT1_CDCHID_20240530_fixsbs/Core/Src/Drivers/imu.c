#include "main.h"
#include "ak09918.h"
#include "icm42688p.h"
#include "usbd_cdc_if.h"

// JOSEPH: add usb handler to send data directly @20220513
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t UserRxBuffer[CDC_RX_DATA_SIZE];
extern uint8_t UserTxBuffer[CDC_TX_DATA_SIZE];
extern uint32_t UserTxBufPtrIn;
extern uint32_t UserTxBufPtrOut;

static uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

uint8_t imu_buffer[24] = {0x55};

bool init_imu(void)
{
	bool status = true;
	if (ak09918_init()) {
		DPRINTF("icm42688_init OK");
	}
	else {
		DPRINTF("icm42688_init Failed");
		status = false;
	}
	if (icm42688_init()) {
		DPRINTF("icm42688_init OK");
	}
	else {
		DPRINTF("icm42688_init Failed");
		status = false;
	}
	if (status==true) icm42688_enable_data_ready_interrupt();
	return status;

}

bool imu_who_am_i(void)
{
	bool status = true;
	if (ak09918_who_am_i()) {
		DPRINTF("ak09918 OK");
	}
	else {
		DPRINTF("ak09918 Failed");
		status = false;
	}
	if (icm42688_who_am_i()) {
		DPRINTF("icm42688 OK");
	}
	else {
		DPRINTF("icm42688 Failed");
		status = false;
	}
	return status;
}

/*
 * Function: process_sensor
 * --------------------
 * reads the data from the sensors and magnetometer and sends
 * it on the USB virtual serial port
 *
 *  returns: boolean value returned
 *
 */
bool process_sensor(void)
{
	uint8_t i, checksum = 0;
//	uint8_t gyro_raw[6], accel_raw[6], mag_raw[6], timestamp_raw[3];
//	uint8_t imu_buffer[24];	// 1+6+6+6+3+1: header + gyro + accel + timestamp + crc

	uint32_t times32;

	times32 = HAL_GetTick();
#if 0
#if 0
	imu_buffer[0] = 0;
	imu_buffer[1] = (uint8_t)(times32>>16) & 0x0f;
	imu_buffer[2] = (uint8_t)(times32>>8) & 0xff;
	imu_buffer[3] = (uint8_t)(times32) & 0xff;

	icm42688_gyro_read_raw(&imu_buffer[4]);
	icm42688_accel_read_raw(&imu_buffer[10]);
	ak09918_mag_read_raw(&imu_buffer[16]);

	for (i=0;i<22;i++) {
		checksum += imu_buffer[i];
	}
	imu_buffer[22] = checksum;
	imu_buffer[23] = 0;
#else
//	imu_buffer[0] = 0x55;
	imu_buffer[1] = (uint8_t)(times32>>24) & 0xff;
	imu_buffer[2] = (uint8_t)(times32>>16) & 0xff;
	imu_buffer[3] = (uint8_t)(times32>>8) & 0xff;
	imu_buffer[4] = (uint8_t)(times32) & 0xff;

	icm42688_gyro_read_raw(&imu_buffer[5]);
	icm42688_accel_read_raw(&imu_buffer[11]);
	ak09918_mag_read_raw(&imu_buffer[17]);

	for (i=0;i<23;i++) {
		checksum += imu_buffer[i];
	}
	imu_buffer[23] = checksum;
#endif
//	print_hex_string(imu_buffer, 24);

//	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)&imu_buffer[0], 24);
//
//	if(USBD_CDC_TransmitPacket(&hUsbDeviceFS) == USBD_OK)
//	{
//		UserTxBufPtrOut = 0;
//	}
//	sendCdcData((uint8_t*)&imu_buffer[0], sizeof(imu_buffer));
	sendCdcData((uint8_t*)&imu_buffer[0], 24);
#else
// JOSEPH: Leading Sync: 0x55 0xaa @20220708
//	UserTxBufferFS
/*
	UserTxBufferFS[0] = 0x55;
	UserTxBufferFS[1] = 0xaa;	//(uint8_t)(times32>>24) & 0xff;
	UserTxBufferFS[2] = (uint8_t)(times32>>16) & 0xff;
	UserTxBufferFS[3] = (uint8_t)(times32>>8) & 0xff;
	UserTxBufferFS[4] = (uint8_t)(times32) & 0xff;

	icm42688_gyro_read_raw(&UserTxBufferFS[5]);
	icm42688_accel_read_raw(&UserTxBufferFS[11]);
	ak09918_mag_read_raw(&UserTxBufferFS[17]);

#if 0 // betta added
	uint8_t tap_num = icm42688_get_tap_detection_status();
	if (tap_num>0)
	{
		uint8_t tmp[30];
		sprintf(tmp,"tap_num=%d\n", tap_num);
		sendCdcData((uint8_t*)tmp, strlen(tmp));
		return true;
	}
#endif // betta added

	for (i=0;i<23;i++) {
		checksum += UserTxBufferFS[i];
	}
	UserTxBufferFS[23] = checksum;
	//sendCdcData((uint8_t*)&UserTxBufferFS[0], 24);
#endif
*/

	uint8_t tmp[80];
	axises data1, data2;

	//icm42688_gyro_read(&data1);
	icm42688_accel_read(&data2);
	//sprintf(tmp,"(g.x, g.y, g.z, a.x, a.y, a.z)=(%.1f, %.1f, %.1f) (%.1f, %.1f, %.1f)\n", data1.x, data1.y, data1.z, data2.x, data2.y, data2.z);
	sprintf(tmp,"(a.x, a.y, a.z)= (%.1f, %.1f, %.1f)\n",  data2.x, data2.y, data2.z);
	sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif
	return true;
}
