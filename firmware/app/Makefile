#
# Author: Ice.Marek
# IceNET Technology 2025
#

APP         = app
GCC         = g++
CFLAGS      = -Wall -Wextra -std=c++14 -O2 -g -pthread

CPP_SOURCES = $(shell find . -name "*.cpp")
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

INCLUDES = \
    include \
    include/BlockDevice \
    include/CharDevice \
    include/Network

CFLAGS += $(addprefix -I, $(INCLUDES))

all: $(APP)

$(APP): $(CPP_OBJECTS)
	$(GCC) $(CFLAGS) $^ -o $@
	size $(APP)

%.o: %.cpp
	$(GCC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(APP) $(CPP_OBJECTS)

.PHONY: all clean
