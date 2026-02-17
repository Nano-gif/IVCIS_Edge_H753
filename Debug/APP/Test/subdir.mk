################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/Test/test_mode_switch.c \
../APP/Test/test_motion_detect.c \
../APP/Test/test_net_client.c \
../APP/Test/test_net_diag.c \
../APP/Test/test_power_manager.c \
../APP/Test/test_vision_init.c 

OBJS += \
./APP/Test/test_mode_switch.o \
./APP/Test/test_motion_detect.o \
./APP/Test/test_net_client.o \
./APP/Test/test_net_diag.o \
./APP/Test/test_power_manager.o \
./APP/Test/test_vision_init.o 

C_DEPS += \
./APP/Test/test_mode_switch.d \
./APP/Test/test_motion_detect.d \
./APP/Test/test_net_client.d \
./APP/Test/test_net_diag.d \
./APP/Test/test_power_manager.d \
./APP/Test/test_vision_init.d 


# Each subdirectory must supply rules for building sources it contributes
APP/Test/%.o APP/Test/%.su APP/Test/%.cyclo: ../APP/Test/%.c APP/Test/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H753xx -c -I"E:/STM32/OV/APP/Inc" -I"E:/STM32/OV/APP" -I"E:/STM32/OV/APP/src" -I"E:/STM32/OV/Drivers/BSP" -I"E:/STM32/OV/Drivers/BSP/ov5640" -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I"E:/STM32/OV/APP/Test" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-APP-2f-Test

clean-APP-2f-Test:
	-$(RM) ./APP/Test/test_mode_switch.cyclo ./APP/Test/test_mode_switch.d ./APP/Test/test_mode_switch.o ./APP/Test/test_mode_switch.su ./APP/Test/test_motion_detect.cyclo ./APP/Test/test_motion_detect.d ./APP/Test/test_motion_detect.o ./APP/Test/test_motion_detect.su ./APP/Test/test_net_client.cyclo ./APP/Test/test_net_client.d ./APP/Test/test_net_client.o ./APP/Test/test_net_client.su ./APP/Test/test_net_diag.cyclo ./APP/Test/test_net_diag.d ./APP/Test/test_net_diag.o ./APP/Test/test_net_diag.su ./APP/Test/test_power_manager.cyclo ./APP/Test/test_power_manager.d ./APP/Test/test_power_manager.o ./APP/Test/test_power_manager.su ./APP/Test/test_vision_init.cyclo ./APP/Test/test_vision_init.d ./APP/Test/test_vision_init.o ./APP/Test/test_vision_init.su

.PHONY: clean-APP-2f-Test

