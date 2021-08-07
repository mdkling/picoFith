
ARMGNU ?= arm-none-eabi

XCPU = -mcpu=cortex-m0plus

AOPS = --warn --fatal-warnings $(XCPU)
COPS = -Wall -Wno-pointer-sign -Os -ffreestanding $(XCPU)
LOPS = -nostdlib -nostartfiles
#~ LOCAL_TOOL_PATH = ../tool/gcc-arm-none-eabi/bin/
LOCAL_TOOL_PATH = 

# Windows Drive Letter (eg: D:\ == d)
DRIVE ?= d
UF2_TARGET_DIR = /mnt/$(DRIVE)


all : program.uf2 

program.uf2 : program.elf picoUF2
	./picoUF2 program.elf program.uf2

picoUF2: main.cpp
	g++ main.cpp -O1 -o picoUF2

program.elf : memmap.ld start.o program.o memory.o avl.o
	$(LOCAL_TOOL_PATH)$(ARMGNU)-ld $(LOPS) -T memmap.ld start.o memory.o avl.o program.o -o program.elf
	$(LOCAL_TOOL_PATH)$(ARMGNU)-objdump -D program.elf > program.list

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.uf2

start.o : start.s Makefile
	$(LOCAL_TOOL_PATH)$(ARMGNU)-as $(AOPS) start.s -o start.o

program.o : program.c Makefile
	$(LOCAL_TOOL_PATH)$(ARMGNU)-gcc $(COPS) -mthumb -c program.c -o program.o

memory.o : memory.c Makefile
	$(LOCAL_TOOL_PATH)$(ARMGNU)-gcc $(COPS) -mthumb -c memory.c -o memory.o

avl.o : avl.c Makefile
	$(LOCAL_TOOL_PATH)$(ARMGNU)-gcc $(COPS) -mthumb -c avl.c -o avl.o

program.c: program_src.c Makefile parser.c pio.c helperCpu.c
	re2c -W -i -s program_src.c -o program.c

$(UF2_TARGET_DIR):
	mkdir -p UF2_TARGET_DIR

testRun: program.uf2 | $(UF2_TARGET_DIR)
	sudo mount -t drvfs $(DRIVE): $(UF2_TARGET_DIR)
	cp program.uf2 $(UF2_TARGET_DIR)
