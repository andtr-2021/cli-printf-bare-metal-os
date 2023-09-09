#--------------------------------------Makefile-------------------------------------
CFILES = $(wildcard ./src/*.c)
OFILES = $(CFILES:./src/%.c=./object/%.o)

GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib
LDFLAGS = -nostdlib 

all: clean kernel8.img run



#Create UART0 or UART1 preprocessing define
#Syntax: add -DVAR to compiler's flag will create #define VAR
GCCFLAGS += -DUART$(UARTTYPE)

./object/boot.o: ./src/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

./object/uart.o: 
	aarch64-none-elf-gcc $(GCCFLAGS) -c ./src/uart.c -o $@

./object/%.o: ./src/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

kernel8.img: ./object/boot.o ./object/uart.o $(OFILES)
	aarch64-none-elf-ld $(LDFLAGS) ./object/boot.o $(OFILES)  -T ./src/link.ld -o ./object/kernel8.elf
	aarch64-none-elf-objcopy -O binary ./object/kernel8.elf kernel8.img

clean:
	del *.img .\object\*.elf .\object\*.o

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio




