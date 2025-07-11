###########################################################################################################################
# Timing Constants [ns]
###########################################################################################################################
#
#
#                               Hold Time Control (Maximum Output Delay): 40% of the clock period
# -max (Maximum Output Delay):  The maximum delay after the clock edge at which the signal can change without violating any timing requirements at the receiving device.
#                               This is used to ensure that the signal doesn't change too late, violating the hold time at the receiving end.
#
#
#                          Max 40% of period
#                          For the Hold time
#                                  |
#                                  |
#                                  |
#                                __V____         _______         _______
#                               |  |    |       |       |       |       |
#                        _______|  |    |_______|       |_______|       |_______
#                                  |
#                                  |
#                For 50Mhz clock is 20n Period its 8ns
#               So the maximum hold time after the edge
#                Of the clock cannot be more than 8ns
#           The signal can change up to 8 ns after the clock edge without violating the hold time.
#
#
#
#                               SETUP TIME Control (Minimum Output Delay): 13% of the clock time
# -min (Minimum Output Delay):  The minimum delay after the clock edge at which the signal can change.
#                               This ensures that the output signal is stable long enough to meet the setup time requirement of the receiving register.
#
#
#                      Min 13% of period
#                      For the Setup time
#                              |
#                              |
#                              |
#                              V  _____         _______         _______
#                              | |     |       |       |       |       |
#                        ______|_|     |_______|       |_______|       |_______
#                              |
#                              |
#           For 50Mhz clock is 20n Period is the 2.6ns
#            So the signal must be stable 3ns before
#                     The next clock edge
#
#
###########################################################################################################################
#
#
#                 ____V_____V____________
#                |    |     |            |
#     Input      |    |     |            |
#                |    |     |            |
#           _____|   13%    |            |______________
#                   Setup   |
#                     |    40%
#                     |    Hold
#                     |     |
#                     |  ___V_______             ___________             ___________
#                     | |           |           |           |           |           |
#                     | |           |           |           |           |           |
#                     | |           |           |           |           |           |
#           __________V_|           |___________|           |___________|           |____________
#
#
#
###########################################################################################################################

# The signal is expected to arrive no later than 5.0 ns after the clock edge
set MAX_I_DELAY_50MHz  8.0
# The signal must arrive no earlier than 2.0 ns after the clock edge
set MIN_I_DELAY_50MHz  3.0
# This specifies how long after the clock edge the signal is allowed to change
set MAX_O_DELAY_50MHz  8.0
# Indicating the earliest time the signal can change after the clock edge
set MIN_O_DELAY_50MHz  3.0

# It means that output signals can change no later than 3.0 ns after the rising edge of CLOCK_133MHz
set MAX_O_DELAY_133MHz 3.0
# The output can change no sooner than 1.0 ns after the rising edge of CLOCK_133MHz
set MIN_O_DELAY_133MHz 1.0
set MAX_I_DELAY_133MHz 3.0
set MIN_I_DELAY_133MHz 1.0

# 40% of 1Hz clock ---> 0.400ns :: Hold time
set MAX_O_DELAY_1GHz 0.4

# 13% of 1Hz clock ---> 0.130ns :: Setup time
set MIN_O_DELAY_1GHz 0.13

###########################################################################################################################
# Clock 50MHz :: Base
###########################################################################################################################

create_clock -name CLOCK_MAIN -period 20.000 [get_ports CLOCK_50MHz]
create_clock -name CLOCK_133MHz -period 7.516

derive_pll_clocks
derive_clock_uncertainty

# set CLOCK_133MHz [get_clocks PLL_RamClock_module|altpll_component|auto_generated|pll1|clk[0]]
# set CLOCK_266MHz [get_clocks PLL_RamClock_module|altpll_component|auto_generated|pll1|clk[1]]
# set CLOCK_1GHz [get_clocks PLL_FastClock_module|altpll_component|auto_generated|pll1|clk[0]]

###########################################################################################################################
# Input Constraints :: relative to the CLOCK_50MHz
###########################################################################################################################

# Button inputs
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {BUTTON_1 BUTTON_2 BUTTON_3 BUTTON_4}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {BUTTON_1 BUTTON_2 BUTTON_3 BUTTON_4}]

# SPI primary inputs
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {PRIMARY_CS PRIMARY_MOSI PRIMARY_SCLK}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {PRIMARY_CS PRIMARY_MOSI PRIMARY_SCLK}]

# SPI secondary inputs
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {SECONDARY_CS SECONDARY_MOSI SECONDARY_SCLK}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {SECONDARY_CS SECONDARY_MOSI SECONDARY_SCLK}]

# SPI BMI160 Sensor
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {S1_BMI160_MISO S2_BMI160_MISO S3_BMI160_MISO}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {S1_BMI160_MISO S2_BMI160_MISO S3_BMI160_MISO}]
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {S1_BMI160_INT_1 S1_BMI160_INT_2}]
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {S2_BMI160_INT_1 S2_BMI160_INT_2}]

# External SPI
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {EXTERNAL_MISO}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {EXTERNAL_MISO}]

# UART inputs
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {FPGA_UART_RX GPS_UART_RX}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {FPGA_UART_RX GPS_UART_RX}]

# CAN inputs
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {CAN_BBB_TX CAN_MPP_RX}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {CAN_BBB_TX CAN_MPP_RX}]

# Input interrupts
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {RESET_FROM_CPU}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {RESET_FROM_CPU}]

# Altera
set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {altera_reserved_tck altera_reserved_tdi altera_reserved_tms}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {altera_reserved_tck altera_reserved_tdi altera_reserved_tms}]

