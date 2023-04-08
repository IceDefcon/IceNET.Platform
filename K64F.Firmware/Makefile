TARGET=analyser
CC=arm-none-eabi-gcc
OBJCPY=arm-none-eabi-objcopy
CFLAGS= -DNDEBUG -DCPU_MK64FN1M0VLL12 -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 \
-DFRDM_K64F -DFREEDOM -DFSL_RTOS_FREE_RTOS -Os -Wall -fno-common \
-ffunction-sections -fdata-sections -ffreestanding -fno-builtin \
-mthumb -mapcs -std=gnu99 -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MMD -MP \
--specs=nano.specs --specs=nosys.specs -Wall -fno-common -ffunction-sections \
-fdata-sections -ffreestanding -fno-builtin -mthumb -mapcs -Xlinker --gc-sections \
-Xlinker -static -Xlinker -z -Xlinker muldefs -Xlinker -Map=output.map -mcpu=cortex-m4 \
-mfloat-abi=hard -mfpu=fpv4-sp-d16 -Xlinker --defsym=__stack_size__=2048 -Xlinker \
--defsym=__heap_size__=25600 -Wall

LDSCRIPT=linker/MK64FN1M0xxx12_flash.ld

ASM_SOURCES = startup/startup_MK64F12.S
GCC_SOURCES = $(shell find . -name "*.c")

INCLUDES=\
    -Iapp \
    -ICMSIS \
    -Ifsl \
    -IFreeRTOS/include \
    -IFreeRTOS/include/private \
    -IFreeRTOS/src/portable/GCC/ARM_CM4F \
    -Iinclude \
    -Idrivers/uart \
    -Idrivers/serial_manager \
    -Idrivers/lists \
    -Ilwip/port \
    -Ilwip/src \
    -Ilwip/src/include \

ASM_OBJECTS = $(ASM_SOURCES:.S=.o)
GCC_OBJECTS = $(GCC_SOURCES:.c=.o)
DEP_OBJECTS = $(GCC_SOURCES:.c=.d)

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCPY) $< $@ -O binary

$(TARGET).elf: $(GCC_OBJECTS) $(ASM_OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(ASM_SOURCES) -T $(LDSCRIPT) $^

%.o: %.c
	$(CC) -I $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).d output.map $(GCC_OBJECTS) $(DEP_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean remove
