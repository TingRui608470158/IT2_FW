/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_composite.h"
#include "usbd_cdc.h"
#include <stdio.h>
//http://aws.micromousetaiwan.org/?p=1968
#include "dev76xx.h"
#include "usbd_hid.h"
#include "usb_hid_keys.h"
#include "user_hid_keyboard.h"
//#include "ak09918.h"
//#include "icm42688p.h"
#include "imu.h"
#include "aes.h"
#include "cm32183e.h"
#include "vcnl36828p.h"
#include "brightness.h"
#include "rdc200a.h"
#include "rti_vc_common.h"
#include "test.h"
/* USER CODE END Includes */

//#define PS_INTERRUPT_MODE
uint8_t tap_num = 0;

uint32_t display_timeout;
bool display_on = TRUE;
bool brightness_changed;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_ADDRESS_FOR_DEVICE_DATA    0x0801FFEA
#define POLL_DURATION 100 // unit: mili seconds
#define DISPLAY_MINUTES 1 // unit: minutes
#define DISPLAY_TIMEOUT_COUNT (1000*60*DISPLAY_MINUTES)/POLL_DURATION
//const uint8_t DEVICE_DATA[8] __attribute__((at(FLASH_ADDRESS_FOR_DEVICE_DATA)));
uint8_t DEVICE_DATA[16]  __attribute__((section(".app_info"))) =	// 0x0801FFCE
{
    0x18, 0x7A, 0x93, 0x00, 0x0F, 0x12, 0xAA, 0xBB,	// 0x801ffd0 ~ 0x801ffd7
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08	// 0x801ffd8 ~ 0x801ffdf
};
//uint8_t STATUS_DATA[2] = {0,1};
uint8_t STATUS_DATA[4] = {0xAB, 0x0, 0xCD, 0x1};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t aes_key[] = { 0x4C, 0x65, 0x61, 0x70, 0x73, 0x79, 0x49, 0x54, 0x31, 0x47, 0x6C, 0x61, 0x73, 0x73, 0x65, 0x73 };	// LeapsyIT1Glasses, fix
uint8_t aes_in[]  = { 0x49, 0x44, 0x39, 0x30, 0x34, 0x34, 0x30, 0x32, 0x37, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // ID90440275{000000}, floating: last 6 bytes: random-number(2) + device-id(4)
uint8_t aes_out[16] = {0};
//																																					 |--- num ---|###### device id #####|
//																																					 |-- Phone --|###### Glasses #######|
// JOSEPH: Phone-Glassess AES (ECB) handshake flow:
/*
 * 1. Glasses waiting for usb cdc data when cable connected until Phone send random number
 * 2. Glasses response device-id (4 bytes)
 * 3. Phone send AES_ECB_encrypt result (last 4 bytes) via key and in
 * 4. Glasses response result (success if same AES_ECB_encrypt result)
 */
//uint8_t UserRxBuffer[CDC_User_Data_SIZE];/* Received Data over USB are stored in this buffer */
//uint8_t UserTxBuffer[CDC_TX_DATA_SIZE];/* Transmitted Data over UART (CDC interface) are stored in this buffer */
//uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to
//                               start address when data are received over USART */
//uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to
//                                 start address when data are sent over USB */
bool imuFound, magFound;
uint16_t als_code = 300;
//uint16_t lux_value[]  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000};	// lux
// CH: Adapt to low lux level measured
#if 1 // betta changed

int16_t standard_range = 1200;
int8_t out_brightness = -1;
uint16_t lux = 0;

#else
uint16_t lux_value[]  = {0, 48, 64, 80, 122, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 12000, 16000, 20000};	// lux
#endif
uint16_t brightness_old = 0;
// Store the current led brightness/
int8_t current_brightness = 4;
bool ps_close_state = FALSE;
bool button_click_state = FALSE;
//bool manual_control = FALSE;

#if 1 // betta added
enum {
 PROXIMITY_DEV,
 LIGHT_DEV,
};
#define PROXIMITY_STATUS_CHECKSUM (0x33+0xcc+0xFF)
#define LIGHT_LUX_CHECKSUM (0x22+0xcc+0xFF)

#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */
void do_others(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DEBUG_SWO	0

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
#if DEBUG_SWO
	ITM_SendChar(ch);
#else
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
#endif
	return ch;
}
#if 1 // betta added

void increase_brightness(bool plus)
{
	uint32_t User_Data[USER_DATA_SIZE];
	if(!plus) {
		current_brightness -= 1;
		if(current_brightness < 0)
		{
			current_brightness = 0;
		}
	}
	else {
		current_brightness += 1;
		if(current_brightness > 4)
		{
			current_brightness = 4;
		}
	}
	set_led_brightness(current_brightness);
//	int16_t current_range = current_brightness * 0.2 + 0.1;
//	standard_range = inverse_gamma(current_range, lux);

	Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
	User_Data[brightness]= current_brightness;
	Flash_Write_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, (uint64_t *)User_Data, USER_DATA_SIZE, FLASH_TYPEPROGRAM_DOUBLEWORD);
}


