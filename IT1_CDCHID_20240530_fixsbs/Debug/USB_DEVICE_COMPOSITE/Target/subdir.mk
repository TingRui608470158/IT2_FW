################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE_COMPOSITE/Target/usbd_conf.c 

OBJS += \
./USB_DEVICE_COMPOSITE/Target/usbd_conf.o 

C_DEPS += \
./USB_DEVICE_COMPOSITE/Target/usbd_conf.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE_COMPOSITE/Target/%.o USB_DEVICE_COMPOSITE/Target/%.su USB_DEVICE_COMPOSITE/Target/%.cyclo: ../USB_DEVICE_COMPOSITE/Target/%.c USB_DEVICE_COMPOSITE/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -DCFG_MDC_RDC200A -DCFG_PANEL_RDP551F -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/Devices -I../Core/Inc/Drivers -I../USB_DEVICE_COMPOSITE/App -I../USB_DEVICE_COMPOSITE/Target -I../STM32_USB_Composite/Core/Inc -I../STM32_USB_Composite/Class/HID/Inc -I../STM32_USB_Composite/Class/CDC/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE_COMPOSITE-2f-Target

clean-USB_DEVICE_COMPOSITE-2f-Target:
	-$(RM) ./USB_DEVICE_COMPOSITE/Target/usbd_conf.cyclo ./USB_DEVICE_COMPOSITE/Target/usbd_conf.d ./USB_DEVICE_COMPOSITE/Target/usbd_conf.o ./USB_DEVICE_COMPOSITE/Target/usbd_conf.su

.PHONY: clean-USB_DEVICE_COMPOSITE-2f-Target

