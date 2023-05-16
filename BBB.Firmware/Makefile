#
# Author: Ice.Marek
# 2023 IceNET Technology
#
APP 		= app
LINKED  	= app.linked
GCC  		= g++
CFLAGS 		= -Wall -Wextra -std=c++11 -O2 -g

LDSCRIPT    := linker/linker.ld

CPP_SOURCES = $(shell find . -name "*.cpp")
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

INCLUDES=\
	include \

all: $(APP) $(LINKED)

$(APP): $(CPP_OBJECTS)
	$(GCC) $(CFLAGS) $^ -o $@ 
	size $(APP)

$(LINKED): $(CPP_OBJECTS)
	$(GCC) $(CFLAGS) -T $(LDSCRIPT) $^ -o $@ 
	size $(APP)

%.o: %.cpp
	$(GCC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(APP) $(LINKED) $(CPP_OBJECTS)

size:
	size $(APP)

.PHONY: all clean size
