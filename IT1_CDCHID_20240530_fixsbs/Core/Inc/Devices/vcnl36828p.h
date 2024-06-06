#ifndef VCNL36828P_H_
#define VCNL36828P_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//-------------------------------------------------
// Global definition for VCNL36828P Registers
//-------------------------------------------------
#define VCNL36828P_I2C1_ADD1
#define VCNL36828P_SLAVE_ADD1_7bit		0x60  // Used
#define VCNL36828P_SLAVE_ADD2_7bit		0x51  // Un-used (SCL and SDA reversed)
#define VCNL36828P_PS_CONF1_L			0x00
#define VCNL36828P_PS_CONF1_H			0x00
#define VCNL36828P_PS_CONF2_L			0x01
#define VCNL36828P_PS_CONF2_H			0x01
#define VCNL36828P_PS_CONF3_L			0x02
#define VCNL36828P_PS_CONF3_H			0x02
#define VCNL36828P_PS_THDL				0x03
#define VCNL36828P_PS_THDH				0x04
#define VCNL36828P_PS_CANC				0x05
#define VCNL36828P_PS_DATA				0xF8
#define VCNL36828P_INT_FLAG				0xF9
#define VCNL36828P_ID_CMD				0xFA

//Table 2 Datasheet Register 0x00 Low Byte
#define VCNL36828P_START				0x0080	//0x01<<7		// Must be set to "1" when power on ready (0x70)
//Reserved
#define VCNL36828P_PS_SD_EN				0x0001	//0x01<<0		// 1: PS enable
#define VCNL36828P_PS_SD_DIS			0x0000	//0x00<<0		// 0: PS shutdown

//Table 3 Datasheet Register 0x00 High Byte
//Reserved
#define VCNL36828P_PS_HD_EN				0x4000	//0x01<<6
#define VCNL36828P_PS_HD_DIS			0x0000	//0x00<<6
#define VCNL36828P_PS_SP_INT_EN			0x2000	//0x01<<5
#define VCNL36828P_PS_SP_INT_DIS		0x0000	//0x00<<5
#define VCNL36828P_PS_SMART_PERS_EN		0x1000	//0x01<<4
#define VCNL36828P_PS_SMART_PERS_DIS	0x0000	//0x00<<4
#define VCNL36828P_PS_PERS_1			0x0000	//0x00<<2
#define VCNL36828P_PS_PERS_2			0x0400	//0x01<<2
#define VCNL36828P_PS_PERS_3			0x0800	//0x02<<2
#define VCNL36828P_PS_PERS_4			0x0C00	//0x03<<2
#define VCNL36828P_PS_INT_DIS			0x0000	//0x00<<0		// 0: PS Interrupt Disable
#define VCNL36828P_PS_INT_LOGIC			0x0100	//0x01<<0		// 1: Logic High/Low mode
#define VCNL36828P_PS_INT_FIRST_HIGH	0x0200	//0x02<<0		// 2: 1st trigger by high threshold window
#define VCNL36828P_PS_INT_EN			0x0300	//0x03<<0		// 3: Trigger by each high/low threshold window

//Table 4 Datasheet Register 0x01 Low Byte
#define VCNL36828P_PS_PERIOD_50ms		0x0000	//0x00<<6
#define VCNL36828P_PS_PERIOD_100ms		0x0040	//0x01<<6
#define VCNL36828P_PS_PERIOD_200ms		0x0080	//0x02<<6
#define VCNL36828P_PS_PERIOD_400ms		0x00C0	//0x03<<6
#define VCNL36828P_PS_IT_1T				0x0000	//0x00<<4
#define VCNL36828P_PS_IT_2T				0x0010	//0x01<<4
#define VCNL36828P_PS_IT_4T				0x0020	//0x02<<4
#define VCNL36828P_PS_IT_8T				0x0030	//0x03<<4
#define VCNL36828P_PS_MPS_1				0x0000	//0x00<<2
#define VCNL36828P_PS_MPS_2				0x0004	//0x01<<2
#define VCNL36828P_PS_MPS_4				0x0008	//0x02<<2
#define VCNL36828P_PS_MPS_8				0x000C	//0x03<<2
#define VCNL36828P_PS_ITB_25			0x0000	//0x00<<1
#define VCNL36828P_PS_ITB_50			0x0002	//0x01<<1
#define VCNL36828P_PS_HG_x1				0x0000	//0x00<<0
#define VCNL36828P_PS_HG_x2				0x0001	//0x01<<0

