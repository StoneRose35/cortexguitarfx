
# **********************************
#
# STM32H7 Builds
#
# **********************************

PROJECT=ppfx_daisyseed
MAIN_VERSION=3
SUB_VERSION=0
MINUTES_SINCE_INCUBATION:=$(shell expr `date +%s` \/ 60 - `date -d "20220319" +%s` \/ 60)
FLASH_QSPI_SYNC_NUMBER:=$(shell awk 'BEGIN{srand();print int(rand()*(4294967296))};')
AVR_SYNC_NUMBER:=$(shell date -r mic_stomp_expansion_board/src/main.c.i +%s)
BUILD_DATE:=$(shell date +%Y-%m-%d -u)
BUILD_TIME:=$(shell date +%H:%M:%S -u)
MCU_BOARD=Daisy Seed
CC=arm-none-eabi-gcc
CPP=arm-none-eabi-g++
OBJCPY=arm-none-eabi-objcopy
ELF2UF2=./tools/elf2uf2
OPT=-Og
DEFINES=-DHARDWARE -DSTM32H750xx -DI2S_INPUT -DFLOAT_AUDIO 
CARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -std=gnu11 -Wall -Wpedantic -Wextra -I./Inc -I./Inc/gen
CPPARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -std=c++20 -Wall -Wpedantic -Wextra  -Wno-error=narrowing -I./Inc -I./Inc/gen
LARGS=-g -Xlinker -print-memory-usage -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -T./STM32H750IBKX_FLASH.ld -Xlinker -Map="./out/$(PROJECT).map" -Xlinker --gc-sections -static --specs="nano.specs" -Wl,--start-group -lstdc++ -lm -Wl,--end-group
#LARGS_QSPI=-g -nostdlib -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -T./STM32H750IBKX_FLASH.ld -Xlinker -Map="./out/$(PROJECT)_qspi.map" -Xlinker --gc-sections -static --specs="nano.specs" -Wl,--start-group -lc -lm -Wl,--end-group
#LARGS_BS2=-nostdlib -T ./bs2_default.ld -Xlinker -Map="./out/bs2_default.map"
CPYARGS=-Obinary  --remove-section=.qspi* --remove-section=.dtcm* 
CPYARGS_QSPIBIN=-Obinary  --only-section=.qspi* --only-section=.dtcm* 
DEBUGGER_UART=/dev/ttyACM0

