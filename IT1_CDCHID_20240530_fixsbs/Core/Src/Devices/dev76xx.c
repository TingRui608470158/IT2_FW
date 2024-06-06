/*
 * Dev7635.cpp
 *
 *  Created on: May 15, 2020
 *      Author: lake
 */

#include "dev76xx.h"
#include "stdio.h"
#include "main.h"
#include "../../../USB_DEVICE_COMPOSITE/App/usb_device.h"


//#define I2C_TIMEOUT (10000)
#define I2C_TIMEOUT (1000)

#define PIXART_I2C_DEV_ADDR 0x73

#define REG_BANK 0xEF
// Bank 0
#define REG_MODULE_PART_ID  0X01
#define REG_GES_EN          0x07
#define REG_SUSP_CTRL       0x03
#define REG_GESTURE_FLAG1   0X43
#define REG_GESTURE_FLAG2   0X44
// Bank 1
#define REG_EXP_FIXED       0x5F
// Bank 2
#define REG_EFUSE           0X76
#define REG_EXP_RULE_FLAG1  0X77
#define REG_EXP_RULE_FLAG2  0X78

#define MODULE_ID 0x76
//

extern USBD_HandleTypeDef hUsbDeviceFS;
typedef struct
{
//    Gpio(GPIO_TypeDef* port, uint16_t pin) : mPort(port), mPin(pin)
    GPIO_TypeDef* mPort;
    uint16_t mPin;
}Gpio;

static I2C_HandleTypeDef *sI2CHandle;

#ifdef ISR_MODE
Gpio GpioDev76xxInt = {DEV76XX_INT_GPIO_Port, DEV76XX_INT_Pin};
#endif
Gpio GpioDev76xxRst = {DEV76XX_RSTN_GPIO_Port, DEV76XX_RSTN_Pin};

bool gSensorISR = false;

void dev76xx_assignI2CHandl(I2C_HandleTypeDef *i2cHandle)
{
    sI2CHandle = i2cHandle;
}
#ifdef ISR_MODE
bool dev76xx_setupInterruptPin()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GpioDev76xxInt.mPin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GpioDev76xxInt.mPort, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(DEV76XX_INT_EXTI_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DEV76XX_INT_EXTI_IRQn);
    return true;
}
#endif



bool dev76xx_setupResetPin()
{
//    std::cout << "dev76xx_setupResetPin()" << "\n\r";
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GpioDev76xxRst.mPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GpioDev76xxRst.mPort, &GPIO_InitStruct);
    return true;
}

bool dev76xx_writeReg(uint8_t regAddr, uint8_t regData)
{

    bool ret = true;
    uint8_t buf[] = {regAddr, regData};
    __disable_irq();
    HAL_StatusTypeDef res = HAL_I2C_Master_Transmit(sI2CHandle, PIXART_I2C_DEV_ADDR << 1,
            buf, 2, I2C_TIMEOUT);
    if(res != HAL_OK)
    {
        ret = false;
    }
    __enable_irq();
    return ret;
}

bool dev76xx_readReg(uint8_t regAddr, uint8_t *regData)
{
    bool ret = true;

    __disable_irq();
    if((HAL_I2C_Master_Transmit(sI2CHandle, PIXART_I2C_DEV_ADDR << 1,
            &regAddr, 1, I2C_TIMEOUT) != HAL_OK)
        || (HAL_I2C_Master_Receive(sI2CHandle, PIXART_I2C_DEV_ADDR << 1,
                regData, 1, I2C_TIMEOUT) != HAL_OK))
    {
        ret = false;
    }
    __enable_irq();
    return ret;
}



bool dev76xx_init()
{
    uint8_t buf[] = {
          0xEF, 0x00,
          0x03, 0x01,
          0x04, 0x00,
          0x07, 0x01,
          0x51, 0x10,
          0x57, 0x0B,
          0x5A, 0x03,
          0x60, 0x30,
          0x69, 0x3C,
          0x6A, 0x28,
          0x6C, 0x00,
          0xF0, 0x01,
          0xEF, 0x01,
          0x04, 0x03,
          0x41, 0x44,
          0x64, 0x06,
          0x77, 0x31,
          0x78, 0x07,
          0x7E, 0x00,
          0xEF, 0x02,
          0x20, 0x01,
          0x27, 0x08,
          0x98, 0x30,
    };
    for (uint16_t i=0; i<(sizeof(buf)/2); ++i)
    {
#ifdef DEBUG
//        std::cout << std::setfill('0') << std::setw(2) << std::hex
//                  << "I2C W -> 0x"  << (int)buf[i*2] << ": 0x" << (int)buf[i*2+1]
//                  << std::dec << "\n\r";
#endif
        if (!dev76xx_writeReg(buf[i*2], buf[i*2+1]))
        {
//            std::cout << "Initial failed." << "\n\r";
            return false;
        }
    }

    return dev76xx_expo();
}

