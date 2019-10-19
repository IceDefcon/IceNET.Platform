# IceNET.NetworkAnalyser
Detect and Report Security Anomalies within Computer Network

----------------------------------------------------------------
1. Linux kernel module runs TCP/IP server and sends the commands to the Cortex-M4F
2. Cortex M4F is driven by FreeRTOS 
3. FreeRTOS run Light weight TCP/IP stack to communicate with x86 terminal
4. FreeRTOS run dspi driver to communicate with Cyclon II FPGA
5. FPGA recieve master commands via SPI
6. FPGA execute master commands via Nios II Firmware



# ICE
