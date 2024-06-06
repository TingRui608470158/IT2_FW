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

bool read_button_states(int8_t *led_brightness_buttons);
void set_led_brightness(int8_t brightness);
void control_backlight(uint16_t lux_level);
int8_t convert_lux_to_brightness(uint16_t lux_level);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_DRIVERS_BRIGHTNESS_H_ */
