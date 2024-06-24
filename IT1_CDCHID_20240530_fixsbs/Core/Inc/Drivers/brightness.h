/*
 * brightness.h
 *
 *  Created on: Aug 8, 2022
 *      Author: ASUS
 */
#ifndef INC_DRIVERS_BRIGHTNESS_H_
#define INC_DRIVERS_BRIGHTNESS_H_

#ifdef __cplusplus
extern "C" {
#endif
int16_t inverse_gamma(float c_range, int16_t lux_out);
int16_t positive_gamma(uint8_t index, int16_t lux_in);
int8_t read_button_states(int8_t c_brightness);
void set_led_brightness(int8_t brightness);
void control_backlight(uint16_t lux_level);
int8_t convert_lux_to_brightness(uint16_t lux_level);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_DRIVERS_BRIGHTNESS_H_ */