export AVR_SYNC_NUMBER
all: out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION).dfu
	@rm out/*.o

STM32H750_OBJS := $(patsubst Src/stm32h750/%.c,out/%.o,$(wildcard Src/stm32h750/*.c))
STM32H750_OBJS_CPP := $(patsubst Src/stm32h750/%.cpp,out/%.o,$(wildcard Src/stm32h750/*.cpp))
COMMON_OBJS := $(patsubst Src/common/%.c,out/%.o,$(wildcard Src/common/*.c))
COMMON_OBJS_CPP := $(patsubst Src/common/%.cpp,out/%.o,$(wildcard Src/common/*.cpp))
AUDIO_OBJS := $(patsubst Src/common/audio/%.c,out/%.o,$(wildcard Src/common/audio/*.c))
MATH_OBJS := $(patsubst Src/common/math/%.c,out/%.o,$(wildcard Src/common/math/*.c))
AUDIO_FX_OBJS := $(patsubst Src/pipicofx/%.c,out/%.o,$(wildcard Src/pipicofx/*.c))
AUDIO_FX_OBJS_CPP := $(patsubst Src/pipicofx/%.cpp,out/%.o,$(wildcard Src/pipicofx/*.cpp))
AUDIO_FX_UI_OBJS_CPP := $(patsubst Src/pipicofx/ui/%.cpp,out/%.o,$(wildcard Src/pipicofx/ui/*.cpp))
GRAPHICS_OBJS := $(patsubst Src/common/graphics/%.c,out/%.o,$(wildcard Src/common/graphics/*.c))
APPS_OBJS := $(patsubst Src/apps/%.c,out/%.o,$(wildcard Src/apps/*.c))
SERVICES_OBJS := $(patsubst Src/services/%.c,out/%.o,$(wildcard Src/services/*.c))
USB_OBJS := $(patsubst Src/common/usb/%.c,out/%.o,$(wildcard Src/common/usb/*.c))
USB_OBJS_CPP := $(patsubst Src/common/usb/%.cpp,out/%.o,$(wildcard Src/common/usb/*.cpp))
ASSET_IMAGES := $(patsubst Assets/%.png,Inc/images/%.h,$(wildcard Assets/*.png))


all_stm32h750: $(STM32H750_OBJS) $(STM32H750_OBJS_CPP)
all_common: $(COMMON_OBJS)
all_common_cpp: $(COMMON_OBJS_CPP)
all_audio: $(AUDIO_OBJS) $(AUDIO_FX_OBJS) $(AUDIO_FX_UI_OBJS_CPP)
all_audio_cpp: $(AUDIO_FX_OBJS_CPP)
all_graphics: $(GRAPHICS_OBJS)
all_services: $(SERVICES_OBJS)
all_usb: $(USB_OBJS) $(USB_OBJS_CPP)
all_apps: $(APPS_OBJS)
all_images: $(ASSET_IMAGES)
all_math: $(MATH_OBJS)

clean_objs:
	@rm -f ./out/*
	@rm -f ./Inc/gen/*
	@rmdir ./out

clean: clean_objs


Inc/gen:
	mkdir -p ./Inc/gen

out: Inc/gen/versionDef.h
	mkdir -p ./out

# generate the startup file
out/stm32h750_startup.o: Startup/startup_stm32h750ibkx.S out 
	$(CC) $(CARGS) -c  $< -o ./out/stm32h750_startup.o

out/helpers.o: Src/stm32h750/helpers.s out	
	$(CC) $(CARGS) -c  $< -o ./out/helpers.o

# common libs
out/%.o: Src/common/%.c  Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# common c++ libs
out/%.o: Src/common/%.cpp Inc/gen/version.h out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# audio libs
out/%.o: Src/common/audio/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# math libs
out/%.o: Src/common/math/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx libs, c++
out/%.o: Src/pipicofx/%.cpp  out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# audio fx libs
out/%.o: Src/pipicofx/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx ui libs
out/%.o: Src/pipicofx/ui/%.cpp Inc/gen/version.h out all_images
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# graphics libs
out/%.o: Src/common/graphics/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# stm32h750-specific libs
out/%.o: Src/stm32h750/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# stm32h750-specific libs, c++
out/%.o: Src/stm32h750/%.cpp  out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# application layer
out/%.o: Src/apps/%.c  out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# services layer
out/%.o: Src/services/%.c out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# usb layer
out/%.o: Src/common/usb/%.c out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/common/usb/%.cpp out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# image assets
Inc/images/%.h: Assets/%.png out
	python3 ./tools/helper_scripts.py -convertBwImg $<

# qspi uploader tool
tools/qspi_uart_uploader:
	gcc -Og tools/qspi_uart_uploader.c -o tools/qspi_uart_uploader

tools/bins2dfu:
	gcc -O1 -I Inc -g tools/bins2dfu_src/bin2dfu.cpp -o tools/bins2dfu

tools/flash2dfu:
	gcc -O1 -I Inc -g tools/flash2dfu_src/flash2dfu.cpp -o tools/flash2dfu

# AVR Firmware
mic_stomp_expansion_board/mic_stomp.bin:
	$(MAKE) -C mic_stomp_expansion_board
Inc/gen/versionDef.h: Inc/gen
	@echo "#ifndef _PI_PICO_VERSION_DEF_H_\r\n#define _PI_PICO_VERSION_DEF_H_\r\n" > Inc/gen/versionDef.h 
	@echo "#define PI_PICO_FX_VNR \"V$(MAIN_VERSION).$(SUB_VERSION).$(MINUTES_SINCE_INCUBATION)\"" >>  Inc/gen/versionDef.h 
	@echo "#endif\r\n" >> Inc/gen/versionDef.h 

Inc/gen/version.h: Inc/gen Inc/gen/versionDef.h
#REV=`expr %REV% / 60`
	@echo "#ifndef _PI_PICO_VERSION_H_\r\n#define _PI_PICO_VERSION_H_\r\n" > Inc/gen/version.h 
	@echo "#define PI_PICO_FX_FULL_VERSION \"V$(MAIN_VERSION).$(SUB_VERSION).$(MINUTES_SINCE_INCUBATION) for $(MCU_BOARD) built $(BUILD_DATE)T$(BUILD_TIME)\"" >> Inc/gen/version.h 
	@echo "#define PI_PICO_FX_VERSION_NR \"V$(MAIN_VERSION).$(SUB_VERSION).$(MINUTES_SINCE_INCUBATION)\"" >> Inc/gen/version.h 
	@echo "#define PI_PICO_FX_MCU_BOARD \"$(MCU_BOARD)\"" >> Inc/gen/version.h 
	@echo "#define PI_PICO_FX_BUILD_DATE \"$(BUILD_DATE)\"" >> Inc/gen/version.h 
	@echo "#define PI_PICO_FX_BUILD_TIME \"$(BUILD_TIME)\"" >> Inc/gen/version.h 
	@echo "#ifdef SYNC_NUMBERS" >> Inc/gen/version.h 
	@echo "__attribute__ ((section (\".sync_number_flash\"))) uint32_t FLASH_SYNC_NUMBER=$(FLASH_QSPI_SYNC_NUMBER);" >> Inc/gen/version.h
	@echo "__attribute__ ((section (\".sync_number_qspi\"))) uint32_t QSPI_SYNC_NUMBER=$(FLASH_QSPI_SYNC_NUMBER);" >> Inc/gen/version.h
	@echo "#endif" >> Inc/gen/version.h
	@echo "#define FLASH_QSPI_SYNC_NUMBER $(FLASH_QSPI_SYNC_NUMBER)UL" >> Inc/gen/version.h
	@echo "#define AVR_SYNC_NUMBER $(AVR_SYNC_NUMBER)\r\n" >> Inc/gen/version.h
	@echo "#endif\r\n" >> Inc/gen/version.h 

# main linking and generating flashable content
out/$(PROJECT).elf: out/stm32h750_startup.o out/helpers.o all_stm32h750  all_common all_common_cpp all_audio all_audio_cpp all_graphics all_math all_usb mic_stomp_expansion_board/mic_stomp.bin
	$(CPP) $(LARGS) -o ./out/$(PROJECT).elf ./out/*.o 


out/$(PROJECT).bin: out/$(PROJECT).elf
	@$(OBJCPY) $(CPYARGS) ./out/$(PROJECT).elf ./out/$(PROJECT).bin
	

out/$(PROJECT)_qspi.bin: out/$(PROJECT).elf 
	@$(OBJCPY) $(CPYARGS_QSPIBIN) ./out/$(PROJECT).elf ./out/$(PROJECT)_qspi.bin

out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION).dfu: tools/bins2dfu tools/flash2dfu out/$(PROJECT).bin out/$(PROJECT)_qspi.bin
	tools/bins2dfu out/$(PROJECT).bin out/$(PROJECT)_qspi.bin -o out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION).dfu
	tools/flash2dfu out/$(PROJECT).bin -o out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION)_firstTime.dfu

program_qspi: out/$(PROJECT)_qspi.bin tools/qspi_uart_uploader
	tools/qspi_uart_uploader out/$(PROJECT)_qspi.bin $(DEBUGGER_UART)
	rm out/*.o

program_flash: out/$(PROJECT).bin
	st-flash --connect-under-reset write out/$(PROJECT).bin 0x8000000
	rm out/*.o

program_all: out/$(PROJECT).dfu out/$(PROJECT)_qspi.dfu tools/qspi_uart_uploader
	st-flash --connect-under-reset --reset write out/$(PROJECT).bin 0x8000000
	sleep 1
	tools/qspi_uart_uploader out/$(PROJECT)_qspi.bin $(DEBUGGER_UART)
	rm out/*.o

program_dfu: out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION).dfu
	dfu-util -D out/$(PROJECT)_$(MINUTES_SINCE_INCUBATION).dfu
	rm out/*.o


# *************************************************************
#
# Tests on x86
#
# *************************************************************

CC_TEST=gcc -I./Inc -Og -g3 -Wall -c -fmessage-length=0 -MMD -MP 


TEST_COMMON_OBJS := $(patsubst Src/common/%.c,testout/%.o,$(wildcard Src/common/*.c))
TEST_MOCK_OBJS := $(patsubst Src/mock/%.c,testout/%.o,$(wildcard Src/mock/*.c))
TEST_MAIN_OBJS := $(patsubst Tests/%.c,testout/%.o,$(wildcard Tests/*.c))

clean_tests: 
	@rm -rf ./testout/*
	
testout/$(PROJECT): $(TEST_COMMON_OBJS) $(TEST_MOCK_OBJS) $(TEST_MAIN_OBJS)
	gcc  -o ./testout/$(PROJECT) ./testout/*.o

tests: clean_tests testout/$(PROJECT)

testout/%.o: Src/common/%.c 
	$(CC_TEST) -o $@ -c $^

testout/%.o: Src/mock/%.c
	$(CC_TEST) -o $@ -c $^

testout/%.o: Tests/%.c
	$(CC_TEST) -o $@ -c $^

.PHONY: Inc/gen/version.h mic_stomp_expansion_board/mic_stomp.bin
