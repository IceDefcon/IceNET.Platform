TARGET=lwip
CC=arm-none-eabi-gcc
OBJCPY=arm-none-eabi-objcopy
CFLAGS= -DNDEBUG -DCPU_MK64FN1M0VLL12 -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 \
-DFRDM_K64F -DFREEDOM -DFSL_RTOS_FREE_RTOS -Os -Wall -fno-common \
-ffunction-sections -fdata-sections -ffreestanding -fno-builtin \
-mthumb -mapcs -std=gnu99 -mcpu=cortex-m4 -mfloat-abi=hard \
-mfpu=fpv4-sp-d16 -MMD -MP \
--specs=nano.specs --specs=nosys.specs -Wall -fno-common -ffunction-sections \
-fdata-sections -ffreestanding -fno-builtin -mthumb -mapcs -Xlinker --gc-sections \
-Xlinker -static -Xlinker -z -Xlinker muldefs -Xlinker -Map=output.map -mcpu=cortex-m4 \
-mfloat-abi=hard -mfpu=fpv4-sp-d16 -Xlinker --defsym=__stack_size__=2048 -Xlinker \
--defsym=__heap_size__=25600

LDSCRIPT=linker/MK64FN1M0xxx12_flash.ld
SRCS=\
    startup/startup_MK64F12.S \
    app/lwip_dhcp_freertos.c \
    app/pin_mux.c \
    app/board.c \
    app/clock_config.c \
    drivers/phyksz8081/fsl_phy.c \
    lwip/port/enet_ethernetif_kinetis.c \
    fsl/fsl_sim.c \
    fsl/fsl_clock.c \
    fsl/fsl_enet.c \
    FreeRTOS/src/portable/MemMang/heap_3.c \
    FreeRTOS/src/event_groups.c \
    FreeRTOS/src/list.c \
    FreeRTOS/src/portable/GCC/ARM_CM4F/port.c \
    FreeRTOS/src/queue.c \
    FreeRTOS/src/stream_buffer.c \
    FreeRTOS/src/tasks.c \
    FreeRTOS/src/timers.c \
    fsl/fsl_ftfx_cache.c \
    fsl/fsl_ftfx_controller.c \
    fsl/fsl_ftfx_flash.c \
    fsl/fsl_ftfx_flexnvm.c \
    fsl/fsl_gpio.c \
    fsl/fsl_debug_console.c \
    fsl/fsl_str.c \
    fsl/fsl_uart.c \
    fsl/fsl_smc.c \
    startup/startup_MK64F12.c \
    drivers/uart/uart_adapter.c \
    drivers/serial_manager/serial_manager.c \
    drivers/serial_manager/serial_port_uart.c \
    drivers/lists/generic_list.c \
    fsl/fsl_common.c \
    fsl/fsl_assert.c \
    lwip/port/enet_ethernetif.c \
    lwip/port/sys_arch.c \
    lwip/src/api/api_lib.c \
    lwip/src/api/api_msg.c \
    lwip/src/api/err.c \
    lwip/src/api/if_api.c \
    lwip/src/api/netbuf.c \
    lwip/src/api/netdb.c \
    lwip/src/api/netifapi.c \
    lwip/src/api/sockets.c \
    lwip/src/api/tcpip.c \
    lwip/src/core/altcp.c \
    lwip/src/core/altcp_alloc.c \
    lwip/src/core/altcp_tcp.c \
    lwip/src/core/def.c \
    lwip/src/core/dns.c \
    lwip/src/core/inet_chksum.c \
    lwip/src/core/init.c \
    lwip/src/core/ip.c \
    lwip/src/core/ipv4/autoip.c \
    lwip/src/core/ipv4/dhcp.c \
    lwip/src/core/ipv4/etharp.c \
    lwip/src/core/ipv4/icmp.c \
    lwip/src/core/ipv4/igmp.c \
    lwip/src/core/ipv4/ip4.c \
    lwip/src/core/ipv4/ip4_addr.c \
    lwip/src/core/ipv4/ip4_frag.c \
    lwip/src/core/ipv6/dhcp6.c \
    lwip/src/core/ipv6/ethip6.c \
    lwip/src/core/ipv6/icmp6.c \
    lwip/src/core/ipv6/inet6.c \
    lwip/src/core/ipv6/ip6.c \
    lwip/src/core/ipv6/ip6_addr.c \
    lwip/src/core/ipv6/ip6_frag.c \
    lwip/src/core/ipv6/mld6.c \
    lwip/src/core/ipv6/nd6.c \
    lwip/src/core/mem.c \
    lwip/src/core/memp.c \
    lwip/src/core/netif.c \
    lwip/src/core/pbuf.c \
    lwip/src/core/raw.c \
    lwip/src/core/stats.c \
    lwip/src/core/sys.c \
    lwip/src/core/tcp.c \
    lwip/src/core/tcp_in.c \
    lwip/src/core/tcp_out.c \
    lwip/src/core/timeouts.c \
    lwip/src/core/udp.c \
    lwip/src/netif/bridgeif.c \
    lwip/src/netif/bridgeif_fdb.c \
    lwip/src/netif/ethernet.c \
    lwip/src/netif/lowpan6.c \
    lwip/src/netif/lowpan6_ble.c \
    lwip/src/netif/lowpan6_common.c \
    lwip/src/netif/ppp/auth.c \
    lwip/src/netif/ppp/ccp.c \
    lwip/src/netif/ppp/chap-md5.c \
    lwip/src/netif/ppp/chap-new.c \
    lwip/src/netif/ppp/chap_ms.c \
    lwip/src/netif/ppp/demand.c \
    lwip/src/netif/ppp/eap.c \
    lwip/src/netif/ppp/eui64.c \
    lwip/src/netif/ppp/fsm.c \
    lwip/src/netif/ppp/ipcp.c \
    lwip/src/netif/ppp/ipv6cp.c \
    lwip/src/netif/ppp/lcp.c \
    lwip/src/netif/ppp/lwip_ecp.c \
    lwip/src/netif/ppp/magic.c \
    lwip/src/netif/ppp/mppe.c \
    lwip/src/netif/ppp/multilink.c \
    lwip/src/netif/ppp/ppp.c \
    lwip/src/netif/ppp/pppapi.c \
    lwip/src/netif/ppp/pppcrypt.c \
    lwip/src/netif/ppp/pppoe.c \
    lwip/src/netif/ppp/pppol2tp.c \
    lwip/src/netif/ppp/pppos.c \
    lwip/src/netif/ppp/upap.c \
    lwip/src/netif/ppp/utils.c \
    lwip/src/netif/ppp/vj.c \
    lwip/src/netif/slipif.c \
    lwip/src/netif/zepif.c \
    fsl/fsl_sbrk.c \

INCLUDES=\
    -Iapp/include \
    -IFreeRTOS/src/portable/GCC/ARM_CM4F \
    -ICMSIS \
    -IFreeRTOS/include \
    -IFreeRTOS/include/private \
    -Idrivers/phyksz8081 \
    -Ifsl \
    -Iinclude \
    -Idrivers/uart \
    -Idrivers/serial_manager \
    -Idrivers/lists \
    -Ilwip/port \
    -Ilwip/src \
    -Ilwip/src/include \

all:
	$(CC) $(INCLUDES) $(SRCS) $(CFLAGS) -T $(LDSCRIPT) -o $(TARGET).elf
	$(OBJCPY) $(TARGET).elf $(TARGET).bin -O binary

clean:
	rm $(TARGET).elf $(TARGET).bin $(TARGET).d output.map 