bool process_command() // betta test
{
	if (cdcReceiveBufferLen==0)
		return false;

	do {
		if (strncmp("bra", cdcReceiveBuffer, 3) == 0)
		{
			increase_brightness(true);
			break;
		}
		if (strncmp("brd", cdcReceiveBuffer, 3) == 0)
		{
			increase_brightness(false);
			break;
		}
		/* 2D display */
		if (strncmp("2dd", cdcReceiveBuffer, 3) == 0)
		{
			rdc200a_sbs_off();
			break;
		}
		/* 3D display */
		if (strncmp("3dd", cdcReceiveBuffer, 3) == 0)
		{
			rdc200a_sbs_on();
			break;
		}

		if (strncmp("p\r\n", cdcReceiveBuffer, cdcReceiveBufferLen) == 0)/* Set proximity threadshold */
		{
			uint16_t ps= VCNL36828P_GET_PS_DATA();
			uint8_t tmp[100]={0};
			uint32_t User_Data[USER_DATA_SIZE];
			Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
			User_Data[proximity]=ps;
			Flash_Write_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, (uint64_t *)User_Data, USER_DATA_SIZE, FLASH_TYPEPROGRAM_DOUBLEWORD);
			VCNL36828P_SET_PS_HighThreshold(ps);
			VCNL36828P_SET_PS_LowThreshold(ps-20);
			sprintf(tmp, "Set PS_HighThreshold= %d\n", ps);
			sendCdcData((uint8_t*)tmp, strlen(tmp));
			HAL_Delay(2000);
			break;
		}
		if (strncmp("dp\r\n", cdcReceiveBuffer, cdcReceiveBufferLen) == 0) {
			uint8_t tmp[100]={0};
			uint32_t User_Data[USER_DATA_SIZE];

			Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
			//Flash_Read_Data(LED_BRIGHTNESS_OFFSET, &User_Data, 1);
			sprintf(tmp, "(PS_HighThreshold, Brightness)=(%d,%d)\n",User_Data[proximity], User_Data[brightness]);
			sendCdcData((uint8_t*)tmp, strlen(tmp));
			HAL_Delay(2000);
			break;
		}

		if (strncmp("help\r\n", cdcReceiveBuffer, cdcReceiveBufferLen) == 0)
		{
			uint8_t tmp[512]= "*********************************************************\r\n \
			You can input the following commands to do something.r\n \
			(1) bra => Switch brightness up.\n \
			(2) brd => Switch brightness down.\n \
			(3) dp => Dump user data in flash.\n \
			(4) 2dd => 2D display.\n \
			(5) 3dd => 3D display.\n \
			(6) p => Calibrate proximity threadshold .\n";

			sendCdcData((uint8_t*)tmp, strlen(tmp));
			HAL_Delay(5000);
			break;
		}
//		sendCdcData('K', 1);
		//puts(cdcReceiveBuffer);
	} while (0);
	memset(cdcReceiveBuffer, 0, 64);
	cdcReceiveBufferLen=0;
	INT_CDC_RECEIVED = false;

	return true;
}

#endif // betta added
/**
 * @brief do others control (button, als, p-sensor)
 * @retval None
 */

