################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/src/Motion_Detect.c \
../APP/src/Net_Client.c \
../APP/src/Power_Manager.c \
../APP/src/Vision_Pipeline.c \
../APP/src/nanoprintf_impl.c \
../APP/src/test_motion_detect.c \
../APP/src/vision_capture.c \
../APP/src/vision_uart_tx.c 

OBJS += \
./APP/src/Motion_Detect.o \
./APP/src/Net_Client.o \
./APP/src/Power_Manager.o \
./APP/src/Vision_Pipeline.o \
./APP/src/nanoprintf_impl.o \
./APP/src/test_motion_detect.o \
./APP/src/vision_capture.o \
./APP/src/vision_uart_tx.o 

C_DEPS += \
./APP/src/Motion_Detect.d \
./APP/src/Net_Client.d \
./APP/src/Power_Manager.d \
./APP/src/Vision_Pipeline.d \
./APP/src/nanoprintf_impl.d \
./APP/src/test_motion_detect.d \
./APP/src/vision_capture.d \
./APP/src/vision_uart_tx.d 


# Each subdirectory must supply rules for building sources it contributes
APP/src/%.o APP/src/%.su APP/src/%.cyclo: ../APP/src/%.c APP/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H753xx -c -I"E:/STM32/OV/APP/Inc" -I"E:/STM32/OV/APP" -I"E:/STM32/OV/APP/src" -I"E:/STM32/OV/Drivers/BSP" -I"E:/STM32/OV/Drivers/BSP/ov5640" -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I"E:/STM32/OV/APP/Test" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-APP-2f-src

clean-APP-2f-src:
	-$(RM) ./APP/src/Motion_Detect.cyclo ./APP/src/Motion_Detect.d ./APP/src/Motion_Detect.o ./APP/src/Motion_Detect.su ./APP/src/Net_Client.cyclo ./APP/src/Net_Client.d ./APP/src/Net_Client.o ./APP/src/Net_Client.su ./APP/src/Power_Manager.cyclo ./APP/src/Power_Manager.d ./APP/src/Power_Manager.o ./APP/src/Power_Manager.su ./APP/src/Vision_Pipeline.cyclo ./APP/src/Vision_Pipeline.d ./APP/src/Vision_Pipeline.o ./APP/src/Vision_Pipeline.su ./APP/src/nanoprintf_impl.cyclo ./APP/src/nanoprintf_impl.d ./APP/src/nanoprintf_impl.o ./APP/src/nanoprintf_impl.su ./APP/src/test_motion_detect.cyclo ./APP/src/test_motion_detect.d ./APP/src/test_motion_detect.o ./APP/src/test_motion_detect.su ./APP/src/vision_capture.cyclo ./APP/src/vision_capture.d ./APP/src/vision_capture.o ./APP/src/vision_capture.su ./APP/src/vision_uart_tx.cyclo ./APP/src/vision_uart_tx.d ./APP/src/vision_uart_tx.o ./APP/src/vision_uart_tx.su

.PHONY: clean-APP-2f-src

