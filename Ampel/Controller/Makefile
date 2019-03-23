#PROGRAMMER = avrisp2
PROGRAMMER = usbtiny
CPU_GCC = attiny85
CPU_DUDE = t85

F_CPU = 8000000


CDEFS = -DF_CPU=$(F_CPU)
CFLAGS = -mmcu=$(CPU_GCC) $(CDEFS) -Wall -Os

PROGRAM = firmware

.phony: clean

all: $(PROGRAM).hex

program: $(PROGRAM).hex
	avrdude -c $(PROGRAMMER) -P usb -p $(CPU_DUDE) -U flash:w:$(PROGRAM).hex

fuse:
	avrdude -c $(PROGRAMMER) -P usb -p $(CPU_DUDE) -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

clean:
	rm *.o *.elf *.hex

$(PROGRAM).hex: $(PROGRAM).c
	avr-gcc $(CFLAGS) -c usitwislave.c -o usitwislave.o
	avr-gcc $(CFLAGS) -c $(PROGRAM).c  -o $(PROGRAM).o
	avr-gcc $(CFLAGS) $(PROGRAM).o usitwislave.o -o $(PROGRAM).elf
	avr-objcopy -R .eeprom -O ihex $(PROGRAM).elf $(PROGRAM).hex
