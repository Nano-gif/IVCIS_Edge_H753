################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../APP/src/Alarm_Handler.c \
../APP/src/Auto_Exposure.c \
../APP/src/Control_Manager.c \
../APP/src/Modbus_RTU.c \
../APP/src/Modbus_RegMap.c \
../APP/src/Motion_Detect.c \
../APP/src/Net_Client.c \
../APP/src/Net_Client_Util.c \
../APP/src/Power_Manager.c \
../APP/src/RS485_Driver.c \
../APP/src/RS485_Manager.c \
../APP/src/Radar_Manager.c \
../APP/src/Servo_Control.c \
../APP/src/Transport_ETH.c \
../APP/src/Transport_HAL.c \
../APP/src/Vision_Pipeline.c \
../APP/src/nanoprintf_impl.c \
../APP/src/test_motion_detect.c \
../APP/src/vision_capture.c \
../APP/src/vision_uart_tx.c

OBJS += \
./APP/src/Alarm_Handler.o \
./APP/src/Auto_Exposure.o \
./APP/src/Control_Manager.o \
./APP/src/Modbus_RTU.o \
./APP/src/Modbus_RegMap.o \
./APP/src/Motion_Detect.o \
./APP/src/Net_Client.o \
./APP/src/Net_Client_Util.o \
./APP/src/Power_Manager.o \
./APP/src/RS485_Driver.o \
./APP/src/RS485_Manager.o \
./APP/src/Radar_Manager.o \
./APP/src/Servo_Control.o \
./APP/src/Transport_ETH.o \
./APP/src/Transport_HAL.o \
./APP/src/Vision_Pipeline.o \
./APP/src/nanoprintf_impl.o \
./APP/src/test_motion_detect.o \
./APP/src/vision_capture.o \
./APP/src/vision_uart_tx.o

C_DEPS += \
./APP/src/Alarm_Handler.d \
./APP/src/Auto_Exposure.d \
./APP/src/Control_Manager.d \
./APP/src/Modbus_RTU.d \
./APP/src/Modbus_RegMap.d \
./APP/src/Motion_Detect.d \
./APP/src/Net_Client.d \
./APP/src/Net_Client_Util.d \
./APP/src/Power_Manager.d \
./APP/src/RS485_Driver.d \
./APP/src/RS485_Manager.d \
./APP/src/Radar_Manager.d \
./APP/src/Servo_Control.d \
./APP/src/Transport_ETH.d \
./APP/src/Transport_HAL.d \
./APP/src/Vision_Pipeline.d \
./APP/src/nanoprintf_impl.d \
./APP/src/test_motion_detect.d \
./APP/src/vision_capture.d \
./APP/src/vision_uart_tx.d

# Each subdirectory must supply rules for building sources it contributes
APP/src/%.o APP/src/%.su APP/src/%.cyclo: ../APP/src/%.c APP/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DTEST_SELECT=0 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I"E:/STM32/OV/Drivers/BSP" -I"E:/STM32/OV/Drivers/BSP/ov5640" -I"E:/STM32/OV/APP" -I"E:/STM32/OV/APP/Inc" -I"E:/STM32/OV/APP/src" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Utilities/JPEG -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-APP-2f-src

clean-APP-2f-src:
	-$(RM) ./APP/src/Alarm_Handler.cyclo ./APP/src/Alarm_Handler.d ./APP/src/Alarm_Handler.o ./APP/src/Alarm_Handler.su ./APP/src/Auto_Exposure.cyclo ./APP/src/Auto_Exposure.d ./APP/src/Auto_Exposure.o ./APP/src/Auto_Exposure.su ./APP/src/Control_Manager.cyclo ./APP/src/Control_Manager.d ./APP/src/Control_Manager.o ./APP/src/Control_Manager.su ./APP/src/Modbus_RTU.cyclo ./APP/src/Modbus_RTU.d ./APP/src/Modbus_RTU.o ./APP/src/Modbus_RTU.su ./APP/src/Modbus_RegMap.cyclo ./APP/src/Modbus_RegMap.d ./APP/src/Modbus_RegMap.o ./APP/src/Modbus_RegMap.su ./APP/src/Motion_Detect.cyclo ./APP/src/Motion_Detect.d ./APP/src/Motion_Detect.o ./APP/src/Motion_Detect.su ./APP/src/Net_Client.cyclo ./APP/src/Net_Client.d ./APP/src/Net_Client.o ./APP/src/Net_Client.su ./APP/src/Net_Client_Util.cyclo ./APP/src/Net_Client_Util.d ./APP/src/Net_Client_Util.o ./APP/src/Net_Client_Util.su ./APP/src/Power_Manager.cyclo ./APP/src/Power_Manager.d ./APP/src/Power_Manager.o ./APP/src/Power_Manager.su ./APP/src/RS485_Manager.cyclo ./APP/src/RS485_Manager.d ./APP/src/RS485_Manager.o ./APP/src/RS485_Manager.su ./APP/src/Radar_Manager.cyclo ./APP/src/Radar_Manager.d ./APP/src/Radar_Manager.o ./APP/src/Radar_Manager.su ./APP/src/Servo_Control.cyclo ./APP/src/Servo_Control.d ./APP/src/Servo_Control.o ./APP/src/Servo_Control.su ./APP/src/Transport_ETH.cyclo ./APP/src/Transport_ETH.d ./APP/src/Transport_ETH.o ./APP/src/Transport_ETH.su ./APP/src/Transport_HAL.cyclo ./APP/src/Transport_HAL.d ./APP/src/Transport_HAL.o ./APP/src/Transport_HAL.su ./APP/src/Vision_Pipeline.cyclo ./APP/src/Vision_Pipeline.d ./APP/src/Vision_Pipeline.o ./APP/src/Vision_Pipeline.su ./APP/src/nanoprintf_impl.cyclo ./APP/src/nanoprintf_impl.d ./APP/src/nanoprintf_impl.o ./APP/src/nanoprintf_impl.su ./APP/src/test_motion_detect.cyclo ./APP/src/test_motion_detect.d ./APP/src/test_motion_detect.o ./APP/src/test_motion_detect.su ./APP/src/vision_capture.cyclo ./APP/src/vision_capture.d ./APP/src/vision_capture.o ./APP/src/vision_capture.su ./APP/src/vision_uart_tx.cyclo ./APP/src/vision_uart_tx.d ./APP/src/vision_uart_tx.o ./APP/src/vision_uart_tx.su

.PHONY: clean-APP-2f-src
