################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_DEVICE_COMPOSITE/Target/subdir.mk
-include USB_DEVICE_COMPOSITE/App/subdir.mk
-include STM32_USB_Composite/Core/Src/subdir.mk
-include STM32_USB_Composite/Class/HID/Src/subdir.mk
-include STM32_USB_Composite/Class/CDC/Src/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/Drivers/subdir.mk
-include Core/Src/Devices/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := IT1_CDCHID_20240530_fixsbs
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
IT1_CDCHID_20240530_fixsbs.elf \

MAP_FILES += \
IT1_CDCHID_20240530_fixsbs.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
IT1_CDCHID_20240530_fixsbs.list \

OBJCOPY_BIN += \
IT1_CDCHID_20240530_fixsbs.bin \


# All Target
all: main-build

# Main-build Target
main-build: IT1_CDCHID_20240530_fixsbs.elf secondary-outputs

# Tool invocations
IT1_CDCHID_20240530_fixsbs.elf IT1_CDCHID_20240530_fixsbs.map: $(OBJS) $(USER_OBJS) C:\Users\USER\Desktop\Ray\IT2\stm32\project\0605\IT1_CDCHID_20240530_fixsbs\STM32L412CBUX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "IT1_CDCHID_20240530_fixsbs.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\USER\Desktop\Ray\IT2\stm32\project\0605\IT1_CDCHID_20240530_fixsbs\STM32L412CBUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="IT1_CDCHID_20240530_fixsbs.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

IT1_CDCHID_20240530_fixsbs.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "IT1_CDCHID_20240530_fixsbs.list"
	@echo 'Finished building: $@'
	@echo ' '

IT1_CDCHID_20240530_fixsbs.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "IT1_CDCHID_20240530_fixsbs.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) IT1_CDCHID_20240530_fixsbs.bin IT1_CDCHID_20240530_fixsbs.elf IT1_CDCHID_20240530_fixsbs.list IT1_CDCHID_20240530_fixsbs.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
