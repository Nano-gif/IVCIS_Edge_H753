################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/src/Net_Client.c \
../APP/src/Vision_Pipeline.c 

OBJS += \
./APP/src/Net_Client.o \
./APP/src/Vision_Pipeline.o 

C_DEPS += \
./APP/src/Net_Client.d \
./APP/src/Vision_Pipeline.d 


# Each subdirectory must supply rules for building sources it contributes
APP/src/%.o APP/src/%.su APP/src/%.cyclo: ../APP/src/%.c APP/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H753xx -c -I"E:/STM32/OV/APP/Inc" -I"E:/STM32/OV/APP" -I"E:/STM32/OV/APP/src" -I"E:/STM32/OV/Drivers/BSP" -I"E:/STM32/OV/Drivers/BSP/ov5640" -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Utilities/JPEG -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I"E:/STM32/OV/APP" -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-APP-2f-src

clean-APP-2f-src:
	-$(RM) ./APP/src/Net_Client.cyclo ./APP/src/Net_Client.d ./APP/src/Net_Client.o ./APP/src/Net_Client.su ./APP/src/Vision_Pipeline.cyclo ./APP/src/Vision_Pipeline.d ./APP/src/Vision_Pipeline.o ./APP/src/Vision_Pipeline.su

.PHONY: clean-APP-2f-src

