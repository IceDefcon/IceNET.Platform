#
# Author: Ice.Marek
# IceNET Technology 2025
#

APP         = app
GCC         = g++
CFLAGS      = -Wall -Wextra -std=c++14 -O2 -g -pthread
MAKEFLAGS  += -j4

VERBOSE     := @

CPP_SOURCES = $(shell find . -name "*.cpp")
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

INCLUDES = \
    include \
    include/Diagnostics \
    include/KernelComms \
    include/KernelComms/BlockDevice \
    include/KernelComms/CharDevice \
    include/Network

CFLAGS += $(addprefix -I, $(INCLUDES))

all: $(APP)

$(APP): $(CPP_OBJECTS)
	@echo " LINKING  $(APP)"
	$(VERBOSE) $(GCC) $(CFLAGS) $^ -o $@
	@size $(APP)

%.o: %.cpp
	@echo " COMPILE  $<"
	$(VERBOSE) $(GCC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " CLEAN"
	rm -f $(APP) $(CPP_OBJECTS)

.PHONY: all clean

