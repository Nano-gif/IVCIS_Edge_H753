################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../X-CUBE-AI/App/vehicle_detector.c \
../X-CUBE-AI/App/vehicle_detector_data.c \
../X-CUBE-AI/App/vehicle_detector_data_params.c 

OBJS += \
./X-CUBE-AI/App/vehicle_detector.o \
./X-CUBE-AI/App/vehicle_detector_data.o \
./X-CUBE-AI/App/vehicle_detector_data_params.o 

C_DEPS += \
./X-CUBE-AI/App/vehicle_detector.d \
./X-CUBE-AI/App/vehicle_detector_data.d \
./X-CUBE-AI/App/vehicle_detector_data_params.d 


# Each subdirectory must supply rules for building sources it contributes
X-CUBE-AI/App/%.o X-CUBE-AI/App/%.su X-CUBE-AI/App/%.cyclo: ../X-CUBE-AI/App/%.c X-CUBE-AI/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I"E:/STM32/OV/Drivers/BSP" -I"E:/STM32/OV/Drivers/BSP/ov5640" -I"E:/STM32/OV/APP" -I"E:/STM32/OV/APP/Inc" -I"E:/STM32/OV/APP/src" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Utilities/JPEG -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-X-2d-CUBE-2d-AI-2f-App

clean-X-2d-CUBE-2d-AI-2f-App:
	-$(RM) ./X-CUBE-AI/App/vehicle_detector.cyclo ./X-CUBE-AI/App/vehicle_detector.d ./X-CUBE-AI/App/vehicle_detector.o ./X-CUBE-AI/App/vehicle_detector.su ./X-CUBE-AI/App/vehicle_detector_data.cyclo ./X-CUBE-AI/App/vehicle_detector_data.d ./X-CUBE-AI/App/vehicle_detector_data.o ./X-CUBE-AI/App/vehicle_detector_data.su ./X-CUBE-AI/App/vehicle_detector_data_params.cyclo ./X-CUBE-AI/App/vehicle_detector_data_params.d ./X-CUBE-AI/App/vehicle_detector_data_params.o ./X-CUBE-AI/App/vehicle_detector_data_params.su

.PHONY: clean-X-2d-CUBE-2d-AI-2f-App