void do_others(void)
{
	int i = 0;

#if TEST_PS
	//	if (NewPSFlag)
	{
		uint16_t ps_interrupt = get_ps_if();
#if 1 // betta added
		uint8_t tmp[40]={0};
/*
		uint16_t ps= VCNL36828P_GET_PS_DATA();
		sprintf(tmp,"ps=%d\n", ps);
		DPRINTF("%s\r\n", tmp);
		sendCdcData((uint8_t*)tmp, strlen(tmp));
*/
		//sprintf(tmp,"fr=(x, y, z, fr)=(0x%04x, 0x%04x, 0x%04x, %d)\n", RDC_REG_GET(0x49), RDC_REG_GET(0x82b), RDC_REG_GET(0x92b), rdc200a_get_frame_rate_from_reg());
		//sprintf(tmp,"fr=%d\n", rdc200a_get_frame_rate_from_reg());
		//sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif
		if (ps_interrupt&VCNL36828P_PS_IF_AWAY) {
			//DPRINTF("\r\n<<<<<<<<<< %x: %s%s%s\r\n", ps_interrupt, COLOR_RED, "AWAY", COLOR_WHITE);
			memset(tmp,0,30);
			sprintf(tmp,"ps=%d, AWAY\n", VCNL36828P_GET_PS_DATA());
			sendCdcData((uint8_t*)tmp, strlen(tmp));

			ps_close_state = FALSE;
#if 0 // betta added
			memset(tmp,0,30);
			// header:	0x33ccff
			tmp[0] = 0x33;
			tmp[1] = 0xcc;
			tmp[2] = 0xff;

			// sensor index
			tmp[3] = PROXIMITY_DEV;

			// value
			tmp[4] = 1;

			// checksum = PROXIMITY_STATUS_CHECKSUM + tmp[4] = 511
			sprintf(&tmp[5],"%d", PROXIMITY_STATUS_CHECKSUM + tmp[4]);
			sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif

			// TODO display off (LM3435 OFF, RDC200A OFF)
#if TEST_LM3435
			/* Configure GPIO pin Output Level */
			/*// betta masked
			HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_RESET); // PB0
			HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_RESET); // PB1
			HAL_Delay(5);
			*/
#endif /* TEST_LM3435 */
#if TEST_RDC200A
			//display_suspend(FALSE);
#endif /* TEST_RDC200A */
		}
		else if (ps_interrupt&VCNL36828P_PS_IF_CLOSE) {
			//DPRINTF("\r\n<<<<<<<<<< %x: %s%s%s\r\n", ps_interrupt, COLOR_GREEN, "CLOSE", COLOR_WHITE);
			memset(tmp,0,20);
			sprintf(tmp,"ps=%d, CLOSE\n", VCNL36828P_GET_PS_DATA());
			sendCdcData((uint8_t*)tmp, strlen(tmp));
			ps_close_state = TRUE;
			display_timeout=DISPLAY_TIMEOUT_COUNT;

#if 0 // betta added
			memset(tmp,0,30);
			// header:	0x33ccff
			tmp[0] = 0x33;
			tmp[1] = 0xcc;
			tmp[2] = 0xff;

			// sensor index
			tmp[3] = PROXIMITY_DEV;

			// value
			tmp[4] = 2;

			// checksum = PROXIMITY_STATUS_CHECKSUM + tmp[4] = 512
			sprintf(&tmp[5],"%d", PROXIMITY_STATUS_CHECKSUM + tmp[4]);
			sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif
			// TODO adjust brightness (LM3435: adjust level, RDC200A ON)
#if TEST_LM3435
			/* Configure GPIO pin Output Level */
			HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_SET); // PB0
			HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_SET); // PB1
			HAL_Delay(5);
#endif /* TEST_LM3435 */
#if TEST_RDC200A
			//display_resume(FALSE, RDC_VIDEO_COORD_INPUT_LVDS);
#endif /* TEST_RDC200A */
		}

		//ps_close_state = TRUE; // betta temporarily added

