################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
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

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
NRF24_N_SOLAR_LION3.lss \

FLASH_IMAGE += \
NRF24_N_SOLAR_LION3.hex \

EEPROM_IMAGE += \
NRF24_N_SOLAR_LION3.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: NRF24_N_SOLAR_LION3.elf secondary-outputs

# Tool invocations
NRF24_N_SOLAR_LION3.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,NRF24_N_SOLAR_LION3.map -Wl,--gc-sections -mmcu=atmega328p -o "NRF24_N_SOLAR_LION3.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

NRF24_N_SOLAR_LION3.lss: NRF24_N_SOLAR_LION3.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S NRF24_N_SOLAR_LION3.elf  >"NRF24_N_SOLAR_LION3.lss"
	@echo 'Finished building: $@'
	@echo ' '

NRF24_N_SOLAR_LION3.hex: NRF24_N_SOLAR_LION3.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex NRF24_N_SOLAR_LION3.elf  "NRF24_N_SOLAR_LION3.hex"
	@echo 'Finished building: $@'
	@echo ' '

NRF24_N_SOLAR_LION3.eep: NRF24_N_SOLAR_LION3.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex NRF24_N_SOLAR_LION3.elf  "NRF24_N_SOLAR_LION3.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: NRF24_N_SOLAR_LION3.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p NRF24_N_SOLAR_LION3.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) NRF24_N_SOLAR_LION3.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents

-include ../makefile.targets
