################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Main_mAbassi.cpp \
../src/MyApp_mAbassi.cpp \
../src/test.cpp 

C_SRCS += \
../src/Main_mAbassi.c \
../src/MyApp_mAbassi.c 

OBJS += \
./src/Main_mAbassi.o \
./src/MyApp_mAbassi.o \
./src/test.o 

C_DEPS += \
./src/Main_mAbassi.d \
./src/MyApp_mAbassi.d 

CPP_DEPS += \
./src/Main_mAbassi.d \
./src/MyApp_mAbassi.d \
./src/test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	arm-altera-eabi-g++ -O0 -g -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	arm-altera-eabi-g++ -O0 -g -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


