#include "main.h"
#include "lm3435.h"

// Button state. 0 = button pressed in, 1 = button off
bool button_mode_state = FALSE;
bool button_plus_state = FALSE;
bool button_minus_state = FALSE;
// Store the current led brightness
uint8_t led_brightness_current = -1;
uint16_t lux_level_current = 0;

// Store both the button brightness
int8_t led_button_change_size = 1; // Increase/decrease brightness by this amount

bool button_mode = FALSE;
double lux_gamma =  2.2;
float gamma_index[] = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};


// 根據原始lux_out值, 找到新的基準值Vin
int16_t inverse_gamma(float c_range, int16_t lux_out)
{
	int16_t Vin = lux_out / pow(c_range, lux_gamma);
	return Vin;
}

// 根據比例, 找到Vout
int16_t positive_gamma(uint8_t index, int16_t Vin)
{
	int16_t Vout = Vin * pow(gamma_index[index], lux_gamma);
	return Vout;
}



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

int8_t read_button_states(int8_t c_brightness)
{
	/* Check the pressed button */
//	uint8_t UserTxBufferFS[4];// betta added
	uint8_t tmp[30]={0};
	float Vin;
	float current_range;

	if(HAL_GPIO_ReadPin(BRIGHTNESS_MINUS_GPIO_Port,BRIGHTNESS_MINUS_Pin)==0) {
		// Set Mode on button release
	    //*led_brightness_buttons = convert_lux_to_brightness(lux_level_current);
		if(c_brightness>0)
			c_brightness--;


	//	UserTxBufferFS
#if 0
		tmp[0] = 0x22;
		tmp[1] = 0xdd;
		tmp[2] = 0xff;
		tmp[3] = 0x01;	// Press on the brightness- button
	    //sendCdcData((uint8_t*)&UserTxBufferFS[0], 4);


#endif

//	    goto out;
	}else if(HAL_GPIO_ReadPin(BRIGHTNESS_PLUS_GPIO_Port,BRIGHTNESS_PLUS_Pin)== 0) {
		//*led_brightness_buttons = convert_lux_to_brightness(lux_level_current);
		// Increase Brightness
		if(c_brightness<5)
			c_brightness++;
//	UserTxBufferFS
#if 0
		tmp[0] = 0x22;
		tmp[1] = 0xdd;
		tmp[2] = 0xff;
		tmp[3] = 0x02;	// Press on the brightness+ button
	    //sendCdcData((uint8_t*)&UserTxBufferFS[0], 4);
#endif

	}
	return c_brightness;
}

void set_led_brightness(int8_t local_brightness)
{
	uint32_t User_Data[USER_DATA_SIZE];
	uint8_t tmp[25]={0};

	// Validate
#if 0 // betta changed
	if(local_brightness > 15)
	{
		local_brightness = 15;
	}
#else
	if(local_brightness > 5)
	{
		local_brightness = 5;
	}
#endif
	if(local_brightness < 0)
	{
		local_brightness = 0;
	}
	if(led_brightness_current != local_brightness)
	{
		led_brightness_current = local_brightness;

		//DPRINTF("set_led_brightness: Brightness=%d\r\n", brightness);
		LM3435_Write_Current_LED((uint8_t) local_brightness);

		Flash_Read_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, User_Data, USER_DATA_SIZE);
		User_Data[brightness]= local_brightness;
		Flash_Write_Data(FLASH_ADDRESS_FOR_DEVICE_DATA_BYTE, (uint64_t *)User_Data, USER_DATA_SIZE, FLASH_TYPEPROGRAM_DOUBLEWORD);
		//HAL_Delay(50);
	}
	sprintf(tmp, "BRIGHTNESS-LEVEL=%d \r\n",local_brightness);
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


