################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Devices/ak09918.c \
../Core/Src/Devices/cm32183e.c \
../Core/Src/Devices/dev76xx.c \
../Core/Src/Devices/icm42688p.c \
../Core/Src/Devices/lm3435.c \
../Core/Src/Devices/rdc200_lut_patch.c \
../Core/Src/Devices/rdc200a.c \
../Core/Src/Devices/rdc200a_flash.c \
../Core/Src/Devices/rdp551f.c \
../Core/Src/Devices/rti_vc_delay.c \
../Core/Src/Devices/rti_vc_devif.c \
../Core/Src/Devices/rti_vc_dload.c \
../Core/Src/Devices/rti_vc_main.c \
../Core/Src/Devices/rti_vc_panel.c \
../Core/Src/Devices/rti_vc_rdc.c \
../Core/Src/Devices/rti_vc_regio.c \
../Core/Src/Devices/test_common.c \
../Core/Src/Devices/test_download.c \
../Core/Src/Devices/test_main.c \
../Core/Src/Devices/test_rdc200a.c \
../Core/Src/Devices/test_rdc200a_auto.c \
../Core/Src/Devices/vc_android_usb.c \
../Core/Src/Devices/vcnl36828p.c 

OBJS += \
./Core/Src/Devices/ak09918.o \
./Core/Src/Devices/cm32183e.o \
./Core/Src/Devices/dev76xx.o \
./Core/Src/Devices/icm42688p.o \
./Core/Src/Devices/lm3435.o \
./Core/Src/Devices/rdc200_lut_patch.o \
./Core/Src/Devices/rdc200a.o \
./Core/Src/Devices/rdc200a_flash.o \
./Core/Src/Devices/rdp551f.o \
./Core/Src/Devices/rti_vc_delay.o \
./Core/Src/Devices/rti_vc_devif.o \
./Core/Src/Devices/rti_vc_dload.o \
./Core/Src/Devices/rti_vc_main.o \
./Core/Src/Devices/rti_vc_panel.o \
./Core/Src/Devices/rti_vc_rdc.o \
./Core/Src/Devices/rti_vc_regio.o \
./Core/Src/Devices/test_common.o \
./Core/Src/Devices/test_download.o \
./Core/Src/Devices/test_main.o \
./Core/Src/Devices/test_rdc200a.o \
./Core/Src/Devices/test_rdc200a_auto.o \
./Core/Src/Devices/vc_android_usb.o \
./Core/Src/Devices/vcnl36828p.o 

C_DEPS += \
./Core/Src/Devices/ak09918.d \
./Core/Src/Devices/cm32183e.d \
./Core/Src/Devices/dev76xx.d \
./Core/Src/Devices/icm42688p.d \
./Core/Src/Devices/lm3435.d \
./Core/Src/Devices/rdc200_lut_patch.d \
./Core/Src/Devices/rdc200a.d \
./Core/Src/Devices/rdc200a_flash.d \
./Core/Src/Devices/rdp551f.d \
./Core/Src/Devices/rti_vc_delay.d \
./Core/Src/Devices/rti_vc_devif.d \
./Core/Src/Devices/rti_vc_dload.d \
./Core/Src/Devices/rti_vc_main.d \
./Core/Src/Devices/rti_vc_panel.d \
./Core/Src/Devices/rti_vc_rdc.d \
./Core/Src/Devices/rti_vc_regio.d \
./Core/Src/Devices/test_common.d \
./Core/Src/Devices/test_download.d \
./Core/Src/Devices/test_main.d \
./Core/Src/Devices/test_rdc200a.d \
./Core/Src/Devices/test_rdc200a_auto.d \
./Core/Src/Devices/vc_android_usb.d \
./Core/Src/Devices/vcnl36828p.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Devices/%.o Core/Src/Devices/%.su Core/Src/Devices/%.cyclo: ../Core/Src/Devices/%.c Core/Src/Devices/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -DCFG_MDC_RDC200A -DCFG_PANEL_RDP551F -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/Devices -I../Core/Inc/Drivers -I../USB_DEVICE_COMPOSITE/App -I../USB_DEVICE_COMPOSITE/Target -I../STM32_USB_Composite/Core/Inc -I../STM32_USB_Composite/Class/HID/Inc -I../STM32_USB_Composite/Class/CDC/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Devices

