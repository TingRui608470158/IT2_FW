/*
 * Dev7635.h
 *
 *  Created on: May 15, 2020
 *      Author: lake
 */

#ifndef DEV76XX_H_
#define DEV76XX_H_

//#define ISR_MODE


#ifdef __cplusplus
extern "C" {
#endif
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>


extern void dev76xx_assignI2CHandl(I2C_HandleTypeDef *i2cHandle);
extern bool dev76xx_setupInterruptPin();
extern bool dev76xx_setupResetPin();

extern bool dev76xx_init();
extern void dev76xx_reset();
extern bool dev76xx_expo();
extern int dev76xx_detectGesture();

extern bool dev76xx_writeReg(uint8_t regAddr, uint8_t regData);
extern bool dev76xx_readReg(uint8_t regAddr, uint8_t *regData);
extern bool dev76xx_switchBank(uint8_t bank);

extern char* motionStr[];
extern bool gSensorISR;

#ifdef __cplusplus
}
#endif

#endif /* DEV76XX_H_ */