#if 1 // betta masked
		if (ps_close_state) {
#if TEST_LM3435
			 // masked to read  brightness in ISR
#if 1
			if (button_click_state) { // manual mode
				set_led_brightness(current_brightness);
				button_click_state=FALSE;
			}
#endif

#if 1 // betta changed
			if (1) {// betta changed
#else // original
			else if (!manual_control) { // auto mode
#endif
#if TEST_ALS
				lux = (uint16_t)read_CM32183E();


#if 0 // betta added to debug

				sprintf(tmp,"lux=%d\r\n", lux);
				sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif
#if 0 // betta added
					memset(tmp,0,30);
					// header:	0x33ccff
					tmp[0] = 0x33;
					tmp[1] = 0xcc;
					tmp[2] = 0xff;

					// sensor index
					tmp[3] = LIGHT_DEV;

					// value
					sprintf(&tmp[4],"%d", lux);

					// checksum = LIGHT_LUX_CHECKSUM + lux
					sprintf(&tmp[6],"%d", LIGHT_LUX_CHECKSUM + tmp[4]);
					sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif

#if 1
				if(out_brightness != -1)
				{
					float current_range = out_brightness * 0.2 + 0.1;
					standard_range = inverse_gamma(current_range, lux);

#if 1
					memset(tmp,0,50);
					sprintf(tmp,"out_brightness =%d\r\n", out_brightness);
					sendCdcData((uint8_t*)tmp, strlen(tmp));
					delay_us(100);
					memset(tmp,0,50);
					sprintf(tmp,"current_range =%f\r\n", current_range);
					sendCdcData((uint8_t*)tmp, strlen(tmp));
					delay_us(100);

					memset(tmp,0,50);
					sprintf(tmp,"lux = %d \r\n", lux);
					sendCdcData((uint8_t*)tmp, strlen(tmp));
					delay_us(100);
					memset(tmp,0,50);
					sprintf(tmp,"standard_range = %d \r\n", standard_range);
					sendCdcData((uint8_t*)tmp, strlen(tmp));
					delay_us(100);
#endif
					out_brightness = -1;
				}


				for (i=1;i<6;i++)
				{
					uint8_t current_gamma_out = positive_gamma(i, standard_range);

					if (lux< current_gamma_out) {
//						memset(tmp,0,50);
//						sprintf(tmp,"lux_value[%d] = %d\r\n", i, current_gamma_out);
//						sendCdcData((uint8_t*)tmp, strlen(tmp));

						current_brightness = i-1;
						break;
					}
				}

				if(lux > standard_range)
				{
					current_brightness = 5;
				}
				// CH: remove the overshoot when lux level is greater than maximum
//				if (lux_level>5) lux_level = 5;
#else
				if (lux>9000) current_brightness = 9;
				else current_brightness = lux/1000;
#endif
				//DPRINTF("\r\nlux = %d, level=%s<%d>%s", lux, COLOR_CYAN, lux_level, COLOR_WHITE);

				if (brightness_old!=current_brightness)
				{
					brightness_old = current_brightness;
					set_led_brightness(current_brightness);
#if 0 // betta added
					control_backlight(current_brightness);
					current_brightness = convert_lux_to_brightness(current_brightness);
#endif
#if 0 // betta added to debug
				memset(tmp,0,50);
				sprintf(tmp,"current_brightness =%d\n", current_brightness);
				sendCdcData((uint8_t*)tmp, strlen(tmp));
#endif

					//DPRINTF("\r\nlux = %d, level=%s<%d>%s, current_brightness=%d\n", lux, COLOR_CYAN, lux_level, COLOR_WHITE, current_brightness);
				}
				//control_backlight(lux_level); // betta masked
#endif /* TEST_ALS */
			}
#endif /* TEST_LM3435 */
		}
#endif // betta masked
	}
#endif /* TEST_PS */
#if TEST_SOS
	sos_button_click_state = read_sos_button_states();
#endif /* TEST_SOS */
}

