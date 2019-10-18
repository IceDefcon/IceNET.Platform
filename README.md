# IceNET.NetworkAnalyser
Detect and Report Security Anomalies within the Computer Network

----------------------------------------------------------------
1. Linux kernel module runs TCP/IP server and sends the commands to the Cortex-M4F
2. Cortex M4F is driven by FreeRTOS 
3. FreeRTOS run Light weight TCP/IP stack to Rx/Tx commands from x86 terminal
4. FreeRTOS run dspi driver to forward commands from x86 terminal to Cyclon II FPGA
5. FPGA recieve master commands from x86 Linux kernel module via SPI
6. FPGA execute master commands in Nios II Firmware



# ICE
