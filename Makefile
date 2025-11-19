# **********************************
#
# RP2040 Builds
#
# **********************************

PROJECT=freestompfx
MAIN_VERSION=0
SUB_VERSION=7
MINUTES_SINCE_INCUBATION:=$(shell expr `date +%s` \/ 60 - `date -d "20220319" +%s` \/ 60)
FLASH_QSPI_SYNC_NUMBER:=$(shell awk 'BEGIN{srand();print int(rand()*(4294967296))};')
AVR_SYNC_NUMBER:=$(shell date -r mic_stomp_expansion_board/src/main.c.i +%s)
BUILD_DATE:=$(shell date +%Y-%m-%d -u)
BUILD_TIME:=$(shell date +%H:%M:%S -u)
MCU_BOARD=Raspberry Pi Pico
CC=arm-none-eabi-gcc
CPP=arm-none-eabi-g++
OBJCPY=arm-none-eabi-objcopy
ELF2UF2=./tools/elf2uf2
OPT=-Og
PAD_CKECKSUM=./tools/pad_checksum
DEFINES=-DRP2040_FEATHER -DI2S_INPUT 
CARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -std=gnu11 -Wall -I./Inc/RpiPico -I./Inc -I./Inc/gen -I./Src/tusb
CPPARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -Wall -Wno-error=narrowing -I./Inc/RpiPico -I./Inc -I./Inc/gen -I./Src/tusb
LARGS=-g -Xlinker -print-memory-usage -mcpu=cortex-m0plus -mthumb -Wl,--wrap=__aeabi_idiv -T./rp2040_feather.ld -Xlinker -Map="./out/$(PROJECT).map" -Xlinker --gc-sections -static --specs="nano.specs" -Wl,--start-group -lm -lstdc++ -Wl,--end-group
LARGS_BS2=-nostdlib -T ./bs2_default.ld -Xlinker -Map="./out/bs2_default.map"
CPYARGS=-Obinary
BOOTLOADER=bs2_fast_qspi2

export AVR_SYNC_NUMBER
all: bs2_code_size $(PROJECT).uf2 

