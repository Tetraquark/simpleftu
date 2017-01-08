################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Client.c \
../src/Common.c \
../src/Config.c \
../src/Crypto.c \
../src/Serializer.c \
../src/Server.c \
../src/main.c \
../src/md5.c 

OBJS += \
./src/Client.o \
./src/Common.o \
./src/Config.o \
./src/Crypto.o \
./src/Serializer.o \
./src/Server.o \
./src/main.o \
./src/md5.o 

C_DEPS += \
./src/Client.d \
./src/Common.d \
./src/Config.d \
./src/Crypto.d \
./src/Serializer.d \
./src/Server.d \
./src/main.d \
./src/md5.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


