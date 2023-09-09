################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../crc8.c \
../ds18x20.c \
../komendy_at.c \
../main.c \
../mkuart.c \
../nrf24.c \
../onewire.c \
../power_save.c \
../spi.c \
../timer.c 

OBJS += \
./crc8.o \
./ds18x20.o \
./komendy_at.o \
./main.o \
./mkuart.o \
./nrf24.o \
./onewire.o \
./power_save.o \
./spi.o \
./timer.o 

C_DEPS += \
./crc8.d \
./ds18x20.d \
./komendy_at.d \
./main.d \
./mkuart.d \
./nrf24.d \
./onewire.d \
./power_save.d \
./spi.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