//#define TEST_IMU_XYZ_AXIS
#define	AWAKE_DISPLAY
#ifdef TEST_IMU_XYZ_AXIS
uint8_t axis[3] = {'x', 'y' , 'z'};
enum{
	x,
	y,
	z
};
#endif
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint8_t tmp[100]={0};
	static uint32_t tap_times[3]= {0, 0, 0}; // For x, y, z
	static uint32_t tap_count=0;

	switch (GPIO_Pin) {
		case PROX_INT_H_Pin:
			//ps_int=true;
			//sprintf(tmp,"PROX_INT_H_Pin\n");
			//sendCdcData((uint8_t*)tmp, strlen(tmp));
			HAL_NVIC_ClearPendingIRQ(PROX_INT_H_EXTI_IRQn);
			break;
		case (BRIGHTNESS_MINUS_Pin|BRIGHTNESS_PLUS_Pin):
			if (ps_close_state) {
				out_brightness = read_button_states(current_brightness);

			}
			HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
			break;
		case IMU_INT1_H_Pin:
		do {
			uint8_t val;
/*
			val = icm42688_get_tilt_detection_status();
			if (val>0)
			{
				sprintf(tmp,"val=0x%04X\n", val);
				sendCdcData((uint8_t*)tmp, strlen(tmp));
				break;
			}
*/
#if 1 // MASKED
			val = icm42688_get_tap_detection_status();
			tap_num= (val>>3)&3;

			/* : Represents the accelerometer axis on which tap
energy is concentrated
			 *  */
#ifdef TEST_IMU_XYZ_AXIS
			uint8_t tap_axis = (val>>1)&3;
#endif
			/* Polarity of tap pulse */
			//uint8_t tap_dir = val&1;
			if (tap_num == 2)
			{

#ifdef  AWAKE_DISPLAY
				HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_SET); // PB0
				HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_SET); // PB1
				display_timeout=DISPLAY_TIMEOUT_COUNT;
				ps_close_state = TRUE;
#else //  !AWAKE_DISPLAY

#ifdef TEST_IMU_XYZ_AXIS
				if (tap_count<300)
				{
					sprintf(tmp,"tap_count=%d, (%c-axis -> %d times)\n",++tap_count, axis[tap_axis], ++tap_times[tap_axis]);
		    		sendCdcData((uint8_t*)tmp, strlen(tmp));
				}
				else if (tap_count==300)
				{
					sprintf(tmp,"tap_count=%d, (x,y,z) -> (%d,%d,%d) times)\n",tap_count,  tap_times[x], tap_times[y], tap_times[z]);
		    		sendCdcData((uint8_t*)tmp, strlen(tmp));
				}

				display_on= (tap_axis==z)? true : false;
#endif
				// sprintf(tmp,"(tap_num, tap_axis, tap_dir)=(%d, %c-axis, %d)\n", tap_num, axis[tap_axis], tap_dir);
#ifndef TEST_IMU_XYZ_AXIS
	    		if (ps_close_state)
#endif
	    		{
	    			if (display_on)
	    			{
	    				HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_RESET); // PB0
	    				HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_RESET); // PB1
#ifndef TEST_IMU_XYZ_AXIS
	    				display_on = false;
#endif
	    			}
	    			else
	    			{
	    				HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_SET); // PB0
	    				HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_SET); // PB1
#ifndef TEST_IMU_XYZ_AXIS
	    				display_on = true;
#endif
	    			}
	    		}
#endif // end of AWAKE_DISPLAY
			}
#endif
		} while(0);
			HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
			break;
		default:
			__NOP();
			break;
    }

}