RP2040_OBJS := $(patsubst Src/rp2040/%.c,out/%.o,$(wildcard Src/rp2040/*.c))
RP2040_OBJS_CPP := $(patsubst Src/rp2040/%.cpp,out/%.o,$(wildcard Src/rp2040/*.cpp))
RP2040_OBJS_ASM := $(patsubst Src/rp2040/%.S,out/%.o,$(wildcard Src/rp2040/*.S))
COMMON_OBJS := $(patsubst Src/common/%.c,out/%.o,$(wildcard Src/common/*.c))
COMMON_OBJS_CPP := $(patsubst Src/common/%.cpp,out/%.o,$(wildcard Src/common/*.cpp))
AUDIO_OBJS := $(patsubst Src/common/audio/%.c,out/%.o,$(wildcard Src/common/audio/*.c))
MATH_OBJS := $(patsubst Src/common/math/%.c,out/%.o,$(wildcard Src/common/math/*.c))
AUDIO_FX_OBJS := $(patsubst Src/pipicofx/%.c,out/%.o,$(wildcard Src/pipicofx/*.c))
AUDIO_FX_OBJS_CPP := $(patsubst Src/pipicofx/%.cpp,out/%.o,$(wildcard Src/pipicofx/*.cpp))
AUDIO_FX_UI_OBJS_CPP := $(patsubst Src/pipicofx/ui/%.cpp,out/%.o,$(wildcard Src/pipicofx/ui/*.cpp))
GRAPHICS_OBJS := $(patsubst Src/common/graphics/%.c,out/%.o,$(wildcard Src/common/graphics/*.c))
NEOPIXEL_OBJS := $(patsubst Src/common/neopixel/%.c,out/%.o,$(wildcard Src/common/neopixel/*.c))
SDCARD_OBJS := $(patsubst Src/common/sdcard/%.c,out/%.o,$(wildcard Src/common/sdcard/*.c))
APPS_OBJS := $(patsubst Src/apps/%.c,out/%.o,$(wildcard Src/apps/*.c))
SERVICES_OBJS := $(patsubst Src/services/%.c,out/%.o,$(wildcard Src/services/*.c))
ASSET_IMAGES := $(patsubst Assets/%.png,Inc/images/%.h,$(wildcard Assets/*.png))


all_rp2040: $(RP2040_OBJS) $(RP2040_OBJS_ASM) $(RP2040_OBJS_CPP)
all_common: $(COMMON_OBJS)
all_common_cpp: $(COMMON_OBJS_CPP)
all_audio: $(AUDIO_OBJS) $(AUDIO_FX_OBJS) $(AUDIO_FX_UI_OBJS_CPP)
all_audio_cpp: $(AUDIO_FX_OBJS_CPP)
all_math: $(MATH_OBJS)
all_graphics: $(GRAPHICS_OBJS)
all_neopixel: $(NEOPIXEL_OBJS)
all_sdcard: $(SDCARD_OBJS)
all_apps: $(APPS_OBJS)
all_services: $(SERVICES_OBJS)
all_images: $(ASSET_IMAGES)

clean_objs:
	@rm -f ./out/*
	@rm -f ./Inc/gen/*

clean: clean_objs
	@rm -f ./tools/elf2uf2
	@rm -f ./tools/pioasm

# compile tools (elf2uf2)
tools/elf2uf2:
	g++ ./tools/elf2uf2_src/main.cpp -o ./tools/elf2uf2

PIOASM_SRC_DIR := tools/pioasm_src/
PIOASM_GEN_DIR := $(PIOASM_SRC_DIR)gen/

tools/pioasm_src/gen:
	mkdir ./tools/pioasm_src/gen

Inc/gen:
	mkdir ./Inc/gen

out:
	mkdir ./out

tools/pioasm_src/gen/lexer.cpp: tools/pioasm_src/gen
	flex -o ./tools/pioasm_src/gen/lexer.cpp ./tools/pioasm_src/lexer.ll 

tools/pioasm_src/gen/parser.cpp: tools/pioasm_src/gen tools/pioasm_src/gen/lexer.cpp
	bison --defines=./tools/pioasm_src/gen/parser.hpp -o ./tools/pioasm_src/gen/parser.cpp ./tools/pioasm_src/parser.yy

tools/pioasm: tools/pioasm_src/gen/parser.cpp tools/pioasm_src/gen/lexer.cpp
	g++ -DYY_NO_UNISTD_H -Itools/pioasm_src -Itools/pioasm_src/gen -Wno-psabi -o ./tools/pioasm ./tools/pioasm_src/*.cpp ./tools/pioasm_src/gen/*.cpp 


# boot stage code variants for the rp2040 feather and the rp2040 itsybitsy
bs2_std.o: out
	$(CC) $(CARGS) $(OPT) -c ./Startup/pico_bs2_std.S -o ./out/bs2_std.o

bs2_dspi.o: out
	$(CC) $(CARGS) $(OPT) -c ./Startup/pico_bs2_dspi.S -o ./out/bs2_dspi.o

bs2_fast_qspi.o: out
	$(CC) $(CARGS) $(OPT) -c ./Startup/pico_bs2_fast_qspi.S -o ./out/bs2_fast_qspi.o

bs2_fast_qspi2.o: out
	$(CC) $(CARGS) $(OPT) -c ./Startup/pico_bs2_fast_qspi2.S -o ./out/bs2_fast_qspi2.o


# generating the boot stage2 assembly file
# via a full roundtrip Assembly -> .o (declared above) -> .elf -> .bin -> Assembly (data declaration with crc32 checksum) 
bs2_code.elf: $(BOOTLOADER).o out
	$(CC) $(LARGS_BS2) -o ./out/bs2_code.elf ./out/$(BOOTLOADER).o

bs2_code.bin: bs2_code.elf out
	$(OBJCPY) $(CPYARGS) ./out/bs2_code.elf ./out/bs2_code.bin

bs2_code_size: bs2_code.bin
	@echo '**********************************'
	@echo '* Boot Stage 2 Code is' `ls -l ./out/bs2_code.bin | cut -d ' ' -f5` 'bytes' 
	@echo '**********************************'

bootstage2.S: bs2_code.bin out
	$(PAD_CKECKSUM) -s 0xffffffff ./out/bs2_code.bin ./out/bootstage2.S

# rp2040 feather startup stage
pico_startup2.o: out
	$(CC) $(CARGS) $(OPT) -c ./Startup/pico_startup2.S -o ./out/pico_startup2.o 

bootstage2.o: bootstage2.S
	$(CC) $(CARGS) $(OPT) -c ./out/bootstage2.S -o ./out/bootstage2.o 

# common libs
out/%.o: Src/common/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# common c++ libs
out/%.o: Src/common/%.cpp $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# audio libs
out/%.o: Src/common/audio/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# math libs
out/%.o: Src/common/math/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx libs
out/%.o: Src/pipicofx/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx libs, c++
out/%.o: Src/pipicofx/%.cpp $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# audio fx ui libs
out/%.o: Src/pipicofx/ui/%.cpp $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# graphics libs
out/%.o: Src/common/graphics/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# sdcard libs
out/%.o: Src/common/neopixel/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# neopixel libs
out/%.o: Src/common/sdcard/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific libs
out/%.o: Src/rp2040/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific libs, c++
out/%.o: Src/rp2040/%.cpp $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CPP) $(CPPARGS) $(OPT) -c $< -o $@

# rp2040 specific assembly libs
out/%.o: Src/rp2040/%.S $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# application layer
out/%.o: Src/apps/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# services layer
out/%.o: Src/services/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h Inc/gen/version.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# image assets
Inc/images/%.h: Assets/%.png
	python3 ./tools/helper_scripts.py -convertBwImg $<


# pio assembler
Inc/gen/pio0_pio.h: Inc/gen tools/pioasm
	./tools/pioasm -o c-sdk ./Src/rp2040/pio0.pio ./Inc/gen/pio0_pio.h

Inc/gen/version.h: Inc/gen
#REV=`expr %REV% / 60`
	@echo "#ifndef _PI_PICO_VERSION_H_\r\n#define _PI_PICO_VERSION_H_\r\n" > Inc/gen/version.h 
	@echo "const char PI_PICO_FX_FULL_VERSION[]=\"V$(MAIN_VERSION).$(SUB_VERSION).$(MINUTES_SINCE_INCUBATION) for $(MCU_BOARD) built $(BUILD_DATE)T$(BUILD_TIME)\";\r\n" >> Inc/gen/version.h 
	@echo "const char PI_PICO_FX_VERSION_NR[]=\"V$(MAIN_VERSION).$(SUB_VERSION).$(MINUTES_SINCE_INCUBATION)\";\r\n" >> Inc/gen/version.h 
	@echo "const char PI_PICO_FX_MCU_BOARD[]=\"$(MCU_BOARD)\";\r\n" >> Inc/gen/version.h 
	@echo "const char PI_PICO_FX_BUILD_DATE[]=\"$(BUILD_DATE)\";\r\n" >> Inc/gen/version.h 
	@echo "const char PI_PICO_FX_BUILD_TIME[]=\"$(BUILD_TIME)\";\r\n" >> Inc/gen/version.h 
	@echo "const uint32_t FLASH_SYNC_NUMBER=$(FLASH_QSPI_SYNC_NUMBER);\r\n" >> Inc/gen/version.h
	@echo "const uint32_t AVR_SYNC_NUMBER=$(AVR_SYNC_NUMBER);\r\n" >> Inc/gen/version.h
	@echo "#endif\r\n" >> Inc/gen/version.h 

# AVR Firmware
mic_stomp_expansion_board/mic_stomp.bin:
	$(MAKE) -C mic_stomp_expansion_board


# main linking and generating flashable content
$(PROJECT).elf: bootstage2.o pico_startup2.o all_rp2040 all_common all_common_cpp  all_audio all_audio_cpp all_graphics all_math $(ASSET_IMAGES) mic_stomp_expansion_board/mic_stomp.bin
	$(CPP) $(LARGS) -o ./out/$(PROJECT).elf ./out/*.o 


$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCPY) $(CPYARGS) ./out/$(PROJECT).elf ./out/$(PROJECT).bin

$(PROJECT).uf2: tools/elf2uf2 $(PROJECT).elf 
	$(ELF2UF2) ./out/$(PROJECT).elf ./out/$(PROJECT).uf2
	@rm -rf ./out/*.o

.PHONY: mic_stomp_expansion_board/mic_stomp.bin


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
