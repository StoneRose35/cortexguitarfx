
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
DEFINES=-DDEBUG -DSTM32 -DNUCLEO_F446ZE -DSTM32F4 -DSTM32F446ZETx -DI2S_INPUT -DFLOAT_AUDIO
CARGS=-fno-builtin -g $(DEFINES) -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -std=gnu11 -Wall -I./Inc/RpiPico -I./Inc -I./Inc/gen
LARGS=-g -nostdlib -Xlinker -print-memory-usage -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -T./STM32F446ZETX_FLASH.ld -Xlinker -Map="./out/$(PROJECT).map" -Xlinker --gc-sections -static --specs="nano.specs" -Wl,--start-group -lc -lm -Wl,--end-group
#LARGS_BS2=-nostdlib -T ./bs2_default.ld -Xlinker -Map="./out/bs2_default.map"
CPYARGS=-Obinary

all: $(PROJECT).bin

#RP2040_OBJS := $(patsubst Src/rp2040/%.c,out/%.o,$(wildcard Src/rp2040/*.c))
#RP2040_OBJS_ASM := $(patsubst Src/rp2040/%.S,out/%.o,$(wildcard Src/rp2040/*.S))
STM32F446_OBJS := $(patsubst Src/stm32f446/%.c,out/%.o,$(wildcard Src/stm32f446/*.c))
COMMON_OBJS := $(patsubst Src/common/%.c,out/%.o,$(wildcard Src/common/*.c))
AUDIO_OBJS := $(patsubst Src/common/audio/%.c,out/%.o,$(wildcard Src/common/audio/*.c))
AUDIO_FX_OBJS := $(patsubst Src/pipicofx/%.c,out/%.o,$(wildcard Src/pipicofx/*.c))
GRAPHICS_OBJS := $(patsubst Src/common/graphics/%.c,out/%.o,$(wildcard Src/common/graphics/*.c))
NEOPIXEL_OBJS := $(patsubst Src/common/neopixel/%.c,out/%.o,$(wildcard Src/common/neopixel/*.c))
SDCARD_OBJS := $(patsubst Src/common/sdcard/%.c,out/%.o,$(wildcard Src/common/sdcard/*.c))
APPS_OBJS := $(patsubst Src/apps/%.c,out/%.o,$(wildcard Src/apps/*.c))
SERVICES_OBJS := $(patsubst Src/services/%.c,out/%.o,$(wildcard Src/services/*.c))
ASSET_IMAGES := $(patsubst Assets/%.png,Inc/images/%.h,$(wildcard Assets/*.png))


#all_rp2040: $(RP2040_OBJS) $(RP2040_OBJS_ASM)
all_stm32f446: $(STM32F446_OBJS)
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


Inc/gen:
	mkdir ./Inc/gen

Inc/gen/compilationInfo.h: Inc/gen
	echo "const char * COMPILATIONINFO=\"compiled on " `date` " using \\\\r\\\\n " `arm-none-eabi-gcc --version | sed -z 's/\n/\\\\\\\\r\\\\\\\\n/g'` "\";" > Inc/gen/compilationInfo.h

# generate the startup file
out/stm32f446_startup.o: Startup/startup_stm32f446zetx.s
	$(CC) $(CARGS) -c  $< -o ./out/stm32f446_startup.o

out/helpers.o: Src/stm32f446/helpers.s	
	$(CC) $(CARGS) -c  $< -o ./out/helpers.o

# stm32f446-specific libraries
out/%.o: Src/stm32f446/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# common libs
out/%.o: Src/common/%.c $(ASSET_IMAGES) Inc/gen/compilationInfo.h
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio libs
out/%.o: Src/common/audio/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# audio fx libs
out/%.o: Src/pipicofx/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# graphics libs
out/%.o: Src/common/graphics/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# sdcard libs
out/%.o: Src/common/neopixel/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# neopixel libs
out/%.o: Src/common/sdcard/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific libs
out/%.o: Src/rp2040/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# rp2040 specific assembly libs
out/%.o: Src/rp2040/%.S $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# application layer
out/%.o: Src/apps/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# services layer
out/%.o: Src/services/%.c $(ASSET_IMAGES) 
	$(CC) $(CARGS) $(OPT) -c $< -o $@

# image assets
Inc/images/%.h: Assets/%.png
	./tools/helper_scripts.py -convertImg $^

# main linking and generating flashable content
$(PROJECT).elf: out/stm32f446_startup.o out/helpers.o all_stm32f446 all_common all_apps all_audio all_graphics  $(ASSET_IMAGES)
	$(CC) $(LARGS) -o ./out/$(PROJECT).elf ./out/*.o 

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCPY) $(CPYARGS) ./out/$(PROJECT).elf ./out/$(PROJECT).bin


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

.PHONY: Inc/gen/compilationInfo.h
