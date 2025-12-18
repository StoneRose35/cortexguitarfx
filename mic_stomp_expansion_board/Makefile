PROJECTNAME=mic_stomp
CC=avr-gcc
CARGS=-Wall -Og -DF_CPU=4000000UL -mmcu=atmega88p -Wl,-T./avr.ld
OBJCOPY=avr-objcopy
OCARGS=-j .text -j .data -O ihex 
OCARGS_BIN=-j .text -j .data -j .firmwaresig -O binary 


OBJS := $(patsubst src/%.c.i,out/%.o,$(wildcard src/*.c.i))

all: $(PROJECTNAME).bin size

out:
	mkdir out

src/main.c: src/main.c.i
	@rm -f src/main.c
	@awk -v magicnr="${AVR_SYNC_NUMBER}" 'BEGIN{srand();}{gsub("<FIRMWARE_VERSION>",magicnr,$$0); print $$0}' src/main.c.i > src/main.c

out/%.o: src/%.c out
	$(CC) $(CARGS) -c $< -o $@

$(PROJECTNAME).elf: $(OBJS)
	$(CC) $(CARGS) -o $(PROJECTNAME).elf out/*.o

size: $(PROJECTNAME).elf
	avr-size --mcu=atmega88 -C $(PROJECTNAME).elf

$(PROJECTNAME).hex: $(PROJECTNAME).elf
	$(OBJCOPY) $(OCARGS) $(PROJECTNAME).elf $(PROJECTNAME).hex

$(PROJECTNAME).bin: $(PROJECTNAME).elf
	$(OBJCOPY) $(OCARGS_BIN) $(PROJECTNAME).elf $(PROJECTNAME).bin

clean:
	rm -rf out/*
	rm -rf $(PROJECTNAME).*

.PHONY: all clean src/main.c

program: clean all
	avrdude -p m88p -c dragon_isp -P usb -U flash:w:$(PROJECTNAME).hex
