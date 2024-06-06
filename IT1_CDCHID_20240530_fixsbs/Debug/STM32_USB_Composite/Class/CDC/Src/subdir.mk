################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../STM32_USB_Composite/Class/CDC/Src/usbd_cdc.c 

OBJS += \
./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.o 

C_DEPS += \
./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.d 


# Each subdirectory must supply rules for building sources it contributes
STM32_USB_Composite/Class/CDC/Src/%.o STM32_USB_Composite/Class/CDC/Src/%.su STM32_USB_Composite/Class/CDC/Src/%.cyclo: ../STM32_USB_Composite/Class/CDC/Src/%.c STM32_USB_Composite/Class/CDC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -DCFG_MDC_RDC200A -DCFG_PANEL_RDP551F -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/Devices -I../Core/Inc/Drivers -I../USB_DEVICE_COMPOSITE/App -I../USB_DEVICE_COMPOSITE/Target -I../STM32_USB_Composite/Core/Inc -I../STM32_USB_Composite/Class/HID/Inc -I../STM32_USB_Composite/Class/CDC/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-STM32_USB_Composite-2f-Class-2f-CDC-2f-Src

clean-STM32_USB_Composite-2f-Class-2f-CDC-2f-Src:
	-$(RM) ./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.cyclo ./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.d ./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.o ./STM32_USB_Composite/Class/CDC/Src/usbd_cdc.su

.PHONY: clean-STM32_USB_Composite-2f-Class-2f-CDC-2f-Src

