PROJECTNAME=mic_stomp
CC=avr-gcc
CARGS=-Wall -O0 -DF_CPU=1000000UL -mmcu=atmega88
OBJCOPY=avr-objcopy
OCARGS=-j .text -j .data -O ihex 


OBJS := $(patsubst src/%.c,out/%.o,$(wildcard src/*.c))

all: $(PROJECTNAME).hex size

out:
	mkdir out

out/%.o: src/%.c out
	$(CC) $(CARGS) -c $< -o $@

$(PROJECTNAME).elf: $(OBJS)
	$(CC) $(CARGS) -o $(PROJECTNAME).elf out/*.o

size: $(PROJECTNAME).elf
	avr-size --mcu=atmega88p -C $(PROJECTNAME).elf

$(PROJECTNAME).hex: $(PROJECTNAME).elf
	$(OBJCOPY) $(OCARGS) $(PROJECTNAME).elf $(PROJECTNAME).hex


clean:
	rm -rf out/*
	rm $(PROJECTNAME).*

.PHONY: all clean

program:
	avrdude -p m88p -c dragon_isp -P usb -U flash:w:$(PROJECTNAME).hex
