
ARMGNU ?= arm-none-eabi

XCPU = -mcpu=cortex-m0plus

AOPS = --warn --fatal-warnings $(XCPU)
COPS = -Wall -Wno-pointer-sign -Os -ffreestanding $(XCPU)
LOPS = -nostdlib -nostartfiles

all : program.uf2 

program.uf2 : program.elf picoUF2
	./picoUF2 program.elf program.uf2

picoUF2: main.cpp
	g++ main.cpp -O1 -o picoUF2

program.elf : memmap.ld start.o program.o memory.o avl.o
	$(ARMGNU)-ld $(LOPS) -T memmap.ld start.o memory.o program.o avl.o -o program.elf
	$(ARMGNU)-objdump -D program.elf > program.list

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.uf2

start.o : start.s
	$(ARMGNU)-as $(AOPS) start.s -o start.o

program.o : program.c Makefile
	$(ARMGNU)-gcc $(COPS) -mthumb -c program.c -o program.o

memory.o : memory.c Makefile
	$(ARMGNU)-gcc $(COPS) -mthumb -c memory.c -o memory.o

avl.o : avl.c Makefile
	$(ARMGNU)-gcc $(COPS) -mthumb -c avl.c -o avl.o

program.c: program_src.c Makefile parser.c pio.c
	re2c -W -i -s program_src.c -o program.c

testRun: program.uf2
	sudo mount -t drvfs d: /mnt/d
	cp program.uf2 /mnt/d

##-------------------------------------------------------------------------
##
## Copyright (c) 2021 David Welch dwelch@dwelch.com
##
## Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##
##-------------------------------------------------------------------------