###########################################################################################################################
# Output Constraints
###########################################################################################################################

# LED outputs
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {LED_1 LED_2 LED_3 LED_4 LED_5 LED_6 LED_7 LED_8}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {LED_1 LED_2 LED_3 LED_4 LED_5 LED_6 LED_7 LED_8}]

# SPI primary outputs
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {PRIMARY_MISO}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {PRIMARY_MISO}]

# SPI secondary outputs
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {SECONDARY_MISO}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {SECONDARY_MISO}]

# SPI BMI160 Sensor
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {S1_BMI160_CS S1_BMI160_MOSI S1_BMI160_SCLK}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {S1_BMI160_CS S1_BMI160_MOSI S1_BMI160_SCLK}]
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {S2_BMI160_CS S2_BMI160_MOSI S2_BMI160_SCLK}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {S2_BMI160_CS S2_BMI160_MOSI S2_BMI160_SCLK}]
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {S3_BMI160_CS S3_BMI160_MOSI S3_BMI160_SCLK}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {S3_BMI160_CS S3_BMI160_MOSI S3_BMI160_SCLK}]

# EXTERNAL SPI
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {EXTERNAL_CS EXTERNAL_MOSI EXTERNAL_SCLK}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {EXTERNAL_CS EXTERNAL_MOSI EXTERNAL_SCLK}]

# UART outputs
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {FPGA_UART_TX GPS_UART_TX}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {FPGA_UART_TX GPS_UART_TX}]

# CAN outputs
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {CAN_BBB_RX CAN_MPP_TX}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {CAN_BBB_RX CAN_MPP_TX}]

# PWM output
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports PWM_SIGNAL]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports PWM_SIGNAL]

# Debug signals and logic channels
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {LOGIC_CH1 LOGIC_CH2 LOGIC_CH3 LOGIC_CH4 LOGIC_CH5 LOGIC_CH6}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {LOGIC_CH1 LOGIC_CH2 LOGIC_CH3 LOGIC_CH4 LOGIC_CH5 LOGIC_CH6}]

# Output Interrupts
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {SPI_INT_FROM_FPGA TIMER_INT_FROM_FPGA WDG_INT_FROM_FPGA}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {SPI_INT_FROM_FPGA TIMER_INT_FROM_FPGA WDG_INT_FROM_FPGA}]

# Altera
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports altera_reserved_tdo]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports altera_reserved_tdo]

###########################################################################################################################
# I2C Constraints
###########################################################################################################################

set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports I2C_SDA]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports I2C_SDA]
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports I2C_SDA]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports I2C_SDA]

set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports I2C_SCK]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports I2C_SCK]
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports I2C_SCK]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports I2C_SCK]

set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {I2C_ADXL345_INT_1 I2C_ADXL345_INT_2}]

###########################################################################################################################
# NRF905 Constraints
###########################################################################################################################

set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {NRF905_uCLK NRF905_CD NRF905_AM NRF905_DR NRF905_MISO}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {NRF905_uCLK NRF905_CD NRF905_AM NRF905_DR NRF905_MISO}]

set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {NRF905_TX_EN NRF905_TRX_CE NRF905_PWR_UP NRF905_MOSI NRF905_SCK NRF905_CSN}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {NRF905_TX_EN NRF905_TRX_CE NRF905_PWR_UP NRF905_MOSI NRF905_SCK NRF905_CSN}]

###########################################################################################################################
# SDRAM @ CLOCK_133MHz
###########################################################################################################################

set_output_delay -clock CLOCK_133MHz -max $MAX_O_DELAY_133MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
set_output_delay -clock CLOCK_133MHz -min $MIN_O_DELAY_133MHz [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]

set_input_delay -clock CLOCK_133MHz -max $MAX_I_DELAY_133MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
set_input_delay -clock CLOCK_133MHz -min $MIN_I_DELAY_133MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
set_output_delay -clock CLOCK_133MHz -max $MAX_O_DELAY_133MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
set_output_delay -clock CLOCK_133MHz -min $MIN_O_DELAY_133MHz [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]

set_output_delay -clock CLOCK_133MHz -max $MAX_O_DELAY_133MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]
set_output_delay -clock CLOCK_133MHz -min $MIN_O_DELAY_133MHz [get_ports {CLK_SDRAM BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]

# This is a 133Mhz clock shfited by 180° directly taken from PLL output
set_false_path -to [get_ports CLK_SDRAM]

###########################################################################################################################
# Debug
###########################################################################################################################

# set_false_path -to [get_ports {A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12}]
# set_false_path -to [get_ports {D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 D10 D11 D12 D13 D14 D15}]
# set_false_path -to [get_ports {BA0 BA1 CAS CKE RAS WE CS LDQM UDQM}]

set_input_delay -clock CLOCK_MAIN -max $MAX_I_DELAY_50MHz [get_ports {OUT_MISO TEST_CS TEST_MOSI TEST_CLK}]
set_input_delay -clock CLOCK_MAIN -min $MIN_I_DELAY_50MHz [get_ports {OUT_MISO TEST_CS TEST_MOSI TEST_CLK}]
set_output_delay -clock CLOCK_MAIN -max $MAX_O_DELAY_50MHz [get_ports {TEST_MISO OUT_CS OUT_MOSI OUT_CLK}]
set_output_delay -clock CLOCK_MAIN -min $MIN_O_DELAY_50MHz [get_ports {TEST_MISO OUT_CS OUT_MOSI OUT_CLK}]