void delay_us(uint32_t udelay)
{
  uint32_t startval,tickn,delays,wait;

  startval = SysTick->VAL;
  tickn = HAL_GetTick();
  //sysc = 72000;  //SystemCoreClock / (1000U / uwTickFreq);
  delays =udelay * 72; //sysc / 1000 * udelay;
  if(delays > startval)
	{
		while(HAL_GetTick() == tickn)
		{

		}
		wait = 72000 + startval - delays;
		while(wait < SysTick->VAL)
		{

		}
	}
  else
	{
		wait = startval - delays;
		while(wait < SysTick->VAL && HAL_GetTick() == tickn)
		{

		}
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int loop_counter = 0;
	int i;
	uint32_t User_Data[USER_DATA_SIZE];

	bool status = false;
//	uint8_t Buf[64];
//	char str[80];

	int ret;
    E_VC_PANEL_DEVICE_T panel_type;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  //MX_USB_DEVICE_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */
#if TEST_LT7911D
	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LT7911D_RESET_GPIO_Port, LT7911D_RESET_Pin, GPIO_PIN_RESET); // PB14
#endif

#if TEST_MDM_UNLOCK
	MX_USB_DEVICE_Init(); // betta: Due to Unexpected issue, USB won't work if enable it early.
	DPRINTF("iT1 Start: DEVICE_DATA: %x %x %x %x", DEVICE_DATA[0], DEVICE_DATA[1], DEVICE_DATA[2], DEVICE_DATA[3]);
RETRY_KEY:
	//sendCdcData("ABC", 3);
	waitForCdcData(2);

	DPRINTF("cdc receive: %x %x", cdcReceiveBuffer[0], cdcReceiveBuffer[1]);
	aes_in[10] = cdcReceiveBuffer[0];
	aes_in[11] = cdcReceiveBuffer[1];
	memcpy(&aes_in[12], &DEVICE_DATA[0], 4);
	memcpy(aes_out, aes_in, 16);
	// http://aes.online-domain-tools.com/
	print_hex_string(aes_in, 16);
	struct AES_ctx ctx;

	AES_init_ctx(&ctx, aes_key);
	AES_ECB_encrypt(&ctx, aes_out);
	print_hex_string(aes_out, 16);

	sendCdcData(&DEVICE_DATA[0], 4);
	waitForCdcData(4);

	DPRINTF("cdc receive: %x %x %x %x", cdcReceiveBuffer[0], cdcReceiveBuffer[1], cdcReceiveBuffer[2], cdcReceiveBuffer[3]);
	status = true;
	for (i=0;i<4;i++) {
		if (aes_out[12+i] != cdcReceiveBuffer[i]) {
			status = false;
			break;
		}
	}
	if (status) {
		DPRINTF("\r\n%sMDM SUCCESS!%s", COLOR_CYAN, COLOR_WHITE);
		sendCdcData(&STATUS_DATA[2], 2);
		HAL_Delay(100);
	}
	else {
		DPRINTF("\r\n%sMDM FAIL! Retry!%s", COLOR_RED, COLOR_WHITE);
		sendCdcData(&STATUS_DATA[0], 2);

		/* Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(SECURITY_KEY_GPIO_Port, SECURITY_KEY_Pin, GPIO_PIN_RESET); // PB8
		HAL_GPIO_WritePin(SECURITY_KEY_GPIO_Port, SECURITY_KEY_Pin, GPIO_PIN_RESET); // PB8
		HAL_Delay(5);
		goto RETRY_KEY;
	}
#endif
	MX_USB_DEVICE_Init(); // betta: Due to Unexpected issue, USB won't work if enable it early.
#if TEST_LT7911D
	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LT7911D_RESET_GPIO_Port, LT7911D_RESET_Pin, GPIO_PIN_SET); // PB14
#endif

#if TEST_LM3435
	/* Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_SET); // PB0
	HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_SET); // PB1
	HAL_Delay(5);

	// TODO adjust brightness (LM3435: adjust level, RDC200A ON)
#endif
#if TEST_RDC200A
//	DPRINTF("TEST_RDC200A");
#if 1
//	rdc200a_setupOutputpin();
#endif
	HAL_Delay(500);  //1sec delay Must need.

//	rdc200a_power_on();
//	const char * test_param[] = {"1", "0x4A", "i", "2", "r", "+"};
	const char * test_param[] = {"1", "0x4A", "j", "0"};
//	const char * test_param[] = {"1", "0x4A", "k"};
	rdc200a_test_main(-3, test_param);
//	DPRINTF("TEST_RDC200A");
	set_rdc200a_output_90Hz();
	// set_rdc200a_output_60Hz();

#if TEST_SWITCH_2D3D_View
	rdc200a_sbs_on();
#endif

	#endif
#if TEST_RDC200A_PATTERN
	RDC_REG_SET(0x082B, 0x00);
	RDC_REG_SET(0x092B, 0x00);
	RDC_REG_SET(0x0200, 0x02);
	RDC_REG_SET(0x0280, 0x01);
#endif

#if TEST_GUESTURE
	dev76xx_assignI2CHandl(&hi2c2);

	dev76xx_setupResetPin();

#ifdef ISR_MODE
	dev76xx_setupInterruptPin();
#endif
	dev76xx_reset();
	dev76xx_init();
#endif

#if TEST_IMU
	if (init_imu())
	{

	}
#endif

	/* Initialization of ALS parameters to the default values */
#if TEST_ALS
	initialize_CM32183E(als_code);
#endif
#if TEST_PS
	initialize_VCNL36828P();
#endif
  /* USER CODE END 2 */
	Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
	current_brightness = User_Data[brightness];// It was read for flash.
	set_led_brightness(current_brightness);
	display_on = TRUE;

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		//uint8_t val;
		// JOSEPH: Move from HAL_GPIO_EXTI_Callback for better imu performance @20220518
		// JOSEPH: (40) => ~220 frames/second, (80) => ~306 frames/second (but other sensors will slower)
#if 1 // betta changed
		process_command();
		HAL_Delay(POLL_DURATION); //=> ok, 24
		//process_sensor();
		do_others();
		if (ps_close_state==FALSE)
				if (0==display_timeout)
				{
					HAL_GPIO_WritePin(LED_D_EN_L_GPIO_Port, LED_D_EN_L_Pin, GPIO_PIN_RESET); // PB0
					HAL_GPIO_WritePin(LED_D_EN_R_GPIO_Port, LED_D_EN_R_Pin, GPIO_PIN_RESET); // PB1
				}
				else if (display_timeout>0)
					display_timeout--;
#else  // betta changed
		if (loop_counter<=80) {
#if TEST_IMU
			process_sensor();
#endif
		}
		else {
			loop_counter = 0;
#if TEST_DO_OTHERS
			do_others();
#endif
		}

		HAL_Delay(1);
		// JOSEPH: If only delay 500us, imu will send up to 800hz (need to check host performance, 835 maybe too busy) @20220518
		//	  delay_us(500);
		loop_counter++;
#endif  // betta changed

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00702991;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00702991;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 0;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535; // betta
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_D_EN_L_Pin|LED_D_EN_R_Pin|DEV76XX_RSTN_Pin|VIBRATER_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LT7911D_RESET_Pin|SECURITY_KEY_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, FAN_EN_Pin|IR_LED_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RDC200A_RSTB_H_Pin */
  GPIO_InitStruct.Pin = RDC200A_RSTB_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RDC200A_RSTB_H_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IMU_INT2_H_Pin SOS_KEY_Pin */
  GPIO_InitStruct.Pin = IMU_INT2_H_Pin|SOS_KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PROX_INT_H_Pin */
  GPIO_InitStruct.Pin = PROX_INT_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(PROX_INT_H_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ALS_INTN_H_Pin LED_D_FAULT_L_Pin IMU_INT1_H_Pin LED_D_FAULT_R_Pin */
  GPIO_InitStruct.Pin = ALS_INTN_H_Pin|LED_D_FAULT_L_Pin|IMU_INT1_H_Pin|LED_D_FAULT_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_D_EN_L_Pin LED_D_EN_R_Pin DEV76XX_RSTN_Pin VIBRATER_EN_Pin */
  GPIO_InitStruct.Pin = LED_D_EN_L_Pin|LED_D_EN_R_Pin|DEV76XX_RSTN_Pin|VIBRATER_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BRIGHTNESS_MINUS_Pin BRIGHTNESS_PLUS_Pin RDC200A_BOOTING_INDICATION_Pin */
#if 1 // betta changed
  GPIO_InitStruct.Pin = RDC200A_BOOTING_INDICATION_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BRIGHTNESS_MINUS_Pin|BRIGHTNESS_PLUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // To support tap detection interrupt
  GPIO_InitStruct.Pin = IMU_INT1_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;//GPIO_MODE_IT_RISING is okay too;
  GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLDOWN is okay too;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#else
  GPIO_InitStruct.Pin = BRIGHTNESS_MINUS_Pin|BRIGHTNESS_PLUS_Pin|RDC200A_BOOTING_INDICATION_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif

  /*Configure GPIO pins : LT7911D_RESET_Pin SECURITY_KEY_Pin */
  GPIO_InitStruct.Pin = LT7911D_RESET_Pin|SECURITY_KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : FAN_EN_Pin IR_LED_EN_Pin */
  GPIO_InitStruct.Pin = FAN_EN_Pin|IR_LED_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : VSYNC_H_Pin */
  GPIO_InitStruct.Pin = VSYNC_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(VSYNC_H_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
/*
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
*/

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
