/*
 * switch_view.c
 *
 *  Created on: 2021年12月30日
 *      Author: LEAPSY
 */

/* Includes */
#include <string.h>
#include <stdlib.h>
#include "main.h"

// JOSEPH: add usb handler to send data directly @20220513
#include "usbd_cdc_if.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
extern UART_HandleTypeDef huart1;
// JOSEPH: add 2D/3D mode record @20220513
uint8_t mode_2d3d_old = 0;
uint8_t mode_2d3d = 0;	// default: 2d
// JOSEPH: fix lonk bug @20220519
uint8_t CDCRxBuffer[64];
uint32_t cdc_rx_total;

/*
 * The boolean variable two_dim_video_mode_flag is true
 * when switching to 2D mode, otherwise 3D.
 */
bool two_dim_video_mode_flag;
/*
 * The boolean variable three_dim_video_mode_flag is true
 * when switching to 3D mode, otherwise 2D.
 */
bool three_dim_video_mode_flag = false;

/*
 * Function: Get2DVideoModeFlag
 * --------------------
 * gets the flag, i.e. two_dim_video_mode_flag
 *
 *  returns: boolean value returned
 *
 */
bool Get2DVideoModeFlag(void)
{
  return two_dim_video_mode_flag;
}

/*
 * Function: Set2DVideoModeFlag
 * --------------------
 * sets the flag, i.e. two_dim_video_mode_flag
 *
 *  Setting: boolean value
 *
 */
void Set2DVideoModeFlag(bool Setting)
{
  two_dim_video_mode_flag = Setting;
}

/*
 * Function: Get3DVideoModeFlag
 * --------------------
 * gets the flag, i.e. three_dim_video_mode_flag
 *
 *  returns: boolean value returned
 *
 */
bool Get3DVideoModeFlag(void)
{
  return three_dim_video_mode_flag;
}

/*
 * Function: Set3DVideoModeFlag
 * --------------------
 * sets the flag, i.e. three_dim_video_mode_flag
 *
 *  Setting: boolean value
 *
 */
void Set3DVideoModeFlag(bool Setting)
{
  three_dim_video_mode_flag = Setting;
}

/*
 * Function: Get_Mode_from_Video_Buffer
 * --------------------
 * gets 2D/3D mode by parsing UART message, i.e.
 * 	set2d3d 0 (2D display mode) or
 * 	set2d3d 1 (3D display mode)
 *
 *  msg: message received from a mobile device across a serial line
 *
 */
int32_t Get_Mode_from_Video_Buffer(uint8_t *msg)
{
	uint8_t i=0, ret = 0;
  char *fnstr = "set2d3d";
  char *pos, *ptr;
  uint32_t mode = -1;
  // JOSEPH: do more simple compare @20220513
  for (i=0;i<7;i++)
	  if (*(msg+i)!=*(fnstr+i)) ret = -1;
  if (ret >= 0) mode = *(msg+8) - 0x30;

  return mode;
}

/*
 * Function: Set2DVideoMode
 * --------------------
 * Switches to 2D view
 *
 */
void Set2DVideoMode(void)
{
#if defined(DEBUG_VIDEO_MODE)
    uint8_t Test[] = "Switch to 2D Video Mode !!!\r\n"; //Data to send

    // Send out buffer (error message)
    HAL_UART_Transmit(&huart1,Test,sizeof(Test),HAL_MAX_DELAY);// Sending in normal mode

    // Wait
    HAL_Delay(500);
#endif /* defined(DEBUG_VIDEO_MODE) */
    // Stop displaying data on left and right panel
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)"Set2DVideoMode\n", 15);
	USBD_CDC_TransmitPacket(&hUsbDeviceFS);

//    VideoOutputDisable();
//    HAL_GPIO_WritePin(LT_GPIO5_H_GPIO_Port, LT_GPIO5_H_Pin, GPIO_PIN_RESET);
    Set2DVideoModeFlag(true);
// JOSEPH: need to check VSYNC @20220513
    HAL_Delay(500);
//	VideoOutputEnable();
}
/*
 * Function: Set3DVideoMode
 * --------------------
 * Switches to 3D view
 *
 */
void Set3DVideoMode(void)
{
#if defined(DEBUG_VIDEO_MODE)
    uint8_t Test[] = "Switch to 3D Video Mode !!!\r\n"; //Data to send

    // Send out buffer (error message)
    HAL_UART_Transmit(&huart1,Test,sizeof(Test),HAL_MAX_DELAY);// Sending in normal mode

    // Wait
    HAL_Delay(500);
#endif /* defined(DEBUG_VIDEO_MODE) */
    // Stop displaying data on left and right panel

    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)"Set3DVideoMode\n", 15);
	USBD_CDC_TransmitPacket(&hUsbDeviceFS);

//    VideoOutputDisable();
//    HAL_GPIO_WritePin(LT_GPIO5_H_GPIO_Port, LT_GPIO5_H_Pin, GPIO_PIN_SET);
    Set3DVideoModeFlag(true);
// JOSEPH: need to check VSYNC @20220513
    HAL_Delay(500);
//    VideoOutputEnable();
}

/*ㄍㄧ
 * Function: Set3DVideoMode
 * --------------------
 * Switches between 2D and 3D view by clicking on the 2D/3D toggle button
 * located in the application.
 *
 */
void switch_2d3d_view(void)
{

  // Parse message received from a mobile device across a serial line
//  uint32_t mode = Get_Mode_from_Video_Buffer(CDCRxBuffer);

//  JOSEPH: add mode record check @20220513
  if (mode_2d3d_old != mode_2d3d) {
	  mode_2d3d_old = mode_2d3d;
	  printf("switch_2d3d_view : %d\r\n", mode_2d3d);
	  // Switching between 2D and 3D
	  switch (mode_2d3d) {
		case 0:
		  Set2DVideoMode();
		  break;
		case 1:
		  Set3DVideoMode();
		  break;
		default:
		  break;
	  }
  }

  // Clear the buffer
  memset (CDCRxBuffer, '\0', 64);  // clear the buffer
}