void dev76xx_reset()
{
	printf("dev76xx_reset\r\n");
    HAL_GPIO_WritePin(GpioDev76xxRst.mPort, GpioDev76xxRst.mPin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GpioDev76xxRst.mPort, GpioDev76xxRst.mPin, GPIO_PIN_SET);
    HAL_Delay(20);
}

bool dev76xx_switchBank(uint8_t bank)
{
//	printf("dev76xx_switchBank\r\n");
    bool ret = dev76xx_writeReg(REG_BANK, bank);
    if (!ret) {
//        std::cout << "Bank switch failed." << "\n\r";
//    	printf("dev76xx_switchBank: Bank switch failed.\r\n");
        return false;
    }
    return true;
}

// Exposure Time Control
bool dev76xx_expo()
{
    // Bank 2
    static const uint8_t REG_CODE = 0x79;
    // Bank 0
    static const uint8_t REG_TRIM_CODE = 0x02;
    static const uint8_t REG_AE_Exposure_UB_L = 0x48;
    static const uint8_t REG_AE_Exposure_UB_H = 0x49;
    static const uint8_t REG_AE_Exposure_LB_L = 0x4A;
    static const uint8_t REG_AE_Exposure_LB_H = 0x4B;

    const int set_exp = 120;
    const int base1 = 135;
    const int base2x1000 = 3226;
    const int base3 = 85;
    int step1, step2;
    int flag1, flag2, efuse_bit58;
    int new_exp;
    int trim_code;
    int R_AE_Exposure_UB, R_AE_Exposure_LB;
    uint8_t tmp = 0;

    // Switch to bank 0.
    if (!dev76xx_switchBank(0)) return false;
    bool ret = dev76xx_readReg(REG_TRIM_CODE, &tmp);
    if (!ret)
    {
//        std::cout << "Read REG_TRIM_CODE failed." << "\n\r";
        return false;
    }
    trim_code = tmp >> 4;
//    std::cout << "trim_code =" << trim_code << "\n\r";

    if (trim_code == 0) {
        // No
        new_exp = set_exp;
    } else if (trim_code == 1 || trim_code == 2) {
        // Yes
        int code;
        // Switch to bank 2.
        if (!dev76xx_switchBank(2)) return false;

        if (!dev76xx_readReg(REG_EXP_RULE_FLAG1, &tmp))
        {
//            std::cout << "Read REG_EXP_RULE_FLAG1 failed." << "\n\r";
            return false;
        }
        flag1 = tmp & 0x01 /*bit [0]*/;
        if (!dev76xx_readReg(REG_EXP_RULE_FLAG2, &tmp))
        {
//            std::cout << "Read REG_EXP_RULE_FLAG2 failed." << "\n\r";
            return false;
        }
        flag2 = tmp & 0xFC /*bit [7:2]*/;

        if (!dev76xx_readReg(REG_CODE, &tmp))
        {
//            std::cout << "Read REG_CODE failed." << "\n\r";
            return false;
        }
        code = tmp >> 3;
//        std::cout << "code =" << code << "\n\r";
        if (code >= 31) {
//            std::cout << "Illegal code value " << code << "." << "\n\r";
            return false;
        }
        step1 = base1 * set_exp;
//        std::cout << "base1 * set_exp        = " << step1 << "\n\r";
        step2 = (code*base2x1000)/1000;
//        std::cout << "(code*base2x1000)/1000 = " << step2 << "\n\r";

        if (!dev76xx_readReg(REG_EFUSE, &tmp))
        {
//            std::cout << "Read REG_EFUSE failed." << "\n\r";
            return false;
        }
        efuse_bit58 = (tmp & 0xFC /*bit [7:2]*/)>>2;

        if ((flag1 == 0) && (flag2 == 0))
        {
//            std::cout << "new" << "\n\r";
            if (efuse_bit58 == 63)
                new_exp = step1*125/((step2+base3)*100);
            else if (efuse_bit58 == 62)
                new_exp = (step1*125/((step2+base3)*100))*107/130;
            else
            {
//                std::cout << "Invalid efuse value " << efuse_bit58 << "\n\r";
                return false;
            }
        }
        else
        {
//            std::cout << "old" << "\n\r";
            new_exp = step1/(step2+base3);
        }

        if (trim_code == 1) {
            if (!dev76xx_switchBank(1)) return false;

            uint8_t verify = 0;
            dev76xx_writeReg(REG_EXP_FIXED, 0x39);
            dev76xx_readReg(REG_EXP_FIXED, &verify);
//            std::cout << "Read back - REG_EXP_FIXED: 0x"
//                    << std::hex << verify << std::dec << "\n\r";
        }
    }
    else
    {
//        std::cout << "Invalid trim_code value." << "\n\r";
        new_exp = set_exp;
    }

//    std::cout << "new_exp = " << new_exp << "\n\r";

    R_AE_Exposure_UB = new_exp;
    R_AE_Exposure_LB = new_exp*1000/2000;
//    std::cout << "R_AE_Exposure_UB = " << R_AE_Exposure_UB << "\n\r";
//    std::cout << "R_AE_Exposure_LB = " << R_AE_Exposure_LB << "\n\r";
    // Switch to bank 0
    if (!dev76xx_switchBank(0)) return false;

    dev76xx_writeReg(REG_AE_Exposure_UB_L, R_AE_Exposure_UB & 0xFF);
    dev76xx_writeReg(REG_AE_Exposure_UB_H, (R_AE_Exposure_UB >> 8) & 0xFF );
    dev76xx_writeReg(REG_AE_Exposure_LB_L, R_AE_Exposure_LB & 0xFF);
    dev76xx_writeReg(REG_AE_Exposure_LB_H, (R_AE_Exposure_LB >> 8) & 0xFF );
    return true;
}

