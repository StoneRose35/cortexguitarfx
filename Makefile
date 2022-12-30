
# **********************************
#
# RP2040 Builds
#
# **********************************

PROJECT=microsys_audio
CC=arm-none-eabi-gcc
OBJCPY=arm-none-eabi-objcopy
ELF2UF2=./tools/elf2uf2
OPT=-Og
PAD_CKECKSUM=./tools/pad_checksum
DEFINES=-DRP2040_FEATHER -DI2S_INPUT -DCS4270 
CARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -std=gnu11 -Wall -I./Inc/RpiPico -I./Inc -I./Inc/gen -I./Src/tusb
LARGS=-g -Xlinker -print-memory-usage -mcpu=cortex-m0plus -mthumb -T./rp2040_feather.ld -Xlinker -Map="./out/$(PROJECT).map" -Xlinker --gc-sections -static --specs="nano.specs" -Wl,--start-group -lc -lm -Wl,--end-group
LARGS_BS2=-nostdlib -T ./bs2_default.ld -Xlinker -Map="./out/bs2_default.map"
CPYARGS=-Obinary
BOOTLOADER=bs2_fast_qspi2

all: bs2_code_size $(PROJECT).uf2 

RP2040_OBJS := $(patsubst Src/rp2040/%.c,out/%.o,$(wildcard Src/rp2040/*.c))
RP2040_OBJS_ASM := $(patsubst Src/rp2040/%.S,out/%.o,$(wildcard Src/rp2040/*.S))
COMMON_OBJS := $(patsubst Src/common/%.c,out/%.o,$(wildcard Src/common/*.c))
AUDIO_OBJS := $(patsubst Src/common/audio/%.c,out/%.o,$(wildcard Src/common/audio/*.c))
AUDIO_FX_OBJS := $(patsubst Src/pipicofx/%.c,out/%.o,$(wildcard Src/pipicofx/*.c))
GRAPHICS_OBJS := $(patsubst Src/common/graphics/%.c,out/%.o,$(wildcard Src/common/graphics/*.c))
NEOPIXEL_OBJS := $(patsubst Src/common/neopixel/%.c,out/%.o,$(wildcard Src/common/neopixel/*.c))
SDCARD_OBJS := $(patsubst Src/common/sdcard/%.c,out/%.o,$(wildcard Src/common/sdcard/*.c))
APPS_OBJS := $(patsubst Src/apps/%.c,out/%.o,$(wildcard Src/apps/*.c))
SERVICES_OBJS := $(patsubst Src/services/%.c,out/%.o,$(wildcard Src/services/*.c))
ASSET_IMAGES := $(patsubst Assets/%.png,Inc/images/%.h,$(wildcard Assets/*.png))





TUSB_SRC_C += \
	Src/tusb/tusb.c \
	tusb/common/tusb_fifo.c \
	tusb/device/usbd.c \
	tusb/device/usbd_control.c \
	tusb/class/audio/audio_device.c \
	tusb/class/cdc/cdc_device.c \
	tusb/class/dfu/dfu_device.c \
	tusb/class/dfu/dfu_rt_device.c \
	tusb/class/hid/hid_device.c \
	tusb/class/midi/midi_device.c \
	tusb/class/msc/msc_device.c \
	tusb/class/net/ecm_rndis_device.c \
	tusb/class/net/ncm_device.c \
	tusb/class/usbtmc/usbtmc_device.c \
	tusb/class/video/video_device.c \
	tusb/class/vendor/vendor_device.c \
	tusb/portable/raspberrypi/rp2040/dcd_rp2040.c \
	tusb/portable/raspberrypi/rp2040/hcd_rp2040.c \
	tusb/portable/raspberrypi/rp2040/rp2040_usb.c \


TUSB_FILES_C += \
	usb_descriptors.c \
	cdc_msc.c \
	tusb.c \
	tusb_fifo.c \
	usbd.c \
	usbd_control.c \
	audio_device.c \
	cdc_device.c \
	dfu_device.c \
	dfu_rt_device.c \
	hid_device.c \
	midi_device.c \
	msc_device.c \
	ecm_rndis_device.c \
	ncm_device.c \
	usbtmc_device.c \
	video_device.c \
	vendor_device.c \
	dcd_rp2040.c \
	hcd_rp2040.c \
	rp2040_usb.c \

TUSB_OBJS += $(addprefix out/, $(TUSB_FILES_C:.c=.o))

echotusb:
	@echo $(TUSB_OBJS)

out/%.o: Src/tusb/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/common/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/device/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/audio/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/cdc/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/dfu/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/hid/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/midi/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/msc/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/net/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/usbtmc/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/video/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/class/vendor/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

out/%.o: Src/tusb/portable/raspberrypi/rp2040/%.c
	$(CC) $(CARGS) $(OPT) -c $< -o $@

all_usb: $(TUSB_OBJS)
all_rp2040: $(RP2040_OBJS) $(RP2040_OBJS_ASM)
all_common: $(COMMON_OBJS)
all_audio: $(AUDIO_OBJS) $(AUDIO_FX_OBJS)
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
out/%.o: Src/common/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio libs
out/%.o: Src/common/audio/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx libs
out/%.o: Src/pipicofx/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# graphics libs
out/%.o: Src/common/graphics/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# sdcard libs
out/%.o: Src/common/neopixel/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# neopixel libs
out/%.o: Src/common/sdcard/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific libs
out/%.o: Src/rp2040/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific assembly libs
out/%.o: Src/rp2040/%.S $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# application layer
out/%.o: Src/apps/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# services layer
out/%.o: Src/services/%.c $(ASSET_IMAGES) Inc/gen/pio0_pio.h out
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# image assets
Inc/images/%.h: Assets/%.png
	./tools/helper_scripts.py -convertImg $^


Src/rp2040/neopixelDriver.c: Inc/gen/pio0_pio.h

Src/rp2040/simple_neopixel.c: Inc/gen/pio0_pio.h

Src/rp2040/simple_timertest.c: Inc/gen/pio0_pio.h

Src/rp2040/ds18b20.c: Inc/gen/pio0_pio.h

# pio assembler
Inc/gen/pio0_pio.h: Inc/gen tools/pioasm
	./tools/pioasm -o c-sdk ./Src/rp2040/pio0.pio ./Inc/gen/pio0_pio.h


# main linking and generating flashable content
$(PROJECT).elf: bootstage2.o pico_startup2.o all_rp2040 all_common  all_audio all_graphics  $(ASSET_IMAGES)
	$(CC) $(LARGS) -o ./out/$(PROJECT).elf ./out/*.o 
#~/pico/pico-libs/rp2_common/pico_stdio/stdio.c.obj ~/pico/pico-libs/common/pico_sync/mutex.c.obj ~/pico/pico-libs/rp2_common/hardware_timer/timer.c.obj ~/pico/pico-libs/common/pico_time/time.c.obj

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCPY) $(CPYARGS) ./out/$(PROJECT).elf ./out/$(PROJECT).bin

$(PROJECT).uf2: tools/elf2uf2 $(PROJECT).elf 
	$(ELF2UF2) ./out/$(PROJECT).elf ./out/$(PROJECT).uf2

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
