#ifndef LM3435_H_
#define LM3435_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <stdint.h>
#include "main.h"

#define LM3435_ADDR  			0x50
#define LM3435_7bit_ADDR		0x28

#define LM3435_LEDLO	0x00
#define LM3435_GLEDH	0x01
#define LM3435_BLEDH	0x02
#define LM3435_RLEDH	0x03
#define LM3435_FLT_		0x05
#define LM3435_DELAY	0x06
#define LM3435_FAULT	0x07

typedef struct
{
	uint8_t LEDLO;
	uint8_t GLEDH;
	uint8_t BLEDH;
	uint8_t RLEDH;
}LM_Current;

bool LM3435_L_setupResetPin();
bool LM3435_R_setupResetPin();
void LM3435_Write_Current_LED(uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif /* LM3435_H_ */
