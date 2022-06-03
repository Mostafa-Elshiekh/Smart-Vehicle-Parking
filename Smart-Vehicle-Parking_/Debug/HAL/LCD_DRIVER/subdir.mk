################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL/LCD_DRIVER/lcd_me.c 

OBJS += \
./HAL/LCD_DRIVER/lcd_me.o 

C_DEPS += \
./HAL/LCD_DRIVER/lcd_me.d 


# Each subdirectory must supply rules for building sources it contributes
HAL/LCD_DRIVER/lcd_me.o: ../HAL/LCD_DRIVER/lcd_me.c
	arm-none-eabi-gcc -gdwarf-2 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C6Tx -DDEBUG -c -I"C:/Users/musta/STM32CubeIDE/workspace_1.4.0/ECU3Smart_CAR_Parking/STM32_F103C6_DRIVERS" -I"C:/Users/musta/STM32CubeIDE/workspace_1.4.0/ECU3Smart_CAR_Parking/HAL/include" -I"C:/Users/musta/STM32CubeIDE/workspace_1.4.0/ECU3Smart_CAR_Parking/STM32_F103C6_DRIVERS/INC" -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"HAL/LCD_DRIVER/lcd_me.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

