CC      = arm-none-eabi-gcc
CPU     = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEV     = vendor/CMSIS/Device/ST/STM32G4xx
CFLAGS  = $(CPU) -DSTM32G474xx -Iinc -isystem vendor/CMSIS/Core/Include -isystem $(DEV)/Include -Wall -Wextra -Werror -g -ffunction-sections -fdata-sections
#C compiler settings 

CXX      = arm-none-eabi-g++
CXXFLAGS = $(filter-out -std=%,$(CFLAGS)) -std=c++17 -fno-exceptions -fno-rtti -fno-threadsafe-statics
#c++ compiler settings 

LDFLAGS = $(CPU) -T STM32G474RETX_FLASH.ld --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=build/main.map
#linker settings 

OBJS    = build/main.o build/system_stm32g4xx.o build/startup_stm32g474xx.o build/hello.o

build/main.elf: $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o build/main.elf

build/main.o: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

build/hello.o: src/hello.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c src/hello.cpp -o build/hello.o

build/system_stm32g4xx.o: $(DEV)/Source/system_stm32g4xx.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $(DEV)/Source/system_stm32g4xx.c -o build/system_stm32g4xx.o

build/startup_stm32g474xx.o: $(DEV)/Source/startup_stm32g474xx.s
	mkdir -p build
	$(CC) $(CPU) -c $(DEV)/Source/startup_stm32g474xx.s -o build/startup_stm32g474xx.o

clean:
	rm -rf build
