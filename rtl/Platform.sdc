####################################################################################################################################
# Clock 50MHz
####################################################################################################################################

set MAX_I_DELAY_50MHz  2
set MAX_O_DELAY_50MHz  2
set MIN_I_DELAY_50MHz  1
set MIN_O_DELAY_50MHz  1

create_clock -name {CLOCK_50MHz} -period 20 CLOCK_50MHz
# create_clock -name {CLOCK_50MHz} -period 20 -waveform {0.000 10.000} [get_nets {CLOCK_50MHz}]

set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz -fall [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz -fall [get_nets CLOCK_50MHz]

set_clock_uncertainty 0.2 -rise_from [get_clocks {CLOCK_50MHz}] -to [get_clocks {CLOCK_50MHz}]
set_clock_uncertainty 0.1 -fall_from [get_clocks {CLOCK_50MHz}] -to [get_clocks {CLOCK_50MHz}]

###########################################################################################################################
# Clock 50MHz - Derived from PLL
###########################################################################################################################

set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz -fall [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz -fall [get_nets CLOCK_50MHz]

set_clock_uncertainty 0.2 -rise_from [get_clocks {CLOCK_50MHz}] -to [get_clocks {CLOCK_50MHz}]
set_clock_uncertainty 0.1 -fall_from [get_clocks {CLOCK_50MHz}] -to [get_clocks {CLOCK_50MHz}]

set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]

# create_clock -name {CLOCK_50MHz} -period 5 -waveform {0.000 2.500} [get_nets {CLOCK_50MHz}]

# set MAX_I_DELAY_200MHz 0.5
# set MAX_O_DELAY_200MHz 0.5
# set MIN_I_DELAY_200MHz 0.1
# set MIN_O_DELAY_200MHz 0.1

# # create_clock -name {CLOCK_200MHz} -period 5 CLOCK_200MHz
# create_clock -name {CLOCK_200MHz} -period 5 -waveform {0.000 2.500} [get_nets {CLOCK_200MHz}]

# set_input_delay -clock [get_clocks CLOCK_200MHz] -max $MAX_I_DELAY_200MHz -rise [get_nets CLOCK_200MHz]
# set_input_delay -clock [get_clocks CLOCK_200MHz] -max $MAX_I_DELAY_200MHz -fall [get_nets CLOCK_200MHz]
# set_input_delay -clock [get_clocks CLOCK_200MHz] -min $MIN_I_DELAY_200MHz -rise [get_nets CLOCK_200MHz]
# set_input_delay -clock [get_clocks CLOCK_200MHz] -min $MIN_I_DELAY_200MHz -fall [get_nets CLOCK_200MHz]

# set_clock_uncertainty 0.2 -rise_from [get_clocks {CLOCK_200MHz}] -to [get_clocks {CLOCK_200MHz}]
# set_clock_uncertainty 0.1 -fall_from [get_clocks {CLOCK_200MHz}] -to [get_clocks {CLOCK_200MHz}]

# set_output_delay -clock [get_clocks CLOCK_200MHz] -max $MAX_O_DELAY_200MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
# set_output_delay -clock [get_clocks CLOCK_200MHz] -min $MIN_O_DELAY_200MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
# set_output_delay -clock [get_clocks CLOCK_200MHz] -max $MAX_O_DELAY_200MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
# set_output_delay -clock [get_clocks CLOCK_200MHz] -min $MIN_O_DELAY_200MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
# set_output_delay -clock [get_clocks CLOCK_200MHz] -max $MAX_O_DELAY_200MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]
# set_output_delay -clock [get_clocks CLOCK_200MHz] -min $MIN_O_DELAY_200MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]

###########################################################################################################################
# Input Constraints :: relative to the CLOCK_50MHz
###########################################################################################################################

# Button inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {BUTTON_1 BUTTON_2 BUTTON_3 BUTTON_4}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {BUTTON_1 BUTTON_2 BUTTON_3 BUTTON_4}]

# SPI primary inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {PRIMARY_CS PRIMARY_MOSI PRIMARY_SCLK}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {PRIMARY_CS PRIMARY_MOSI PRIMARY_SCLK}]

# SPI secondary inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {SECONDARY_CS SECONDARY_MOSI SECONDARY_SCLK}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {SECONDARY_CS SECONDARY_MOSI SECONDARY_SCLK}]

# UART inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {UART_BBB_TX UART_x86_RX}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {UART_BBB_TX UART_x86_RX}]

# CAN inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {CAN_BBB_TX CAN_MPP_RX}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {CAN_BBB_TX CAN_MPP_RX}]

# Other inputs
set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {INT_FROM_CPU WDG_INT_FROM_CPU}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {INT_FROM_CPU WDG_INT_FROM_CPU}]

###########################################################################################################################
# Output Constraints
###########################################################################################################################

# LED outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {LED_1 LED_2 LED_3 LED_4 LED_5 LED_6 LED_7 LED_8}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {LED_1 LED_2 LED_3 LED_4 LED_5 LED_6 LED_7 LED_8}]

# SPI primary outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {PRIMARY_MISO}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {PRIMARY_MISO}]

# SPI secondary outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {SECONDARY_MISO}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {SECONDARY_MISO}]

# UART outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {UART_BBB_RX UART_x86_TX}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {UART_BBB_RX UART_x86_TX}]

# CAN outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {CAN_BBB_RX CAN_MPP_TX}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {CAN_BBB_RX CAN_MPP_TX}]

# PWM output
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports PWM_SIGNAL]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports PWM_SIGNAL]

# Debug signals and logic channels
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {LOGIC_CH1 LOGIC_CH2 LOGIC_CH3 LOGIC_CH4 LOGIC_CH5 LOGIC_CH6}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {LOGIC_CH1 LOGIC_CH2 LOGIC_CH3 LOGIC_CH4 LOGIC_CH5 LOGIC_CH6}]

# Other outputs
set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {INT_FROM_FPGA WDG_INT_FROM_FPGA}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {INT_FROM_FPGA WDG_INT_FROM_FPGA}]

###########################################################################################################################
# I2C Constraints
###########################################################################################################################

set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports I2C_SDA]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports I2C_SDA]

set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports I2C_SCK]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports I2C_SCK]

# False path for I2C SDA and SCL
set_false_path -to [get_ports I2C_SDA]
set_false_path -to [get_ports I2C_SCK]

###########################################################################################################################
# NRF905 Constraints
###########################################################################################################################

set_input_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_I_DELAY_50MHz [get_ports {NRF905_uCLK NRF905_CD NRF905_AM NRF905_DR NRF905_MISO}]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_I_DELAY_50MHz [get_ports {NRF905_uCLK NRF905_CD NRF905_AM NRF905_DR NRF905_MISO}]

set_output_delay -clock [get_clocks CLOCK_50MHz] -max $MAX_O_DELAY_50MHz [get_ports {NRF905_TX_EN NRF905_TRX_CE NRF905_PWR_UP NRF905_MOSI NRF905_SCK NRF905_CSN}]
set_output_delay -clock [get_clocks CLOCK_50MHz] -min $MIN_O_DELAY_50MHz [get_ports {NRF905_TX_EN NRF905_TRX_CE NRF905_PWR_UP NRF905_MOSI NRF905_SCK NRF905_CSN}]

