################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/aes.c \
../Core/Src/Drivers/brightness.c \
../Core/Src/Drivers/i2c.c \
../Core/Src/Drivers/imu.c \
../Core/Src/Drivers/switch_view.c \
../Core/Src/Drivers/tools.c \
../Core/Src/Drivers/usbd_composite.c 

OBJS += \
./Core/Src/Drivers/aes.o \
./Core/Src/Drivers/brightness.o \
./Core/Src/Drivers/i2c.o \
./Core/Src/Drivers/imu.o \
./Core/Src/Drivers/switch_view.o \
./Core/Src/Drivers/tools.o \
./Core/Src/Drivers/usbd_composite.o 

C_DEPS += \
./Core/Src/Drivers/aes.d \
./Core/Src/Drivers/brightness.d \
./Core/Src/Drivers/i2c.d \
./Core/Src/Drivers/imu.d \
./Core/Src/Drivers/switch_view.d \
./Core/Src/Drivers/tools.d \
./Core/Src/Drivers/usbd_composite.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/%.o Core/Src/Drivers/%.su Core/Src/Drivers/%.cyclo: ../Core/Src/Drivers/%.c Core/Src/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -DCFG_MDC_RDC200A -DCFG_PANEL_RDP551F -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/Devices -I../Core/Inc/Drivers -I../USB_DEVICE_COMPOSITE/App -I../USB_DEVICE_COMPOSITE/Target -I../STM32_USB_Composite/Core/Inc -I../STM32_USB_Composite/Class/HID/Inc -I../STM32_USB_Composite/Class/CDC/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers

clean-Core-2f-Src-2f-Drivers:
	-$(RM) ./Core/Src/Drivers/aes.cyclo ./Core/Src/Drivers/aes.d ./Core/Src/Drivers/aes.o ./Core/Src/Drivers/aes.su ./Core/Src/Drivers/brightness.cyclo ./Core/Src/Drivers/brightness.d ./Core/Src/Drivers/brightness.o ./Core/Src/Drivers/brightness.su ./Core/Src/Drivers/i2c.cyclo ./Core/Src/Drivers/i2c.d ./Core/Src/Drivers/i2c.o ./Core/Src/Drivers/i2c.su ./Core/Src/Drivers/imu.cyclo ./Core/Src/Drivers/imu.d ./Core/Src/Drivers/imu.o ./Core/Src/Drivers/imu.su ./Core/Src/Drivers/switch_view.cyclo ./Core/Src/Drivers/switch_view.d ./Core/Src/Drivers/switch_view.o ./Core/Src/Drivers/switch_view.su ./Core/Src/Drivers/tools.cyclo ./Core/Src/Drivers/tools.d ./Core/Src/Drivers/tools.o ./Core/Src/Drivers/tools.su ./Core/Src/Drivers/usbd_composite.cyclo ./Core/Src/Drivers/usbd_composite.d ./Core/Src/Drivers/usbd_composite.o ./Core/Src/Drivers/usbd_composite.su

.PHONY: clean-Core-2f-Src-2f-Drivers

