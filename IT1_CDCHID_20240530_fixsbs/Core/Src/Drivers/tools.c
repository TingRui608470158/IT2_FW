#include "main.h"

#include <stdio.h>

//#include "usb_device.h"
//#include "usbd_core.h"
#include "usbd_composite.h"
#include "../../../USB_DEVICE_COMPOSITE/App/usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint32_t UserTxBufPtrOut;

uint8_t DPRINTU(uint8_t *pbuff, uint32_t length)
{
#if DEBUG_VIA_USB_PRINT
//	return USBD_CDC_SetTxBuffer(&hUsbDeviceFS, pbuff, length);
#else
	return 0;
#endif
}

uint8_t DPRINTU_END()
{
#if DEBUG_VIA_USB_PRINT
//	return USBD_CDC_TransmitPacket(&hUsbDeviceFS);
#else
	return 0;
#endif
}

void print_string(uint8_t* str, int len)
{
  DPRINTU((uint8_t *)str, strlen((char *)str));
  DPRINTU_END();

	DPRINTF((char *)str);
}
/*
 * print_hex_string: print hex string to usb-cdc and uart
 *
 */
void print_hex_string(uint8_t* str, int len)
{
	char hexstr[80]={0};
	int i;

  /* Calculate String Length */
  for (i = 0; i<len;i++) {
  	sprintf(&hexstr[i*3], "%02x ", str[i]);
  }
  sprintf(&hexstr[i*3],"%s\r\n","");
  DPRINTU((uint8_t *)hexstr, strlen(hexstr));
  DPRINTU_END();

	DPRINTF(hexstr);
}

void waitForCdcData(uint32_t len)
{
	cdcReceiveBufferLen = 0;
	do {
//		sendCdcData("ABC", 3);
		HAL_Delay(100);
//		if (cdcReceiveBufferLen>=4) INT_CDC_RECEIVED = false;
	} while (cdcReceiveBufferLen<len);
	INT_CDC_RECEIVED = false;
}

void sendCdcData(uint8_t *pbuff, uint32_t length)
{
//	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
//	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, pbuff, length);
//
//	if(USBD_CDC_TransmitPacket(&hUsbDeviceFS) == USBD_OK)
//	{
//		UserTxBufPtrOut = 0;
//	}
	CDC_Transmit_FS(pbuff, length);
}
