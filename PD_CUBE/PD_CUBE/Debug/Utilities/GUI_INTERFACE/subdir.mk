################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/GUI_INTERFACE/bsp_gui.c \
../Utilities/GUI_INTERFACE/data_struct_tlv.c \
../Utilities/GUI_INTERFACE/gui_api.c 

OBJS += \
./Utilities/GUI_INTERFACE/bsp_gui.o \
./Utilities/GUI_INTERFACE/data_struct_tlv.o \
./Utilities/GUI_INTERFACE/gui_api.o 

C_DEPS += \
./Utilities/GUI_INTERFACE/bsp_gui.d \
./Utilities/GUI_INTERFACE/data_struct_tlv.d \
./Utilities/GUI_INTERFACE/gui_api.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/GUI_INTERFACE/%.o Utilities/GUI_INTERFACE/%.su Utilities/GUI_INTERFACE/%.cyclo: ../Utilities/GUI_INTERFACE/%.c Utilities/GUI_INTERFACE/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -DUSE_FULL_LL_DRIVER -DUSBPD_PORT_COUNT=1 -D_RTOS -D_SNK -D_TRACE -D_GUI_INTERFACE -DUSBPDCORE_LIB_PD3_FULL -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../TCPP/App -I../TCPP/Target -I../TCPP -I../USBPD/App -I../USBPD/Target -I../Utilities/GUI_INTERFACE -I../Utilities/TRACER_EMB -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Middlewares/ST/STM32_USBPD_Library/Core/inc -I../Middlewares/ST/STM32_USBPD_Library/Devices/STM32G0XX/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Utilities-2f-GUI_INTERFACE

clean-Utilities-2f-GUI_INTERFACE:
	-$(RM) ./Utilities/GUI_INTERFACE/bsp_gui.cyclo ./Utilities/GUI_INTERFACE/bsp_gui.d ./Utilities/GUI_INTERFACE/bsp_gui.o ./Utilities/GUI_INTERFACE/bsp_gui.su ./Utilities/GUI_INTERFACE/data_struct_tlv.cyclo ./Utilities/GUI_INTERFACE/data_struct_tlv.d ./Utilities/GUI_INTERFACE/data_struct_tlv.o ./Utilities/GUI_INTERFACE/data_struct_tlv.su ./Utilities/GUI_INTERFACE/gui_api.cyclo ./Utilities/GUI_INTERFACE/gui_api.d ./Utilities/GUI_INTERFACE/gui_api.o ./Utilities/GUI_INTERFACE/gui_api.su

.PHONY: clean-Utilities-2f-GUI_INTERFACE

