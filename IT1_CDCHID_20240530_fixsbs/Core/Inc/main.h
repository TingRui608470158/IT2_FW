/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
//#include "usbd_cdc.h"
// JOSEPH: add for VideoOutputEnable and VideoOutputDisable link
#include "switch_view.h"

//#include "usbd_cdc_if.h"
//#include "usbd_core.h"
//#include "usbd_def.h"
//#include "usbd_cdc.h"
//#include "usb_device.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
static volatile uint8_t NewALSFlag, NewPSFlag, NewIMUFlag, interruptALS, interruptPS;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void print_hex_string(uint8_t* str, int len);
extern void print_string(uint8_t* str, int len);
extern void waitForCdcData(uint32_t len);
extern void sendCdcData(uint8_t *pbuff, uint32_t length);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RDC200A_RSTB_H_Pin GPIO_PIN_13
#define RDC200A_RSTB_H_GPIO_Port GPIOC
#define IMU_INT2_H_Pin GPIO_PIN_14
#define IMU_INT2_H_GPIO_Port GPIOC
#define SOS_KEY_Pin GPIO_PIN_15
#define SOS_KEY_GPIO_Port GPIOC
#define PROX_INT_H_Pin GPIO_PIN_0
#define PROX_INT_H_GPIO_Port GPIOA
#define PROX_INT_H_EXTI_IRQn EXTI0_IRQn
#define ALS_INTN_H_Pin GPIO_PIN_2
#define ALS_INTN_H_GPIO_Port GPIOA
#define LED_D_FAULT_L_Pin GPIO_PIN_3
#define LED_D_FAULT_L_GPIO_Port GPIOA
#define IMU_INT1_H_Pin GPIO_PIN_5
#define IMU_INT1_H_GPIO_Port GPIOA
#define LED_D_FAULT_R_Pin GPIO_PIN_6
#define LED_D_FAULT_R_GPIO_Port GPIOA
#define LED_D_EN_L_Pin GPIO_PIN_0
#define LED_D_EN_L_GPIO_Port GPIOB
#define LED_D_EN_R_Pin GPIO_PIN_1
#define LED_D_EN_R_GPIO_Port GPIOB
#define DEV76XX_RSTN_Pin GPIO_PIN_2
#define DEV76XX_RSTN_GPIO_Port GPIOB
#define BRIGHTNESS_MINUS_Pin GPIO_PIN_12
#define BRIGHTNESS_MINUS_GPIO_Port GPIOB
#define BRIGHTNESS_PLUS_Pin GPIO_PIN_13
#define BRIGHTNESS_PLUS_GPIO_Port GPIOB
#define LT7911D_RESET_Pin GPIO_PIN_14
#define LT7911D_RESET_GPIO_Port GPIOB
#define VIBRATER_EN_Pin GPIO_PIN_15
#define VIBRATER_EN_GPIO_Port GPIOB
#define FAN_EN_Pin GPIO_PIN_8
#define FAN_EN_GPIO_Port GPIOA
#define IR_LED_EN_Pin GPIO_PIN_15
#define IR_LED_EN_GPIO_Port GPIOA
#define RDC200A_BOOTING_INDICATION_Pin GPIO_PIN_5
#define RDC200A_BOOTING_INDICATION_GPIO_Port GPIOB
#define SECURITY_KEY_Pin GPIO_PIN_8
#define SECURITY_KEY_GPIO_Port GPIOB
#define VSYNC_H_Pin GPIO_PIN_9
#define VSYNC_H_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define DEBUG_VIA_USB_PRINT		0
#define DEBUG_VIA_UART_PRINT	1
#define TEST_MDM_UNLOCK			0 //<-- MDM
#define TEST_GUESTURE			0
#define TEST_LM3435				1
#define TEST_RDC200A			1
#define TEST_SWITCH_2D3D_View	1 //<-- Side by Side
#define TEST_RDC200A_PATTERN	0
#define TEST_DO_OTHERS			1
#define TEST_PS					1
#define TEST_ALS				1
#define TEST_IMU				1
#define TEST_AES				0
#define TEST_LT7911D			0
#define COLOR_BLACK 	"\033[1;30m"
#define COLOR_RED 		"\033[1;31m"
#define COLOR_GREEN 	"\033[1;32m"
#define COLOR_YELLOW 	"\033[1;33m"
#define COLOR_BLUE 		"\033[1;34m"
#define COLOR_PURPLE 	"\033[1;35m"
#define COLOR_CYAN 		"\033[1;36m"
#define COLOR_WHITE 	"\033[1;37m"


#define I2C_READ	1
#define I2C_WRITE	0

#define USBD_CDC1_HID1	1

#ifndef TRUE
#define TRUE		true
#endif
#ifndef FALSE
#define FALSE		false
#endif

//#define IMSG(s, ...) do { printf(s, ##__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
//#define DMSG(s, ...) do { printf(s, ##__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
//#define EMSG(s, ...) do { printf(s, ##__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
#if 0
#define IMSG(...)    printf("INFO : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");

#define DMSG(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");

#define EMSG(...)    printf("ERROR : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#endif

////

//#if DEBUG_VIA_USB_PRINT
//	//USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t *)str, strlen(str));
//	#define DPRINTU	USBD_CDC_SetTxBuffer
//	#define DPRINTU_END	USBD_CDC_TransmitPacket
//
//#else
//	#define DPRINTU  do {} while (0)
//	#define DPRINTU_END	do {} while (0)
//#endif

extern uint8_t DPRINTU(uint8_t *pbuff, uint32_t length);
extern uint8_t DPRINTU_END();

#if DEBUG_VIA_UART_PRINT
#define DPRINTF(...)    printf(__VA_ARGS__);\
                            printf("\r\n");

#else
#define DPRINTF(fmt, ...) do {} while (0)
#endif

#define CDC_RX_DATA_SIZE  2065
#define CDC_TX_DATA_SIZE  2065


extern I2C_HandleTypeDef hi2c1;
#define CM32183E_I2C1	hi2c1
#define AK09918C_I2C1 hi2c1
#define VCNL36828P_I2C1	hi2c1
#define LM3435_I2C1	hi2c1
#define RDC200A_I2C1	hi2c1
#define FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE    0x0801FFD0

/* Store proximity threshold, default brightness */
#define USER_DATA_SIZE 2
enum {
	proximity,
	lux_index
};
/*
#define LED_BRIGHTNESS_OFFSET	FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE+8
#define PROXIMITY_THRESHOLD_OFFSET	FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE
*/
extern int rdc200a_test_main(int test_num, const char * test_param[]);

extern void delay_us(uint32_t udelay);
extern bool process_command(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
