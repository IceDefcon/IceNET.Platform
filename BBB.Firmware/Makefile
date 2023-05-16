#
# Author: Ice.Marek
# 2023 IceNET Technology
#
APP 		= app
LINKED  	= app.linked
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

all: $(APP) $(LINKED)

$(APP): $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(GCC) $(CFLAGS) $^ -o $@ 
	size $(APP)

$(LINKED): $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(GCC) $(CFLAGS) -T $(LDSCRIPT) $^ -o $@ 
	size $(APP)

%.o: %.cpp
	$(GCC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

%.o: %.asm
	$(NASM) $(AFLAGS) $(ASM_SOURCES) -o $@

clean:
	rm -f $(APP) $(LINKED) $(CPP_OBJECTS) $(ASM_OBJECTS)

size:
	size $(APP)

.PHONY: all clean size
