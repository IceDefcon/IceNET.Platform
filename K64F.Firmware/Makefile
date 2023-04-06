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

ASM_SOURCES := $(shell find . -name "*.S")
CPP_SOURCES := $(shell find . -name "*.c")

INCLUDES=\
    -Iapp/include \
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


all:
	$(CC) $(INCLUDES) $(CPP_SOURCES) $(ASM_SOURCES) $(CFLAGS) -T $(LDSCRIPT) -o $(TARGET).elf
	$(OBJCPY) $(TARGET).elf $(TARGET).bin -O binary

clean:
	rm $(TARGET).elf $(TARGET).bin $(TARGET).d output.map 
