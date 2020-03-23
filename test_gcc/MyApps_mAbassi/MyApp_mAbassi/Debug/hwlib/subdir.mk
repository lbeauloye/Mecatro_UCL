################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hwlib/alt_generalpurpose_io.c 

OBJS += \
./hwlib/alt_generalpurpose_io.o 

C_DEPS += \
./hwlib/alt_generalpurpose_io.d 


# Each subdirectory must supply rules for building sources it contributes
hwlib/%.o: ../hwlib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	arm-altera-eabi-g++ -O0 -g -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


