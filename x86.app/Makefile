#
# Author: Ice.Marek
# 2023 IceNET Technology
#
DYNAMIC 	= dynamic
STATIC  	= static
GCC  		= g++
NASM 		= nasm 

AFLAGS 		= -f elf64
CFLAGS 		= -m64

LDSCRIPT    := linker/linker.ld

ASM_SOURCES = $(shell find . -name "*.asm")
CPP_SOURCES = $(shell find . -name "*.cpp")

INCLUDES=\
	include \

CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

all: $(DYNAMIC) $(STATIC)

$(DYNAMIC): $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(GCC) $(CFLAGS) $^ -o $@ 
	size $(DYNAMIC)

$(STATIC): $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(GCC) $(CFLAGS) -T $(LDSCRIPT) $^ -o $@ 
	size $(DYNAMIC)
	cd module && $(MAKE)

%.o: %.cpp
	$(GCC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

%.o: %.asm
	$(NASM) $(AFLAGS) $(ASM_SOURCES) -o $@

clean:
	rm -f $(DYNAMIC) $(STATIC) $(CPP_OBJECTS) $(ASM_OBJECTS)
	cd module && $(MAKE) clean

size:
	size $(DYNAMIC)

.PHONY: all clean size
