#
#
# Author: Ice.Marek
# IceNET Technology 2024
#
#
APP 		= app
GCC  		= g++
CFLAGS 		= -Wall -Wextra -std=c++11 -O2 -g -pthread

CPP_SOURCES = $(shell find . -name "*.cpp")
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

INCLUDES=\
	include \

all: $(APP)

$(APP): $(CPP_OBJECTS)
	$(GCC) $(CFLAGS) $^ -o $@ 
	size $(APP)

%.o: %.cpp
	$(GCC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(APP) $(CPP_OBJECTS)

.PHONY: all clean
