#include "main.h"
#include "i2c.h"
#include "stm32l4xx_hal.h"
#include "lm3435.h"

/* The matrix has 4 addressable rows. Rows is selected by a brightness
 * level on the corresponding I2C address pin.
 *
 * Row addr | 0x00 | 0x01 | 0x02 | 0x03
 * ---------+------+------+------+------
 * Line A   | 0x00 | 0x0F | 0x15 | 0x0C
 * Line B   | 0x38 | 0x13 | 0x19 | 0x0F
 * Line C   | 0x20 | 0x17 | 0x1E | 0x13
 * Line D   | 0x20 | 0x1B | 0x23 | 0x17
 * Line E   | 0x18 | 0x1F | 0x27 | 0x1B
 * Line F   | 0x00 | 0x23 | 0x2C | 0x1F
 * Line G   | 0x00 | 0x27 | 0x31 | 0x23
 * Line H   | 0x38 | 0x2B | 0x35 | 0x26
 * Line I   | 0x20 | 0x2F | 0x3A | 0x2A
 * Line J   | 0x20 | 0x33 | 0x3F | 0x2E
 * Line K   | 0x18 | 0x37 | 0x43 | 0x32
 * Line L   | 0x00 | 0x3B | 0x48 | 0x36
 * Line M   | 0x00 | 0x3F | 0x4D | 0x3A
 * Line N   | 0x38 | 0x43 | 0x51 | 0x3D
 * Line O   | 0x20 | 0x47 | 0x56 | 0x41
 * Line P   | 0x20 | 0x4B | 0x5B | 0x45
 */
/*
 * store the 1d array values for the matrix
 */
// CH: Increase brightness up three levels
#if 1 //  betta changed
/*
LM_Current rgb_current[] = {
						{ 0, 0x0F, 0x15, 0x0C },		// Brightness -
						{ 0x10, 0x37, 0x43, 0x32 },		// Brightness -
						{ 0x20, 0x5f, 0x72, 0x58 },		// Brightness -
						{ 0x00, 0x87, 0xA1, 0x7F },		// Brightness -
						{ 0x10, 0xAF, 0xCF, 0xA5 },		// Brightness -
						{ 0x20, 0xD7, 0xFE, 0xCB },		// Boot up
						};

*/

LM_Current rgb_current[] = {
						{ 0, 0x0F, 0x15, 0x0C },		// Brightness -
						{ 0x20, 0x17, 0x1E, 0x13 },		// Brightness -
						{ 0x20, 0x2F, 0x3A, 0x2A },		// Brightness -
						{ 0, 0x53, 0x64, 0x4d },		// Brightness -
						{ 0x38, 0x8B, 0xA5, 0x82 },		// Brightness -
						{ 0x20, 0xD7, 0xFE, 0xCB },		// Boot up
						};
#else
LM_Current rgb_current[] = {
						{ 0x00, 0x0F, 0x15, 0x0C },		// Brightness -

						{ 0x20, 0x1B, 0x23, 0x17 },		// Brightness -

						{ 0x00, 0x27, 0x31, 0x23 },		// Brightness -

						{ 0x20, 0x33, 0x3F, 0x2E },		// Brightness -

						{ 0x00, 0x3F, 0x4D, 0x3A },		// Brightness -

						{ 0x20, 0x4B, 0x5B, 0x45 },		// Boot up
						};
/*
LM_Current rgb_current[] = {
						{ 0x00, 0x0F, 0x15, 0x0C },		// Brightness -
						{ 0x38, 0x13, 0x19, 0x0F },		// Brightness -
						{ 0x20, 0x17, 0x1E, 0x13 },		// Brightness -
						{ 0x20, 0x1B, 0x23, 0x17 },		// Brightness -
						{ 0x18, 0x1F, 0x27, 0x1B },		// Brightness -
						{ 0x00, 0x23, 0x2C, 0x1F },		// Brightness -
						{ 0x00, 0x27, 0x31, 0x23 },		// Brightness -
						{ 0x38, 0x2B, 0x35, 0x26 },		// Brightness -
						{ 0x20, 0x2F, 0x3A, 0x2A },		// Brightness -
						{ 0x20, 0x33, 0x3F, 0x2E },		// Brightness -
						{ 0x18, 0x37, 0x43, 0x32 },		// Brightness -
						{ 0x00, 0x3B, 0x48, 0x36 },		// Brightness -
						{ 0x00, 0x3F, 0x4D, 0x3A },		// Brightness -
						{ 0x38, 0x43, 0x51, 0x3D },		// Brightness -
						{ 0x20, 0x47, 0x56, 0x41 },		// Brightness -
						{ 0x20, 0x4B, 0x5B, 0x45 },		// Boot up
						};
*/
#endif
void LM3435_Write(uint8_t reg, uint8_t val)
{
	return I2CWriteByte(&LM3435_I2C1, LM3435_7bit_ADDR, reg, val);
}

void LM3435_Write_Current_LED(uint8_t vol)
{
	LM3435_Write(LM3435_LEDLO, rgb_current[vol].LEDLO);
	LM3435_Write(LM3435_GLEDH, rgb_current[vol].GLEDH);
	LM3435_Write(LM3435_BLEDH, rgb_current[vol].BLEDH);
	LM3435_Write(LM3435_RLEDH, rgb_current[vol].RLEDH);
}