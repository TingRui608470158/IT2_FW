/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "usbd_composite.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
bool INT_CDC_RECEIVED = false;
static bool reinit_needed = false;
extern uint8_t mode_2d3d_old;
extern uint8_t mode_2d3d;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
static uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
static uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
extern USBD_CDC_HandleTypeDef *pCDCData;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_CDC_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
//	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pCDCData;
//	if (hcdc->TxState != 0){
//		return USBD_BUSY;
//	}
//	hUsbDeviceFS.pClassData=pCDCData;

  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);

  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:
  	  // connect stage ex
    /*
    if (reinit_needed)
    {
    	MX_USB_DEVICE_Init();
    	reinit_needed= false;
    }
*/
    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:
  	  // connect stage ex
    if (reinit_needed)
    {
        MX_USB_DEVICE_Init();
    	reinit_needed= false;
    }

    break;

    case CDC_SEND_BREAK:

    break;

  default:

    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
#if 1
	uint32_t length = *Len, i;
	hUsbDeviceFS.pClassData=pCDCData;
	if (!INT_CDC_RECEIVED) cdcReceiveBufferLen = 0;
//	DPRINTF("\r\n%sCDC_Receive_FS Len= %d, cdcReceiveBufferLen=%d Buf=%x %x %x %x %s", COLOR_CYAN, length, cdcReceiveBufferLen, UserRxBufferFS[0], UserRxBufferFS[1], UserRxBufferFS[2], UserRxBufferFS[3], COLOR_WHITE);

//	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pCDCData;
//	if (hcdc->TxState != 0){
//		return USBD_BUSY;
//	}
//	hUsbDeviceFS.pClassData=pCDCData;

	INT_CDC_RECEIVED = true;
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  	USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    //USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);

  	memcpy(cdcReceiveBuffer+cdcReceiveBufferLen, UserRxBufferFS, length);
//  for (i=0;i<length;i++) {
//  	cdcReceiveBuffer[cdcReceiveBufferLen+i] = UserRxBufferFS[i];
//  }
  	cdcReceiveBufferLen += length;

//  DPRINTF("\r\n%slen:%d, data= %x %x %x %x %s", COLOR_CYAN, cdcReceiveBufferLen, cdcReceiveBuffer[0], cdcReceiveBuffer[1], cdcReceiveBuffer[2], cdcReceiveBuffer[3], COLOR_WHITE);
//  print_hex_string(cdcReceiveBuffer, cdcReceiveBufferLen);
// 	JOSEPH: add to check 2d/3d mode if data received @20220513
  	/*
  	mode_2d3d = Get_Mode_from_Video_Buffer(Buf);
  	uint8_t send_debug[] = "old:2d, new:2d\n";
  	send_debug[4] = 0x30 + mode_2d3d_old+2;
  	send_debug[12] = 0x30 + mode_2d3d+2;
  	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)&send_debug[0], 15);
  	USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  	*/
#else
  	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	print_hex_string(Buf, sizeof(Buf));
#endif
  reinit_needed = true;
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
//  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
//  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pCDCData;
//  if (hcdc->TxState != 0){
//    return USBD_BUSY;
//  }
//  hUsbDeviceFS.pClassData=pCDCData;
#if 1
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pCDCData;//(USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0){
		return USBD_BUSY;
	}
	hUsbDeviceFS.pClassData=pCDCData;

//  memcpy(UserTxBufferFS, Buf, Len);
//  print_hex_string(UserTxBufferFS, Len);
//  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, Len);
//  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	hcdc->TxBuffer = Buf;
	hcdc->TxLength = Len;
//  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
	if (hcdc->TxState == 0U)
	{
		/* Tx Transfer in progress */
		hcdc->TxState = 1U;

		/* Update the packet total length */
		hUsbDeviceFS.ep_in[CDC_IN_EP & 0xFU].total_length = hcdc->TxLength;

		/* Transmit next packet */
//		(void)USBD_LL_Transmit(&hUsbDeviceFS, CDC_IN_EP, hcdc->TxBuffer, hcdc->TxLength);
		HAL_PCD_EP_Transmit(hUsbDeviceFS.pData, CDC_IN_EP, Buf, Len);
	}
#else
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pCDCData;//(USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0){
		return USBD_BUSY;
	}
	hUsbDeviceFS.pClassData=pCDCData;
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
#endif
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be receivedhUsbDeviceFS
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
