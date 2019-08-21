################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UI.c \
../src/XPT2046.c \
../src/amg8833.c \
../src/button.c \
../src/clock.c \
../src/delay.c \
../src/display_buffer.c \
../src/displaylib.c \
../src/fonts.c \
../src/leds.c \
../src/main.c \
../src/mcuperipherals.c \
../src/scheduler.c \
../src/services.c \
../src/stm32f4xx_it.c \
../src/syscalls.c \
../src/system_stm32f4xx.c 

OBJS += \
./src/UI.o \
./src/XPT2046.o \
./src/amg8833.o \
./src/button.o \
./src/clock.o \
./src/delay.o \
./src/display_buffer.o \
./src/displaylib.o \
./src/fonts.o \
./src/leds.o \
./src/main.o \
./src/mcuperipherals.o \
./src/scheduler.o \
./src/services.o \
./src/stm32f4xx_it.o \
./src/syscalls.o \
./src/system_stm32f4xx.o 

C_DEPS += \
./src/UI.d \
./src/XPT2046.d \
./src/amg8833.d \
./src/button.d \
./src/clock.d \
./src/delay.d \
./src/display_buffer.d \
./src/displaylib.d \
./src/fonts.d \
./src/leds.d \
./src/main.d \
./src/mcuperipherals.d \
./src/scheduler.d \
./src/services.d \
./src/stm32f4xx_it.d \
./src/syscalls.d \
./src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32F407VGTx -DSTM32F4 -DSTM32F4DISCOVERY -DSTM32 -DDEBUG -DUSE_STDPERIPH_DRIVER -DSTM32F40XX -DSTM32F40_41xxx -I"/Users/daymoon/Documents/workspace/stm32f4discovery_stdperiph_lib" -I"/Users/daymoon/Documents/workspace/Thermal_camera/inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


