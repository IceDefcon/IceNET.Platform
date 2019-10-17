# IceNET.NetworkAnalyser

----------------------------------------------------------------
Detect and Report Security Anomalies within the Computer Network

----------------------------------------------------------------
1. x86  → Linux application for overall master control
2. ARM  → Driven by FreeRTOS
3. RTOS → TCP/IP Stack for Communication with x86 terminal
4. RTOS → SPI interface for ARM request → FPGA
5. FPGA → SPI interface for FPGA response → ARM
6. FPGA → Driven by Nios II Firmware
7. FPGA → Fast Fourier transform and digital filtering
8. FPGA → External 640Mbps and 800Mbps LVDS channels for
          Rx and Tx respectively to communicate with the 
          Ethernet target network 