clean-Core-2f-Src-2f-Devices:
	-$(RM) ./Core/Src/Devices/ak09918.cyclo ./Core/Src/Devices/ak09918.d ./Core/Src/Devices/ak09918.o ./Core/Src/Devices/ak09918.su ./Core/Src/Devices/cm32183e.cyclo ./Core/Src/Devices/cm32183e.d ./Core/Src/Devices/cm32183e.o ./Core/Src/Devices/cm32183e.su ./Core/Src/Devices/dev76xx.cyclo ./Core/Src/Devices/dev76xx.d ./Core/Src/Devices/dev76xx.o ./Core/Src/Devices/dev76xx.su ./Core/Src/Devices/icm42688p.cyclo ./Core/Src/Devices/icm42688p.d ./Core/Src/Devices/icm42688p.o ./Core/Src/Devices/icm42688p.su ./Core/Src/Devices/lm3435.cyclo ./Core/Src/Devices/lm3435.d ./Core/Src/Devices/lm3435.o ./Core/Src/Devices/lm3435.su ./Core/Src/Devices/rdc200_lut_patch.cyclo ./Core/Src/Devices/rdc200_lut_patch.d ./Core/Src/Devices/rdc200_lut_patch.o ./Core/Src/Devices/rdc200_lut_patch.su ./Core/Src/Devices/rdc200a.cyclo ./Core/Src/Devices/rdc200a.d ./Core/Src/Devices/rdc200a.o ./Core/Src/Devices/rdc200a.su ./Core/Src/Devices/rdc200a_flash.cyclo ./Core/Src/Devices/rdc200a_flash.d ./Core/Src/Devices/rdc200a_flash.o ./Core/Src/Devices/rdc200a_flash.su ./Core/Src/Devices/rdp551f.cyclo ./Core/Src/Devices/rdp551f.d ./Core/Src/Devices/rdp551f.o ./Core/Src/Devices/rdp551f.su ./Core/Src/Devices/rti_vc_delay.cyclo ./Core/Src/Devices/rti_vc_delay.d ./Core/Src/Devices/rti_vc_delay.o ./Core/Src/Devices/rti_vc_delay.su ./Core/Src/Devices/rti_vc_devif.cyclo ./Core/Src/Devices/rti_vc_devif.d ./Core/Src/Devices/rti_vc_devif.o ./Core/Src/Devices/rti_vc_devif.su ./Core/Src/Devices/rti_vc_dload.cyclo ./Core/Src/Devices/rti_vc_dload.d ./Core/Src/Devices/rti_vc_dload.o ./Core/Src/Devices/rti_vc_dload.su ./Core/Src/Devices/rti_vc_main.cyclo ./Core/Src/Devices/rti_vc_main.d ./Core/Src/Devices/rti_vc_main.o ./Core/Src/Devices/rti_vc_main.su ./Core/Src/Devices/rti_vc_panel.cyclo ./Core/Src/Devices/rti_vc_panel.d ./Core/Src/Devices/rti_vc_panel.o ./Core/Src/Devices/rti_vc_panel.su ./Core/Src/Devices/rti_vc_rdc.cyclo ./Core/Src/Devices/rti_vc_rdc.d ./Core/Src/Devices/rti_vc_rdc.o ./Core/Src/Devices/rti_vc_rdc.su ./Core/Src/Devices/rti_vc_regio.cyclo ./Core/Src/Devices/rti_vc_regio.d ./Core/Src/Devices/rti_vc_regio.o ./Core/Src/Devices/rti_vc_regio.su ./Core/Src/Devices/test_common.cyclo ./Core/Src/Devices/test_common.d ./Core/Src/Devices/test_common.o ./Core/Src/Devices/test_common.su ./Core/Src/Devices/test_download.cyclo ./Core/Src/Devices/test_download.d ./Core/Src/Devices/test_download.o ./Core/Src/Devices/test_download.su ./Core/Src/Devices/test_main.cyclo ./Core/Src/Devices/test_main.d ./Core/Src/Devices/test_main.o ./Core/Src/Devices/test_main.su ./Core/Src/Devices/test_rdc200a.cyclo ./Core/Src/Devices/test_rdc200a.d ./Core/Src/Devices/test_rdc200a.o ./Core/Src/Devices/test_rdc200a.su ./Core/Src/Devices/test_rdc200a_auto.cyclo ./Core/Src/Devices/test_rdc200a_auto.d ./Core/Src/Devices/test_rdc200a_auto.o ./Core/Src/Devices/test_rdc200a_auto.su ./Core/Src/Devices/vc_android_usb.cyclo ./Core/Src/Devices/vc_android_usb.d ./Core/Src/Devices/vc_android_usb.o ./Core/Src/Devices/vc_android_usb.su ./Core/Src/Devices/vcnl36828p.cyclo ./Core/Src/Devices/vcnl36828p.d ./Core/Src/Devices/vcnl36828p.o ./Core/Src/Devices/vcnl36828p.su

.PHONY: clean-Core-2f-Src-2f-Devices