//Table 5 Datasheet Register 0x01 High Byte
//Reserved
#define VCNL36828P_PS_SENS_HIGH			0x2000	//0x01<<5
#define VCNL36828P_PS_SENS_NORMAL		0x0000	//0x00<<5
#define VCNL36828P_PS_OFFSET_EN			0x1000	//0x01<<4
#define VCNL36828P_PS_OFFSET_DIS		0x0000	//0x00<<4
//Reserved
#define VCNL36828P_PS_LED_8_3mA			0x0000	//0x00<<0		// 0: 8.3mA
#define VCNL36828P_PS_LED_9_7mA			0x0100	//0x01<<0		// 1: 9.7mA
#define VCNL36828P_PS_LED_11_7mA		0x0200	//0x02<<0		// 2: 11.7mA
#define VCNL36828P_PS_LED_13_1mA		0x0300	//0x03<<0		// 3: 13.1mA
#define VCNL36828P_PS_LED_15_7mA		0x0400	//0x04<<0		// 4: 15.7mA
#define VCNL36828P_PS_LED_17_1mA		0x0500	//0x05<<0		// 5: 17.1mA
#define VCNL36828P_PS_LED_19_1mA		0x0600	//0x06<<0		// 6: 19.1mA
#define VCNL36828P_PS_LED_20_4mA		0x0700	//0x07<<0		// 7: 20.4mA

//Table 6 Datasheet Register 0x02 Low Byte
//Reserved
#define VCNL36828P_PS_TRIG_EN			0x0020	//0x01<<5		// Enable PS force-mode trigger
#define VCNL36828P_PS_TRIG_DIS			0x0000	//0x00<<5		// Disable PS force-mode trigger
#define VCNL36828P_PS_AF_EN				0x0010	//0x01<<4		// Enable PS force-mode
#define VCNL36828P_PS_AF_DIS			0x0000	//0x00<<4		// Disable PS force-mode
//Reserved

//Table 7 Datasheet Register 0x02 High Byte (PS short period setting)
//Reserved
#define VCNL36828P_PS_SPERIOD_PERIOD	0x0000	//0x00<<0		// 0: follow PS_PERIOD register setting
#define VCNL36828P_PS_SPERIOD_6_25ms	0x4000	//0x01<<6		// 1: 6.25ms
#define VCNL36828P_PS_SPERIOD_12_5ms	0x8000	//0x02<<6		// 2: 12.5ms
#define VCNL36828P_PS_SPERIOD_25ms		0xC000	//0x03<<6		// 3: 25ms

//Table 17 Datasheet Register 0xF9 High Byte
#define VCNL36828P_PS_INVALID			0x4000	//0x01<<6		// PS Enter Sunlight Protect Flag (bit 6)
//Reserved
#define VCNL36828P_PS_SPFLAG			0x1000	//0x01<<4		// PS Enter Sunlight Protect Flag (bit 4)
//Reserved
#define VCNL36828P_PS_IF_CLOSE			0x0200	//0x01<<1		// PS crossing High Threshold INT trigger event (bit 1)
#define VCNL36828P_PS_IF_AWAY			0x0100	//0x01<<0		// PS crossing Low Threshold INT trigger event (bit 0)

//#define VCNL36828P_I2C1        			I2C1_BASE

//volatile bool write_prox_sensor_complete_flag;


extern void initialize_VCNL36828P(void);
extern uint16_t VCNL36828P_read_word(uint8_t command);
extern void VCNL36828P_write_word(uint8_t command, uint16_t val);
extern void I2Cm_bWriteBytes(uint8_t Reg_num, uint8_t Data_lsb, uint8_t Data_msb, uint8_t bCnt);
extern void I2Cm_fReadBytes(uint8_t Reg_num, uint8_t *Data, uint8_t bCnt);

extern void VCNL36828P_SET_PS_LowThreshold(uint16_t LowThreshold);
extern void VCNL36828P_SET_PS_HighThreshold(uint16_t HighThreshold);
extern uint16_t VCNL36828P_GET_PS_LowThreshold();
extern uint16_t VCNL36828P_GET_PS_HighThreshold();
extern void VCNL36828P_SET_PS_ST(uint16_t ps_st);
extern void VCNL36828P_SET_PS_ON(uint16_t ps_on);
extern void VCNL36828P_SET_PS_HD(uint16_t ps_hd);
extern void VCNL36828P_SET_PS_SP_INT(uint16_t ps_sp_int);
extern void VCNL36828P_SET_PS_SMART_PERS(uint16_t Pers);
extern void VCNL36828P_SET_PS_PERS(uint16_t ps_pers);
extern void VCNL36828P_SET_PS_INT(uint16_t ps_int);
extern void VCNL36828P_SET_PS_PERIOD(uint16_t ps_period);
extern void VCNL36828P_SET_PS_IT(uint16_t ps_it);
extern void VCNL36828P_SET_PS_MPS(uint16_t ps_mps);
extern void VCNL36828P_SET_PS_ITB(uint16_t ps_itb);
extern void VCNL36828P_SET_PS_HG(uint16_t ps_hg);
extern void VCNL36828P_SET_PS_LED(uint16_t ps_led);
extern void VCNL36828P_SET_PS_TRIG(uint16_t TriggerBit);
extern void VCNL36828P_SET_PS_AF(uint16_t ps_af);
extern void VCNL36828P_SET_PS_SP(uint16_t ps_sp);
extern void VCNL36828P_WRITE_Reg(uint8_t Reg, uint8_t Data_lsb, uint8_t Data_msb);
extern uint16_t VCNL36828P_READ_Reg(uint8_t Reg);

extern uint16_t get_ps_if(void);

extern uint32_t CalibValue;
#ifdef __cplusplus
}
#endif

#endif /* VCNL36828P_H_ */
