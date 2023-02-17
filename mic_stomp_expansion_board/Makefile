PROJECTNAME=mic_stomp
CC=avr-gcc
CARGS=-Wall -Os -DF_CPU=1000000UL -mmcu=atmega88
OBJCOPY=avr-objcopy
OCARGS=-j .text -j .data -O ihex 


OBJS := $(patsubst src/%.c,out/%.o,$(wildcard src/*.c))

out:
	mkdir out

out/%.o: src/%.c
	$(CC) $(CARGS) -c $< -o $@

$(PROJECTNAME).elf: $(OBJS)
	$(CC) $(CARGS) -o $(PROJECTNAME).elf out/*.o

size: $(PROJECTNAME).elf
	avr-size --mcu=atmega88 -C $(PROJECTNAME).elf

$(PROJECTNAME).hex: $(PROJECTNAME).elf
	$(OBJCOPY) $(OCARGS) $(PROJECTNAME).elf $(PROJECTNAME).hex

all: $(PROJECTNAME).hex size

clean:
	rm -rf out/*
	rm $(PROJECTNAME).*
	
program:
	avrdude -p m88 -c dragon_isp -P usb -U flash:w:$(PROJECTNAME).hex