//char* motionStr1[] = {
//        "Up", "Down", "Left", "Right", "Forward",
//        "Backward", "Clockwise", "Counterclockwise"
//};
//char* motionStr2[] = {
//        "Wave", "Hover", "Clockwise continuous", "Counterclockwise continuous"
//};

//char* motionStr[] = {
//        "Up", "Down", "Left", "Right", "Forward",
//        "Backward", "Clockwise", "Counterclockwise","","",
//				"Wave", "Hover", "Clockwise continuous", "Counterclockwise continuous",""
//};

/*
 * CH: Swap the right to left and left to right swipe gestures, between forward and backward,
 * between clockwise and counterclockwise rotation, and between clockwise continuous and
 * counterclockwise continuous.
 */
char* motionStr[] = {
        "Up", "Down", "Right", "Left", "Backward",
        "Forward", "Counterclockwise", "Clockwise","","",
				"Wave", "Hover", "Counterclockwise continuous", "Clockwise continuous",""
};

int dev76xx_detectGesture()
{
#ifdef ISR_MODE
    __disable_irq();
    if (!gSensorISR) return;
//    else
//        std::cout << "INT coming.." << "\n\r";
    gSensorISR = false;
    __enable_irq();
#endif
    // Switch to bank 0
    if (!dev76xx_switchBank(0)) return -1;

    bool ret = true;
    uint8_t flag1, flag2;
    ret &= dev76xx_readReg(REG_GESTURE_FLAG1, &flag1);
    ret &= dev76xx_readReg(REG_GESTURE_FLAG2, &flag2);
    if (!ret)
    {
    	printf("Read REG_GESTURE_FLAG failed.\n\r");
        return -1;
    }
#ifdef DEBUG
//    std::cout << std::setfill('0') << std::setw(2) << std::hex
//                      << "Flag1 0x" << (int)flag1
//                      << "Flag2 0x" << (int)flag2
//                      << std::dec << "\n\r";
//	printf("Flag1 0x%x Flag2 0x%x\n\r", flag1, flag2);
#endif
    if (flag1)
    {
        //for (int i=0; i<(sizeof(motionStr1)/sizeof(motionStr1[0])); ++i)
    	for (int i=0; i<8; ++i)
        {
            if (((flag1 >> i) & 0x01)) {
//            	printf("%s\n\r", motionStr[i]);
            	return i;
            }
        }
    }
    if (flag2)
    {
        int i = 0;
        if (flag2 & 0x01)
        {
//            std::cout << motionStr2[i] << "\n\r";
//        	printf("%s\n\r", motionStr[10+i]);
        	return (10+i);
        }
        i++;
        if (flag2 & 0x02)
        {
//            std::cout << motionStr2[i] << "\n\r";
//        	printf("%s\n\r", motionStr[10+i]);
        	return (10+i);
        }
        i++;
        if ((flag2 & 0x40) && !(flag2 & 0x04))
        {
//            std::cout << motionStr2[i] << "\n\r";
//        	printf("%s\n\r", motionStr[10+i]);
        	return (10+i);
        }
        i++;
        if ((flag2 & 0x80) && !(flag2 & 0x04))
        {
//            std::cout << motionStr2[i] << "\n\r";
//        	printf("%s\n\r", motionStr[10+i]);
        	return (10+i);
        }
    }
    return -1;
}

#ifdef __cplusplus
extern "C" {
#endif
//void EXTI15_10_IRQHandler(void)
//{
//  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
//}
#ifdef __cplusplus
}
#endif
