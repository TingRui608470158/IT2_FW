#include "main.h"
#include "lm3435.h"

// Button state. 0 = button pressed in, 1 = button off
bool button_mode_state = FALSE;
bool button_plus_state = FALSE;
bool button_minus_state = FALSE;
int8_t glux_index ;
// Store the current led brightness
uint8_t led_brightness_current = -1;
uint16_t lux_level_current = 0;

// Store both the button brightness
int8_t led_button_change_size = 1; // Increase/decrease brightness by this amount

bool button_mode = FALSE;

int8_t convert_lux_to_brightness(uint16_t lux_level)
{
	uint16_t Quotient = lux_level / 3;
	uint16_t remainder = lux_level % 3;
	if (remainder == 2)
		return (int8_t) (Quotient + 1) * 3;
	else if (remainder == 1)
		return (int8_t) Quotient * 3;
	else
		return (int8_t) lux_level;
}


int8_t read_button_states(int8_t c_lux_index)
{
	/* Check the pressed button */
	bool sts=FALSE;
	uint8_t UserTxBufferFS[4];// betta added
	uint8_t tmp[30]={0};
	uint32_t User_Data[USER_DATA_SIZE];

	if(HAL_GPIO_ReadPin(BRIGHTNESS_MINUS_GPIO_Port,BRIGHTNESS_MINUS_Pin)==0) {
		// Set Mode on button release
	    //*led_brightness_buttons = convert_lux_to_brightness(lux_level_current);

		if(c_lux_index>0)
			c_lux_index--;

		//	UserTxBufferFS
#if 0
		tmp[0] = 0x22;
		tmp[1] = 0xdd;
		tmp[2] = 0xff;
		tmp[3] = 0x01;	// Press on the brightness- button
	    //sendCdcData((uint8_t*)&UserTxBufferFS[0], 4);

#endif
	    sts = TRUE;
//	    goto out;
	}else if(HAL_GPIO_ReadPin(BRIGHTNESS_PLUS_GPIO_Port,BRIGHTNESS_PLUS_Pin)== 0) {
		//*led_brightness_buttons = convert_lux_to_brightness(lux_level_current);
		// Increase Brightness
		if(c_lux_index<4)
			c_lux_index++;
	    //	UserTxBufferFS
#if 0
		tmp[0] = 0x22;
		tmp[1] = 0xdd;
		tmp[2] = 0xff;
		tmp[3] = 0x02;	// Press on the brightness+ button
	    //sendCdcData((uint8_t*)&UserTxBufferFS[0], 4);
#endif
	    sts = TRUE;
	}
	glux_index = c_lux_index;


	if (sts) {
		Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
		User_Data[lux_index]= glux_index;
		Flash_Write_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, (uint64_t *)User_Data, USER_DATA_SIZE, FLASH_TYPEPROGRAM_DOUBLEWORD);
		#if 0
		sprintf(tmp, "glux_index=%d\n",User_Data[lux_index]);
		sendCdcData((uint8_t*)tmp, strlen(tmp));
		#endif
	}
	return c_lux_index;
}

void set_led_brightness(int8_t brightness)
{
	uint8_t tmp[25]={0};

	// Validate
#if 0 // betta changed
	if(brightness > 15)
	{
		brightness = 15;
	}
#else
	if(brightness > 5)
	{
		brightness = 5;
	}
#endif
	if(brightness < 0)
	{
		brightness = 0;
	}
	if(led_brightness_current != brightness)
	{
		led_brightness_current = brightness;
		//DPRINTF("set_led_brightness: Brightness=%d\r\n", brightness);
		LM3435_Write_Current_LED((uint8_t) brightness);
		//HAL_Delay(50);
	}
	sprintf(tmp, "BRIGHTNESS-LEVEL=%d \r\n",brightness);
	sendCdcData((uint8_t*)tmp, strlen(tmp));
}

void control_backlight(uint16_t lux_level)
{
	if (lux_level_current != lux_level) {
		if (lux_level > lux_level_current) {
			for (size_t i = lux_level_current; i < lux_level; i++) {
//				DPRINTF("control_backlight: lux_level=%d\r\n", lux_level);
				LM3435_Write_Current_LED((uint8_t) (i + 1));
			}
			lux_level_current = lux_level;
		} else {
			for (size_t i = lux_level_current; i > lux_level; i--) {
//				DPRINTF("control_backlight: lux_level=%d\r\n", lux_level);
				LM3435_Write_Current_LED((uint8_t) (i - 1));
			}
			lux_level_current = lux_level;
		}
		led_brightness_current = lux_level_current;
	}
}
