library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.Types.all;

-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
--
-- Author: Ice.Marek
-- IceNET Technology 2025
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
--
-- Pinout Communication
-- CPU <--> FPGA
--
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
--
-- VIN     :: VIN
-- GND     :: GND
-- PIN_A20 :: PIN_B20
-- _________________________________________________________________________________
--                      Λ                                           Λ
-- PIN_A19 :: PIN_B19   | TIMER_INT_FROM_FPGA   :: FPGA_UART_RX     | H7  :: H8
-- PIN_A18 :: PIN_B18   |                       :: FPGA_UART_TX     | H9  :: H10
-- PIN_A17 :: PIN_B17   |                       ::                  | H11 :: H12
-- PIN_A16 :: PIN_B16   | SECONDARY_SCLK        ::                  | H13 :: H14
-- PIN_A15 :: PIN_B15   |                       ::                  | H15 :: H16
-- PIN_A14 :: PIN_B14   | ----===[ 3V3 ]===---- :: SECONDARY_CS     | H17 :: H18
-- PIN_A13 :: PIN_B13   | PRIMARY_MOSI          ::                  | H19 :: H20
-- PIN_A10 :: PIN_B10   | PRIMARY_MISO          :: SECONDARY_MISO   | H21 :: H22
-- PIN_A9  :: PIN_B9    | PRIMARY_SCLK          :: PRIMARY_CS       | H23 :: H24
-- PIN_A8  :: PIN_B8    |                       ::                  | H25 :: H26
-- PIN_A7  :: PIN_B7    |                       ::                  | H27 :: H28
-- PIN_A6  :: PIN_B6    | SPI_INT_FROM_FPGA     ::                  | H29 :: H30
-- PIN_A5  :: PIN_B5    | WDG_INT_FROM_FPGA     :: RESET_FROM_CPU   | H31 :: H32
-- PIN_C3  :: PIN_C4    |                       ::                  | H33 :: H34
-- PIN_A4  :: PIN_B4    |                       ::                  | H35 :: H36
-- PIN_A3  :: PIN_B3    | SECONDARY_MOSI        ::                  | H37 :: H38
-- PIN_B2  :: PIN_B1    |                       ::                  | H39 :: H40
-- _____________________V___________________________________________V_______________
--
-- PIN_C2  :: PIN_C1
-- PIN_D2  :: PIN_E1
-- PIN_F2  :: PIN_F1
-- PIN_H2  :: PIN_H1
-- PIN_J2  :: PIN_J1
-- PIN_M2  :: PIN_M1
-- PIN_N2  :: PIN_N1
-- PIN_P2  :: PIN_P1
-- PIN_R2  :: PIN_R1
-- GND :: GND
-- 3V3 :: 3V3
-- GND :: GND
--
--
--
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
--
-- VIN      :: VIN
-- GND      :: GND
-- PIN_M19  :: GPS_UART_RX                  | PIN_M20  :: NOTUSED_54
-- PIN_N19  :: GPS_UART_TX                  | PIN_N20  :: NOTUSED_52
-- PIN_B21  :: NOTUSED_49                   | PIN_B22  :: NOTUSED_50
-- PIN_C21  :: NOTUSED_47                   | PIN_C22  :: NOTUSED_48
-- PIN_D21  :: NOTUSED_45                   | PIN_D22  :: NOTUSED_46
-- PIN_E21  :: NOTUSED_43                   | PIN_E22  :: NOTUSED_44
-- PIN_F21  :: NOTUSED_41                   | PIN_F22  :: NOTUSED_42
-- PIN_H21  :: NOTUSED_39                   | PIN_H22  :: NOTUSED_40
-- PIN_J21  :: NOTUSED_37                   | PIN_J22  :: NOTUSED_38
-- PIN_K21  :: NOTUSED_35                   | PIN_K22  :: NOTUSED_36
-- PIN_L21  :: NOTUSED_33                   | PIN_L22  :: NOTUSED_34
-- PIN_M21  :: NOTUSED_31                   | PIN_M22  :: NOTUSED_32
-- PIN_N21  :: NOTUSED_29                   | PIN_N22  :: NOTUSED_30
-- PIN_P21  :: NOTUSED_27                   | PIN_P22  :: NOTUSED_28
-- PIN_R21  :: NOTUSED_25                   | PIN_R22  :: NOTUSED_26
-- PIN_U21  :: NOTUSED_23                   | PIN_U22  :: NOTUSED_24
-- PIN_V21  :: NOTUSED_21                   | PIN_V22  :: NOTUSED_22
-- PIN_W21  :: NOTUSED_19                   | PIN_W22  :: NOTUSED_20
-- PIN_Y21  :: I2C_SDA                      | PIN_Y22  :: I2C_SCK
-- PIN_AB20 :: NOTUSED_15                   | PIN_AA20 :: S2_BMI160_MISO
-- PIN_AB19 :: NOTUSED_13                   | PIN_AA19 :: S2_BMI160_CS
-- PIN_AB18 :: S2_BMI160_INT_1              | PIN_AA18 :: S2_BMI160_MOSI
-- PIN_AB17 :: S2_BMI160_INT_2              | PIN_AA17 :: S2_BMI160_SCLK
-- PIN_AB16 :: S1_BMI160_INT_1              | PIN_AA16 :: S1_BMI160_MISO
-- PIN_AB15 :: S1_BMI160_INT_2              | PIN_AA15 :: S1_BMI160_CS
-- PIN_AB14 :: NOTUSED_03                   | PIN_AA14 :: S1_BMI160_MOSI
-- PIN_AB13 :: NOTUSED_01                   | PIN_AA13 :: S1_BMI160_SCLK
-- GND      :: GND
-- 3V3      :: 3V3
-- GND      :: GND
--
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------

entity Platform is
port
(
    CLOCK_50MHz : in std_logic; -- PIN_T2

    -----------------------------------------------------------------------------
    -- DEBUG
    -----------------------------------------------------------------------------
    LED_1 : out std_logic; -- PIN_U7
    LED_2 : out std_logic; -- PIN_U8
    LED_3 : out std_logic; -- PIN_R7
    LED_4 : out std_logic; -- PIN_T8
    LED_5 : out std_logic; -- PIN_R8
    LED_6 : out std_logic; -- PIN_P8
    LED_7 : out std_logic; -- PIN_M8
    LED_8 : out std_logic; -- PIN_N8
    BUTTON_1 : in std_logic; -- PIN_H20
    BUTTON_2 : in std_logic; -- PIN_K19
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic; -- PIN_K18
    LOGIC_CH1 : out std_logic; -- PIN_B21
    LOGIC_CH2 : out std_logic; -- PIN_B22
    LOGIC_CH3 : out std_logic; -- PIN_C21
    LOGIC_CH4 : out std_logic; -- PIN_C22
    LOGIC_CH5 : out std_logic; -- PIN_D21
    LOGIC_CH6 : out std_logic; -- PIN_D22
    LOGIC_CH7 : out std_logic; -- PIN_E21
    LOGIC_CH8 : out std_logic; -- PIN_E22

    -----------------------------------------------------------------------------
    -- Kernel Communication
    -----------------------------------------------------------------------------
    SPI_INT_FROM_FPGA : out std_logic; -- PIN_A6 :: GPIO01 :: HEADER_PIN_29
    TIMER_INT_FROM_FPGA : out std_logic; -- PIN_A19 :: GPIO09 :: HEADER_PIN_07
    WDG_INT_FROM_FPGA : out std_logic; -- PIN_A5 :: GPIO11 :: HEADER_PIN_31
    RESET_FROM_CPU : in std_logic; -- PIN_B5 :: GPIO07 :: HEADER_PIN_32

    PRIMARY_MOSI : in std_logic;  -- PIN_B6 :: H19 :: SPI0_MOSI :: SDA
    PRIMARY_MISO : out std_logic; -- PIN_A8 :: H21 :: SPI0_MISO :: SAO
    PRIMARY_SCLK : in std_logic;  -- PIN_B8 :: H23 :: SPI0_SCLK :: SCL
    PRIMARY_CS : in std_logic;    -- PIN_A6 :: H24 :: SPI0_CS0  :: CS

    SECONDARY_MOSI : in std_logic;  -- PIN_B14 :: P9_30 :: SPI1_D1
    SECONDARY_MISO : out std_logic; -- PIN_A14 :: P9_29 :: SPI1_D0
    SECONDARY_SCLK : in std_logic;  -- PIN_A15 :: P9_31 :: SPI1_SCLK
    SECONDARY_CS : in std_logic;    -- PIN_B13 :: P9_28 :: SPI1_CS0

    -- EXTERNAL SPI
    EXTERNAL_CS : out std_logic;   -- PIN_F21
    EXTERNAL_MISO : in std_logic;  -- PIN_H21
    EXTERNAL_MOSI : out std_logic; -- PIN_J21
    EXTERNAL_SCLK : out std_logic; -- PIN_K21

    -----------------------------------------------------------------------------
    -- 256Mbit SDRAM
    -----------------------------------------------------------------------------
    A0 : out std_logic; -- PIN_V2
    A1 : out std_logic; -- PIN_V1
    A2 : out std_logic; -- PIN_U2
    A3 : out std_logic; -- PIN_U1
    A4 : out std_logic; -- PIN_V3
    A5 : out std_logic; -- PIN_V4
    A6 : out std_logic; -- PIN_Y2
    A7 : out std_logic; -- PIN_AA1
    A8 : out std_logic; -- PIN_Y3
    A9 : out std_logic; -- PIN_V5
    A10 : out std_logic; -- PIN_W1
    A11 : out std_logic; -- PIN_Y4
    A12 : out std_logic; -- PIN_V6
    CLK_SDRAM : out std_logic; -- PIN_Y6
    BA0 : out std_logic; -- A13 :: PIN_Y1
    BA1 : out std_logic; -- A14 :: PIN_W2
    CAS : out std_logic; -- PIN_AA4
    CKE : out std_logic; -- PIN_W6
    RAS : out std_logic; -- PIN_AB3
    WE : out std_logic; -- PIN_AB4
    CS : out std_logic; -- PIN_AA3
    D0 : inout std_logic; -- PIN_AA10
    D1 : inout std_logic; -- PIN_AB9
    D2 : inout std_logic; -- PIN_AA9
    D3 : inout std_logic; -- PIN_AB8
    D4 : inout std_logic; -- PIN_AA8
    D5 : inout std_logic; -- PIN_AB7
    D6 : inout std_logic; -- PIN_AA7
    D7 : inout std_logic; -- PIN_AB5
    D8 : inout std_logic; -- PIN_Y7
    D9 : inout std_logic; -- PIN_W8
    D10 : inout std_logic; -- PIN_Y8
    D11 : inout std_logic; -- PIN_V9
    D12 : inout std_logic; -- PIN_V10
    D13 : inout std_logic; -- PIN_Y10
    D14 : inout std_logic; -- PIN_W10
    D15 : inout std_logic; -- PIN_V11
    LDQM : inout std_logic; -- PIN_AA5
    UDQM : inout std_logic; -- PIN_W7

    -----------------------------------------------------------------------------
    -- Peripheral Interfaces
    -----------------------------------------------------------------------------
    -- UART
    GPS_UART_RX : in std_logic; -- PIN_M19
    GPS_UART_TX : out std_logic; -- PIN_N19

    FPGA_UART_RX : in std_logic;  -- PIN_B19 :: H8  -> JetsonNano UART1_TXD
    FPGA_UART_TX : out std_logic; -- PIN_B18 :: H10 -> JetsonNano UART1_RXD
    -- I2C Bus
    I2C_SDA : inout std_logic; -- PIN_Y21
    I2C_SCK : inout std_logic; -- PIN_Y22
    -- PWM
    PWM_SIGNAL : out std_logic; -- PIN_R1
    -- SPI
    S1_BMI160_SCLK : out std_logic; -- PIN_AA13
    S1_BMI160_MOSI : out std_logic; -- PIN_AA14
    S1_BMI160_CS : out std_logic;   -- PIN_AA15
    S1_BMI160_MISO : in std_logic;  -- PIN_AA16
    S1_BMI160_INT_1 : in std_logic; -- PIN_AB16
    S1_BMI160_INT_2 : in std_logic; -- PIN_AB15
    S2_BMI160_SCLK : out std_logic; -- PIN_AA17
    S2_BMI160_MOSI : out std_logic; -- PIN_AA18
    S2_BMI160_CS : out std_logic;   -- PIN_AA19
    S2_BMI160_MISO : in std_logic;  -- PIN_AA20
    S2_BMI160_INT_1 : in std_logic; -- PIN_AB18
    S2_BMI160_INT_2 : in std_logic; -- PIN_AB17
    -- Outer PCB Side
    NRF905_TRX_CE : out std_logic;  -- PIN_H2
    NRF905_uCLK : in std_logic;     -- PIN_J2
    NRF905_AM : in std_logic;       -- PIN_M2
    NRF905_MISO : in std_logic;     -- PIN_N2
    NRF905_SCK : out std_logic;     -- PIN_P2
    -- Iner PCB side
    NRF905_TX_EN : out std_logic;   -- PIN_F1
    NRF905_PWR_UP : out std_logic;  -- PIN_H1
    NRF905_CD : in std_logic;       -- PIN_J1
    NRF905_DR : in std_logic;       -- PIN_M1
    NRF905_MOSI : out std_logic;    -- PIN_N1
    NRF905_CSN : out std_logic      -- PIN_P1
);
end Platform;

architecture rtl of Platform is

----------------------------------------------------------------------------------------------------------------
-- Synced :: Asynchronic Input Signals
----------------------------------------------------------------------------------------------------------------

-- PRIMARY SPI
signal synced_PRIMARY_MOSI : std_logic := '0';
signal synced_PRIMARY_SCLK : std_logic := '0';
signal synced_PRIMARY_CS : std_logic := '0';
-- SECONDARY SPI
signal synced_SECONDARY_MOSI : std_logic := '0';
signal synced_SECONDARY_SCLK : std_logic := '0';
signal synced_SECONDARY_CS : std_logic := '0';
-- Main UART
signal synced_FPGA_UART_RX : std_logic := '0';
signal synced_GPS_UART_RX : std_logic := '0';
-- Buttons
signal active_button_1 : std_logic := '0';
-- Spi.0 Primary
signal primary_conversion_complete : std_logic := '0';
signal primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
-- Spi.1 Secondary
signal secondary_parallel_MISO : std_logic_vector(7 downto 0) := (others => '0');

-- Interrupt Vector stage signals
signal FIFO_primary_fifo_wr_en  : std_logic := '0';
signal FIFO_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
-- Interrupt vector interrupts
signal external_offload_vector_interrupt : std_logic := '0';
signal primary_offload_vector_interrupt : std_logic := '0';
signal enable_vector_interrupt : std_logic := '0';
signal start_vector_interrupt : std_logic := '0';
signal trigger_vector_interrupt : std_logic := '0';
signal secondary_dma_trigger_gpio_pulse_20ns : std_logic := '0';
-- Fifo
signal primary_fifo_rd_en : std_logic := '0';
signal primary_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');
signal primary_fifo_full : std_logic := '0';
signal primary_fifo_empty : std_logic := '0';
signal sensor_fifo_bit_count : std_logic_vector(3 downto 0) := (others => '0');
-- Primary Offload
signal primary_offload_ready : std_logic := '0';
signal primary_offload_id : std_logic_vector(7 downto 0) := (others => '0');
signal primary_offload_ctrl : std_logic_vector(7 downto 0) := (others => '0');
signal primary_offload_register : std_logic_vector(7 downto 0) := (others => '0');
signal primary_offload_data : std_logic_vector(7 downto 0) := (others => '0');
-- Primary Offload Wait
signal primary_offload_wait : std_logic := '0';
signal primary_offload_wait_i2c : std_logic := '0';
signal primary_offload_wait_spi_s1 : std_logic := '0';
signal primary_offload_wait_spi_s2 : std_logic := '0';
signal primary_offload_wait_spi_rf : std_logic := '0';
-- PacketSwitch
signal trigger_i2c : std_logic := '0';
signal trigger_bmi160_s1 : std_logic := '0';
signal trigger_bmi160_s2 : std_logic := '0';
signal trigger_external_spi : std_logic := '0';
signal trigger_nRF905 : std_logic := '0';
signal trigger_pwm_m1 : std_logic := '0';
-- CTRL Mux
constant CTRL_I2C : std_logic_vector(1 downto 0) := "00";
constant CTRL_SPI : std_logic_vector(1 downto 0) := "01";
constant CTRL_PWM : std_logic_vector(1 downto 0) := "10";
-- ID Mux
constant ID_BMI160_S1 : std_logic_vector(7 downto 0) := "01000100"; -- "001 0001" :: 0x11 :: Must be upside down :: Due to primary_offload_id for i2c
constant ID_BMI160_S2 : std_logic_vector(7 downto 0) := "00100100"; -- "001 0010" :: 0x12
constant ID_EXTERNAL_SPI : std_logic_vector(7 downto 0) := "01100100"; -- "001 0010" :: 0x13
constant ID_nRF905 : std_logic_vector(7 downto 0) := "00010100"; -- Must be upside down :: Same as upside down
-- Secondary DMA Trigger Interrupts
signal i2c_complete_long : std_logic := '0';
signal spi_nRF905_complete_long : std_logic := '0';
signal spi_bmi160_s1_complete_long : std_logic := '0';
signal spi_bmi160_s2_complete_long : std_logic := '0';
signal pwm_m1_complete_long : std_logic := '0';
-- Feedback data
signal data_i2c_feedback : std_logic_vector(7 downto 0) := (others => '0');
signal data_spi_rf_feedback : std_logic_vector(7 downto 0) := "00010001";
signal data_spi_bmi160_s1_feedback : std_logic_vector(7 downto 0) := "00010101";
signal data_spi_bmi160_s2_feedback : std_logic_vector(7 downto 0) := "00010110";
signal data_pwm_feedback : std_logic_vector(7 downto 0) := "11000011";
-- UART
signal uart_write_enable : std_logic := '0';
signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_write_busy : std_logic := '0';
-- UART Test Log
signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("1101", "1110"); -- 0xDE
signal UART_LOG_MESSAGE_KEY : UART_LOG_KEY := ("1010", "1101"); -- 0xAD
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("1100", "0000", "1101", "1110"); -- 0xC0DE
-- Ram Test
type TEST_STATE is
(
    TEST_IDLE,
    TEST_INIT,
    TEST_CONFIG,
    TEST_READ,
    TEST_WRITE,
    TEST_WAIT,
    TEST_DONE
);

signal test_ram_state : TEST_STATE := TEST_IDLE;
-- Test
signal test_ram_timer : std_logic_vector(28 downto 0) := (others => '0');
signal test_ram_ops : std_logic_vector(3 downto 0) := (others => '0');
signal test_ram_flag : std_logic := '0';
--
--
-- Address ---> Row[23:11] : Bank[10:9] : Column[8:0]
-- Data ---> 0x5570
--
-- ADDR(23 downto 11); -- Row address
-- ADDR(10 downto 9); -- Bank address
-- ADDR(8 downto 0); -- Column address
--
signal TEST_RAM_RESET : std_logic := '1';
signal TEST_RAM_ADDR :  std_logic_vector(23 downto 0) := "000000000000000000000000";
signal TEST_RAM_WRITE_EN : std_logic := '0';
signal TEST_RAM_DATA_IN :  std_logic_vector(15 downto 0) := "0101010101110000";
signal TEST_RAM_READ_EN : std_logic := '0';
signal TEST_RAM_DATA_OUT : std_logic_vector(15 downto 0) := (others => '0');
signal TEST_RAM_BUSY : std_logic := '0';
-- PLL
signal CLOCK_133MHz : std_logic := '0';
-- SPI :: nRF905
signal ctrl_RF_CS : std_logic := '0';
signal ctrl_RF_MISO : std_logic := '0';
signal ctrl_RF_MOSI : std_logic := '0';
signal ctrl_RF_SCLK : std_logic := '0';
-- SPI :: BMI160
signal ctrl_BMI160_S1_CS : std_logic := '0';
signal ctrl_BMI160_S1_MISO : std_logic := '0';
signal ctrl_BMI160_S1_MOSI : std_logic := '0';
signal ctrl_BMI160_S1_SCLK : std_logic := '0';
signal ctrl_BMI160_S2_CS : std_logic := '0';
signal ctrl_BMI160_S2_MISO : std_logic := '0';
signal ctrl_BMI160_S2_MOSI : std_logic := '0';
signal ctrl_BMI160_S2_SCLK : std_logic := '0';
-- External Systms
signal ctrl_EXTERNAL_CS : std_logic := '0';
signal ctrl_EXTERNAL_MISO : std_logic := '0';
signal ctrl_EXTERNAL_MOSI : std_logic := '0';
signal ctrl_EXTERNAL_SCLK : std_logic := '0';
-- Sensor ready
signal global_fpga_reset : std_logic := '0';
-- Debounced interrupt signals
signal s1_bmi160_int1_denoised : std_logic := '0';
signal s1_bmi160_int2_denoised : std_logic := '0';
signal s2_bmi160_int1_denoised : std_logic := '0';
signal s2_bmi160_int2_denoised : std_logic := '0';
-- Sensor Data Ready signals
signal s1_bmi160_int_1_DataReady : std_logic := '0';
signal s1_bmi160_int_2_DataReady : std_logic := '0';
signal s2_bmi160_int_1_DataReady : std_logic := '0';
signal s2_bmi160_int_2_DataReady : std_logic := '0';
-- Acceleration S1
signal acceleration_trigger_bmi160_s1 : std_logic := '0';
signal acceleration_offload_ctrl_bmi160_s1 : std_logic_vector(7 downto 0) := (others => '0');
signal acceleration_offload_register_bmi160_s1 : std_logic_vector(7 downto 0) := (others => '0');
signal acceleration_offload_data_bmi160_s1 : std_logic_vector(7 downto 0) := (others => '0');
signal acceleration_offload_wait_spi_s1 : std_logic := '0';
signal acceleration_interrupt_spi_bmi160_s1_burst : std_logic := '0';
signal acceleration_data_spi_bmi160_s1_burst : std_logic_vector(7 downto 0);
signal acceleration_data_spi_bmi160_s1_feedback : std_logic_vector(7 downto 0) := "00010101";
signal acceleration_ctrl_BMI160_S1_CS : std_logic := '0';
signal acceleration_ctrl_BMI160_S1_MISO : std_logic := '0';
signal acceleration_ctrl_BMI160_S1_MOSI : std_logic := '0';
signal acceleration_ctrl_BMI160_S1_SCLK : std_logic := '0';

type PACKET_SWITCH_SM is
(
    PACKET_SWITCH_IDLE,
    PACKET_SWITCH_ACQUISITION,
    PACKET_SWITCH_DONE
);
signal packetSwitchState: PACKET_SWITCH_SM := PACKET_SWITCH_IDLE;

signal offload_debug : std_logic := '0';

signal sheduler_timer_threshold : std_logic := '0';
signal sheduler_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');
signal sheduler_fifo_full : std_logic := '0';
signal sheduler_fifo_empty : std_logic := '0';

signal wr_busy_job : std_logic := '0';
signal rd_busy_job : std_logic := '0';

signal acceleration_offload_six : integer range 0 to 8 := 0;
signal acceleration_offload_pairs : integer range 0 to 256 := 0;
signal acceleration_offload_x : std_logic_vector(15 downto 0) := (others => '0');
signal acceleration_offload_y : std_logic_vector(15 downto 0) := (others => '0');
signal acceleration_offload_z : std_logic_vector(15 downto 0) := (others => '0');
signal acceleration_offload_x_total : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_y_total : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_z_total : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_counter : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_divident_x : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_divident_y : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_divident_z : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_divisor_xyz : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_trigger : std_logic := '0';
signal acceleration_offload_ready : std_logic := '0';
signal acceleration_offload_quotient_x : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_quotient_y : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_quotient_z : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_reminder_x : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_reminder_y : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_reminder_z : std_logic_vector(20 downto 0) := (others => '0');
signal acceleration_offload_division_complete_x : std_logic := '0';
signal acceleration_offload_division_complete_y : std_logic := '0';
signal acceleration_offload_division_complete_z : std_logic := '0';
signal acceleration_offload_dma_trigger : std_logic := '0';

type ACCELERATION_SM is
(
    ACCELERATION_IDLE,
    ACCELERATION_INIT,
    ACCELERATION_DIVIDE,
    ACCELERATION_COMPUTATION,
    ACCELERATION_DATA_1,
    ACCELERATION_DATA_2,
    ACCELERATION_DATA_3,
    ACCELERATION_DATA_4,
    ACCELERATION_DATA_5,
    ACCELERATION_DATA_6,
    ACCELERATION_DATA_READY,
    ACCELERATION_DMA_TRIGGER,
    ACCELERATION_DONE
);
signal accelreation_state: ACCELERATION_SM := ACCELERATION_IDLE;

signal acceleration_average_data_in : std_logic_vector(7 downto 0) := (others => '0');
signal acceleration_average_RD : std_logic := '0';
signal acceleration_average_WR : std_logic := '0';
signal acceleration_average_empty : std_logic := '0';
signal acceleration_average_full : std_logic := '0';
signal acceleration_average_data_out : std_logic_vector(7 downto 0) := (others => '0');

signal spi_external_single_complete : std_logic := '0';
signal spi_external_burst_complete : std_logic := '0';
signal spi_external_burst_data : std_logic_vector(7 downto 0) := (others => '0');
signal spi_external_single_data : std_logic_vector(7 downto 0) := (others => '0');
signal spi_external_offload_wait : std_logic := '0';

----------------------------------------------------------------------------------------------------------------
-- COMPONENTS DECLARATION
----------------------------------------------------------------------------------------------------------------

component DebounceController
generic
(
    PERIOD : integer := 50000; -- 50Mhz :: 50000*20ns = 1ms
    SM_OFFSET : integer := 3
);
port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    BUTTON_IN : in  std_logic;
    BUTTON_OUT : out std_logic
);
end component;

component SpiConverter
Port
(
    CLOCK : in  std_logic;
    RESET : in std_logic;

    CS : in std_logic;
    SCLK : in std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);
    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic;

    CONVERSION_BIT_COUNT : out std_logic_vector(3 downto 0);
    CONVERSION_COMPLETE : out std_logic
);
end component;

component SpiController
generic
(
    SM_OFFSET : integer := 3;
    BYTE_INIT : integer := 10;
    BYTE_BREAK : integer := 50;
    BYTE_EXIT : integer := 10
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;
    -- IN
    OFFLOAD_TRIGGER : in std_logic;
    OFFLOAD_ID : in std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);
    -- SPI
    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;
    -- OUT
    SINGLE_COMPLETE : out std_logic;
    BURST_COMPLETE : out std_logic;
    BURST_DATA : out std_logic_vector(7 downto 0);
    SINGLE_DATA : out std_logic_vector(7 downto 0);
    OFFLOAD_WAIT : out std_logic
);
end component;

component InterruptGenerator
generic
(
    PERIOD_MS : integer := 1000;
    PULSE_LENGTH : integer := 50;
    TRESHOLD_LENGTH : integer := 50000
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    INTERRUPT_SIGNAL : out std_logic;
    THRESHOLD : out std_logic
);
end component;

component I2cController
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    OFFLOAD_TRIGGER : in std_logic;
    KERNEL_INT : in std_logic;
    FPGA_INT : out std_logic;
    FIFO_INT : out std_logic;

    I2C_SCK : inout std_logic;
    I2C_SDA : inout std_logic;

    OFFLOAD_ID : in std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic;
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : out std_logic;
    FEEDBACK_DATA : out std_logic_vector(7 downto 0)
);
end component;

------------------------------------------------------
--
-- Asynchronous Reset
--
-- 8-Bit
-- 256 Deepth
--
------------------------------------------------------
component FifoData
port
(
    aclr : IN STD_LOGIC ;
    clock : IN STD_LOGIC ;
    -- IN
    data : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdreq : IN STD_LOGIC ;
    wrreq : IN STD_LOGIC ;
    -- OUT
    empty : OUT STD_LOGIC ;
    full : OUT STD_LOGIC ;
    q : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
);
end component;

component RamController
Port
(
    CLOCK_133MHz : in  std_logic;
    RESET : in  std_logic;

    -- SDRAM Interface
    A0           : out std_logic; -- Address Bus
    A1           : out std_logic;
    A2           : out std_logic;
    A3           : out std_logic;
    A4           : out std_logic;
    A5           : out std_logic;
    A6           : out std_logic;
    A7           : out std_logic;
    A8           : out std_logic;
    A9           : out std_logic;
    A10          : out std_logic;
    A11          : out std_logic;
    A12          : out std_logic;

    BA0          : out std_logic; -- Bank Address
    BA1          : out std_logic;

    CKE         : out std_logic;
    CS          : out std_logic;
    RAS         : out std_logic;
    CAS         : out std_logic;
    WE          : out std_logic;

    DQ0         : inout std_logic; -- Data Bus
    DQ1         : inout std_logic;
    DQ2         : inout std_logic;
    DQ3         : inout std_logic;
    DQ4         : inout std_logic;
    DQ5         : inout std_logic;
    DQ6         : inout std_logic;
    DQ7         : inout std_logic;
    DQ8         : inout std_logic;
    DQ9         : inout std_logic;
    DQ10        : inout std_logic;
    DQ11        : inout std_logic;
    DQ12        : inout std_logic;
    DQ13        : inout std_logic;
    DQ14        : inout std_logic;
    DQ15        : inout std_logic;

    LDQM        : out std_logic;
    UDQM        : out std_logic;

    -- User Interface
    ADDR        : in  std_logic_vector(23 downto 0);
    DATA_IN     : in  std_logic_vector(15 downto 0);
    DATA_OUT    : out std_logic_vector(15 downto 0);
    READ_EN     : in  std_logic;
    WRITE_EN    : in  std_logic;
    BUSY       : out std_logic
);
end component;

component OffloadController
port
(
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    OFFLOAD_INTERRUPT_EXT : in std_logic;
    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    FIFO_FULL : in std_logic;
    FIFO_EMPTY : in std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(7 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : in std_logic
);
end component;

component DmaOffloadController
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    OFFLOAD_INTERRUPT  : in  std_logic;
    OFFLOAD_BIT_COUNT : in std_logic_vector(3 downto 0);
    OFFLOAD_FIFO_EMPTY  : in  std_logic;

    OFFLOAD_READ_ENABLE : out std_logic;
    OFFLOAD_SECONDARY_DMA_TRIGGER : out  std_logic;

    OFFLOAD_DEBUG : out std_logic
);
end component;

component PwmController
generic
(
    BASE_PERIOD_MS : integer := 20
);
port
(
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    OFFLOAD_TRIGGER : in std_logic;
    FPGA_INT : out std_logic;

    PWM_VECTOR : in std_logic_vector(7 downto 0);

    PWM_SIGNAL : out std_logic
);
end component;

component UartDataTransfer
port
(
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector;

    FPGA_UART_TX : out std_logic;
    FPGA_UART_RX : in std_logic;

    WRITE_BUSY : out std_logic
);
end component;

component UartDataAssembly
port
(
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    UART_LOG_MESSAGE_ID : in UART_LOG_ID;
    UART_LOG_MESSAGE_KEY : in UART_LOG_KEY;
    UART_LOG_MESSAGE_DATA : in UART_LOG_DATA;

    WRITE_ENABLE : out std_logic;
    WRITE_SYMBOL : out std_logic_vector(6 downto 0);

    WRITE_BUSY : in std_logic
);
end component;

component PLL_RamClock
port
(
	areset : in STD_LOGIC  := '0';
	inclk0 : in STD_LOGIC  := '0';
    c0 : out STD_LOGIC ;
    c1 : out STD_LOGIC ;
	locked : out STD_LOGIC
);
end component;

component PulseController
generic
(
    PULSE_LENGTH : integer := 1
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    ENABLE_CONTROLLER : in std_logic;

    INPUT_PULSE : in std_logic;
    OUTPUT_PULSE : out std_logic
);
end component;

component NoiseController
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    INPUT_SIGNAL : in  std_logic;
    THRESHOLD : in  integer range 0 to 255;

    OUTPUT_SIGNAL  : out std_logic
);
end component;

component DelaySynchroniser
generic
(
    SYNCHRONIZATION_DEPTH : integer := 2
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    ASYNC_INPUT : in std_logic;
    SYNC_OUTPUT : out std_logic
);
end component;

component Divider
generic
(
    DEPTH : integer := 16
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    TRIGGER : in std_logic;
    DIVIDENT : in std_logic_vector(DEPTH - 1 downto 0);
    DIVISOR : in std_logic_vector(DEPTH - 1 downto 0);

    QUOTIENT : out std_logic_vector(DEPTH - 1 downto 0);
    REMINDER : out std_logic_vector(DEPTH - 1 downto 0);
    DIVISION_COMPLETE : out std_logic
);
end component;

component DmaTriggerControl
generic
(
    INTERRUPT_LENGTH_US : integer := 100
);
port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    TRIGGER_LONG : in std_logic;
    TRIGGER_SHORT : in std_logic;

    RETURN_INTERRUPT : out std_logic
);
end component;

component InterruptVectorController
port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    PARALLEL_PRIMARY_MOSI : in std_logic_vector(7 downto 0);
    PARALLEL_CONVERSION_COMPLETE : in std_logic;

    VECTOR_INTERRUPT_EXTERNAL_OFFLOAD : out std_logic;
    VECTOR_INTERRUPT_PRIMARY_OFFLOAD : out std_logic;
    VECTOR_INTERRUPT_ENABLE : out std_logic;
    VECTOR_INTERRUPT_START : out std_logic;
    VECTOR_INTERRUPT_TRIGGER : out std_logic;

    FIFO_PARALLEL_PRIMARY_MOSI : out std_logic_vector(7 downto 0);
    FIFO_PARALLEL_PRIMARY_WR_EN : out std_logic
);
end component;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- //
-- // MAIN ROUTINE
-- //
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

begin

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // DEBOUNCE
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

------------------------------------------------
-- BUTTON_1
------------------------------------------------
ActiveDebug_Button: DebounceController
generic map
(
    PERIOD => 50000, -- 50Mhz :: 50000*20ns = 1ms
    SM_OFFSET => 3
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    BUTTON_IN => BUTTON_1,
    BUTTON_OUT => active_button_1
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // DELAY SYNCHRONISERS
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

------------------------------------------------
-- SPI_0
------------------------------------------------
DelaySynchroniser_SPI0_MOSI: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => PRIMARY_MOSI,
    SYNC_OUTPUT => synced_PRIMARY_MOSI
);

DelaySynchroniser_SPI0_SCLK: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => PRIMARY_SCLK,
    SYNC_OUTPUT => synced_PRIMARY_SCLK
);

DelaySynchroniser_SPI0_CS: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => PRIMARY_CS,
    SYNC_OUTPUT => synced_PRIMARY_CS
);

------------------------------------------------
-- SPI_1
------------------------------------------------
DelaySynchroniser_SPI1_MOSI: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => SECONDARY_MOSI,
    SYNC_OUTPUT => synced_SECONDARY_MOSI
);

DelaySynchroniser_SPI1_SCLK: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => SECONDARY_SCLK,
    SYNC_OUTPUT => synced_SECONDARY_SCLK
);

DelaySynchroniser_SPI1_CS: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => SECONDARY_CS,
    SYNC_OUTPUT => synced_SECONDARY_CS
);

------------------------------------------------
-- UART
------------------------------------------------
DelaySynchroniser_UART: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => FPGA_UART_RX,
    SYNC_OUTPUT => synced_FPGA_UART_RX
);

DelaySynchroniser_GPS_UART: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ASYNC_INPUT => GPS_UART_RX,
    SYNC_OUTPUT => synced_GPS_UART_RX
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // NOISE CONTROLERS :: Used to eliminate spike noise from the long pulses :: Required for breadboard + long cables
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

------------------------------------------------
-- INT1_BMI160_S1
------------------------------------------------
s1_int1_NoiseControl: NoiseController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INPUT_SIGNAL => S1_BMI160_INT_1,
    THRESHOLD => 5, -- 50ns

    OUTPUT_SIGNAL => s1_bmi160_int1_denoised
);

------------------------------------------------
-- INT2_BMI160_S1
------------------------------------------------
s1_int2_NoiseControl: NoiseController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INPUT_SIGNAL => S1_BMI160_INT_2,
    THRESHOLD => 5, -- 50ns

    OUTPUT_SIGNAL => s1_bmi160_int2_denoised
);

------------------------------------------------
-- INT1_BMI160_S2
------------------------------------------------
s2_int1_NoiseControl: NoiseController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INPUT_SIGNAL => S2_BMI160_INT_1,
    THRESHOLD => 5, -- 50ns

    OUTPUT_SIGNAL => s2_bmi160_int1_denoised
);

------------------------------------------------
-- INT2_BMI160_S2
------------------------------------------------
s2_int2_NoiseControl: NoiseController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INPUT_SIGNAL => S2_BMI160_INT_2,
    THRESHOLD => 5, -- 50ns

    OUTPUT_SIGNAL => s2_bmi160_int2_denoised
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // PULSE CONTROLLERS :: Used to cut the pulses to ussually -> 20ns
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

------------------------------------------------
-- INT1_BMI160_S1
------------------------------------------------
Int1_from_bmi160_s1: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ENABLE_CONTROLLER => enable_vector_interrupt,

    INPUT_PULSE => s1_bmi160_int1_denoised,
    OUTPUT_PULSE => s1_bmi160_int_1_DataReady
);

------------------------------------------------
-- INT2_BMI160_S1
------------------------------------------------
Int2_from_bmi160_s1: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ENABLE_CONTROLLER => enable_vector_interrupt,

    INPUT_PULSE => s1_bmi160_int2_denoised,
    OUTPUT_PULSE => s1_bmi160_int_2_DataReady
);

------------------------------------------------
-- INT1_BMI160_S2
------------------------------------------------
Int1_from_bmi160_s2: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ENABLE_CONTROLLER => enable_vector_interrupt,

    INPUT_PULSE => s2_bmi160_int1_denoised,
    OUTPUT_PULSE => s2_bmi160_int_1_DataReady
);

------------------------------------------------
-- INT2_BMI160_S2
------------------------------------------------
Int2_from_bmi160_s2: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ENABLE_CONTROLLER => enable_vector_interrupt,

    INPUT_PULSE => s2_bmi160_int2_denoised,
    OUTPUT_PULSE => s2_bmi160_int_2_DataReady
);

------------------------------------------------
-- Global Reset Nuke
------------------------------------------------
FpgaReset_Interrupt_From_CPU: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => '0',

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => RESET_FROM_CPU,
    OUTPUT_PULSE => global_fpga_reset
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // [SPI] Parallelization and Serialization
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

primarySpiConverter_module: SpiConverter port map
(
	CLOCK => CLOCK_50MHz,
    RESET => global_fpga_reset,

	CS => synced_PRIMARY_CS,
	SCLK => synced_PRIMARY_SCLK, -- Kernel Master always initialise SPI transfer

	SERIAL_MOSI => synced_PRIMARY_MOSI, -- in :: Data from Kernel to Serialize
	PARALLEL_MOSI => primary_parallel_MOSI, -- out :: Serialized Data from Kernel to FIFO
	PARALLEL_MISO => "00011000", -- in :: 0x18 Hard coded Feedback to Serialize
	SERIAL_MISO => PRIMARY_MISO, -- out :: 0x18 Serialized Hard coded Feedback to Kernel

    CONVERSION_BIT_COUNT => open,
    CONVERSION_COMPLETE => primary_conversion_complete -- Out :: Data byte is ready [FIFO Write Enable]
);

secondarySpiConverter_module: SpiConverter port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_fpga_reset,

    CS => synced_SECONDARY_CS,
    SCLK => synced_SECONDARY_SCLK, -- Kernel Master always initialise SPI transfer

    SERIAL_MOSI => synced_SECONDARY_MOSI, -- in :: Serialized Feedback from Kernel :: Set in Kernel to 0x81
    PARALLEL_MOSI => open, -- out :: Not in use !
    PARALLEL_MISO => secondary_parallel_MISO, -- in :: Parallel Data from the packet switch
    SERIAL_MISO => SECONDARY_MISO, -- out :: Serialized Data from the packet switch

    CONVERSION_BIT_COUNT => sensor_fifo_bit_count,
    CONVERSION_COMPLETE => open -- out :: Not in use !
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // Interrupts generators and Feedback interrupts
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WatchdogInterrupt: InterruptGenerator
generic map
(
    PERIOD_MS => 100, -- Every 100ms
    PULSE_LENGTH => 50, -- 50 * 20ns = 1us Interrupt Pulse
    TRESHOLD_LENGTH => 2500 -- 2500 * 20ns = 50us Threashold
)
port map
(
	CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

	INTERRUPT_SIGNAL => WDG_INT_FROM_FPGA,
    THRESHOLD => open
);

TimerInterrupt: InterruptGenerator
generic map
(
    PERIOD_MS => 20, -- Every 20ms
    PULSE_LENGTH => 50, -- 50 * 20ns = 1us Interrupt Pulse
    TRESHOLD_LENGTH => 50000 -- 50000 * 20ns = 1ms Threashold
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INTERRUPT_SIGNAL => TIMER_INT_FROM_FPGA,
    THRESHOLD => sheduler_timer_threshold
);

-----------------------------------------------------------------------------------------
--
--
-- DMA Trigger -> Secondary Transfer
--
--
-----------------------------------------------------------------------------------------

FeedbackControl_module: DmaTriggerControl
generic map
(
    INTERRUPT_LENGTH_US => 100
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    TRIGGER_LONG => i2c_complete_long
                    or pwm_m1_complete_long
                    or spi_nRF905_complete_long
                    or spi_bmi160_s1_complete_long
                    or spi_bmi160_s2_complete_long,
    TRIGGER_SHORT => secondary_dma_trigger_gpio_pulse_20ns,
    -- OUT
    RETURN_INTERRUPT => SPI_INT_FROM_FPGA
);

feedback_data_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        --
        -- TODO :: For now
        --
        -- SPI Return multiplexing is only available
        -- If sensor data acquisition is disabled
        --
        if enable_vector_interrupt = '1' then
            secondary_parallel_MISO <= acceleration_average_data_out;
        elsif i2c_complete_long = '1'  then
            secondary_parallel_MISO <= data_i2c_feedback;
        elsif pwm_m1_complete_long = '1' then
            secondary_parallel_MISO <= data_pwm_feedback; -- Constant 0xC3
        elsif spi_bmi160_s1_complete_long = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s1_feedback;
        elsif spi_bmi160_s2_complete_long = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s2_feedback;
        elsif spi_nRF905_complete_long = '1' then
            secondary_parallel_MISO <= data_spi_rf_feedback;
        end if;

    end if;
end process;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // Interrupt Vector Controller
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InterruptVectorController_module: InterruptVectorController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    PARALLEL_PRIMARY_MOSI => primary_parallel_MOSI,
    PARALLEL_CONVERSION_COMPLETE => primary_conversion_complete,
    -- OUT
    VECTOR_INTERRUPT_EXTERNAL_OFFLOAD => external_offload_vector_interrupt,
    VECTOR_INTERRUPT_PRIMARY_OFFLOAD => primary_offload_vector_interrupt,
    VECTOR_INTERRUPT_ENABLE => enable_vector_interrupt,
    VECTOR_INTERRUPT_START => start_vector_interrupt,
    VECTOR_INTERRUPT_TRIGGER => trigger_vector_interrupt,
    -- OUT
    FIFO_PARALLEL_PRIMARY_MOSI => FIFO_primary_parallel_MOSI,
    FIFO_PARALLEL_PRIMARY_WR_EN => FIFO_primary_fifo_wr_en
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // FIFO Data Flow and Offload Control
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

------------------------------------------------------
-- FIFO
-- 8-Bit
-- 256 Deepth
------------------------------------------------------
PrimarySpiData_Fifo: FifoData
port map
(
    aclr  => global_fpga_reset,
    clock => CLOCK_50MHz,
    -- IN
    data  => FIFO_primary_parallel_MOSI,
    rdreq => primary_fifo_rd_en,
    wrreq => FIFO_primary_fifo_wr_en,
    -- OUT
    empty => primary_fifo_empty,
    full  => primary_fifo_full,
    q     => primary_fifo_data_out
);

OffloadController_primary: OffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    OFFLOAD_INTERRUPT_EXT => external_offload_vector_interrupt,
    OFFLOAD_INTERRUPT => primary_offload_vector_interrupt,
    FIFO_DATA => primary_fifo_data_out,
    FIFO_READ_ENABLE => primary_fifo_rd_en,

    FIFO_FULL => primary_fifo_full,
    FIFO_EMPTY => primary_fifo_empty,
    -- OUT
    OFFLOAD_READY => primary_offload_ready,
    OFFLOAD_ID => primary_offload_id,
    OFFLOAD_CTRL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,

    OFFLOAD_WAIT => primary_offload_wait
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // SDRAM Controller
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PLL_RamClock_module: PLL_RamClock
port map
(
    areset => '0',
    inclk0 => CLOCK_50MHz,
    c0 => CLOCK_133MHz,
    c1 => CLK_SDRAM, -- 133MHz Shifted by 180°
    locked => open
);

RamController_module: RamController
port map
(
    CLOCK_133MHz => CLOCK_133MHz,
    RESET => TEST_RAM_RESET,

    A0 => A0,
    A1 => A1,
    A2 => A2,
    A3 => A3,
    A4 => A4,
    A5 => A5,
    A6 => A6,
    A7 => A7,
    A8 => A8,
    A9 => A9,
    A10 => A10,
    A11 => A11,
    A12 => A12,

    BA0 => BA0,
    BA1 => BA1,

    CKE => CKE,
    CS => CS,
    RAS => RAS,
    CAS => CAS,
    WE => WE,

    DQ0 => D0,
    DQ1 => D1,
    DQ2 => D2,
    DQ3 => D3,
    DQ4 => D4,
    DQ5 => D5,
    DQ6 => D6,
    DQ7 => D7,
    DQ8 => D8,
    DQ9 => D9,
    DQ10 => D10,
    DQ11 => D11,
    DQ12 => D12,
    DQ13 => D13,
    DQ14 => D14,
    DQ15 => D15,

    LDQM => LDQM,
    UDQM => UDQM,

    -- User Interface :: TODO ??? Timing Issue
    ADDR => TEST_RAM_ADDR,
    DATA_IN => TEST_RAM_DATA_IN,
    DATA_OUT => TEST_RAM_DATA_OUT,
    READ_EN => TEST_RAM_READ_EN,
    WRITE_EN => TEST_RAM_WRITE_EN,
    BUSY => TEST_RAM_BUSY
);

--RamControl_test_process:
--process (CLOCK_133MHz, test_ram_state)
--begin
--    if rising_edge(CLOCK_133MHz) then

--        case (test_ram_state) is

--            when TEST_IDLE =>
--                if test_ram_timer = "10111110101111000001111111111" then
--                    test_ram_state <= TEST_INIT;
--                elsif test_ram_timer = "10111110101111000001111111111" - "1011111010111100000111111111" then
--                    TEST_RAM_RESET <= '0';
--                    test_ram_timer <= test_ram_timer + '1';
--                else
--                    test_ram_timer <= test_ram_timer + '1';
--                end if;

--            when TEST_INIT =>
--                test_ram_state <= TEST_WRITE;

--            when TEST_CONFIG =>
--                TEST_RAM_READ_EN <= '0';
--                TEST_RAM_WRITE_EN <= '0';
--                if TEST_RAM_BUSY = '0' then
--                    if test_ram_flag = '0' then
--                        if test_ram_ops = "1001" then
--                            test_ram_ops <= "0000";
--                            test_ram_flag <= '1';
--                            test_ram_state <= TEST_CONFIG;
--                            TEST_RAM_ADDR <= "000000000000000000000000";
--                        else
--                            test_ram_ops <= test_ram_ops + '1';
--                            test_ram_state <= TEST_WRITE;
--                        end if;
--                    elsif test_ram_flag = '1' then
--                        if test_ram_ops = "1010" then
--                            test_ram_ops <= "0000";
--                            test_ram_flag <= '0';
--                            test_ram_state <= TEST_DONE;
--                        else
--                            test_ram_ops <= test_ram_ops + '1';
--                            test_ram_state <= TEST_READ;
--                        end if;
--                    end if;
--                end if;

--            when TEST_WRITE =>
--                TEST_RAM_WRITE_EN <= '1';
--                TEST_RAM_DATA_IN <= TEST_RAM_DATA_IN + '1';
--                TEST_RAM_ADDR <= TEST_RAM_ADDR + '1';
--                test_ram_state <= TEST_WAIT;

--            when TEST_READ =>
--                TEST_RAM_READ_EN <= '1';
--                TEST_RAM_ADDR <= TEST_RAM_ADDR + '1';
--                test_ram_state <= TEST_WAIT;

--            when TEST_WAIT =>
--                test_ram_state <= TEST_CONFIG;

--            when TEST_DONE =>
--                test_ram_state <= TEST_DONE;

--        end case;
--    end if;
--end process;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                   //
-- //                   //
-- // [UART] Controller //
-- //                   //
-- //                   //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UartDataTransfer_module: UartDataTransfer
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    FPGA_UART_TX => FPGA_UART_TX,
    FPGA_UART_RX => synced_FPGA_UART_RX,

    WRITE_BUSY => uart_write_busy
);

--FPGA_UART_TX <=  synced_GPS_UART_RX;
--GPS_UART_TX <= synced_FPGA_UART_RX;

UartDataAssembly_module: UartDataAssembly
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    UART_LOG_MESSAGE_ID => UART_LOG_MESSAGE_ID,
    UART_LOG_MESSAGE_KEY => UART_LOG_MESSAGE_KEY,
    UART_LOG_MESSAGE_DATA => UART_LOG_MESSAGE_DATA,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    WRITE_BUSY => uart_write_busy
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                               //
-- //                               //
-- // [PACKED] Interface Controller //
-- //                               //
-- //                               //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PacketSwitch:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then

        case packetSwitchState is
            --------------------------------------------------------------------------------
            -- OFFLOAD_ID :: 7-bits
            --------------------------------------------------------------------------------
            -- |0123|456| :: Other way around
            --------------------------------------------------------------------------------
            -- |1000|100| :: BMI160_S1 :: 0x11
            -- |0100|100| :: BMI160_S2 :: 0x12
            --------------------------------------------------------------------------------
            when PACKET_SWITCH_IDLE =>
                if primary_offload_ready = '1' then
                    if primary_offload_ctrl(2 downto 1) = CTRL_I2C then
                        trigger_i2c <= '1';
                    elsif primary_offload_ctrl(2 downto 1) = CTRL_SPI then
                        if primary_offload_id = ID_BMI160_S1 then
                            trigger_bmi160_s1 <= '1';
                        elsif primary_offload_id = ID_BMI160_S2 then
                            trigger_bmi160_s2 <= '1';
                        elsif primary_offload_id = ID_EXTERNAL_SPI then
                            trigger_external_spi <= '1';
                        elsif primary_offload_id = ID_nRF905 then
                            trigger_nRF905 <= '1';
                        else
                            acceleration_trigger_bmi160_s1 <= '0';
                            trigger_bmi160_s1 <= '0';
                            trigger_bmi160_s2 <= '0';
                            trigger_external_spi <= '0';
                            trigger_nRF905 <= '0';
                        end if;
                    elsif primary_offload_ctrl(2 downto 1) = CTRL_PWM then
                        trigger_pwm_m1 <= '1';
                    end if;
                    packetSwitchState <= PACKET_SWITCH_DONE;
                elsif s1_bmi160_int_1_DataReady = '1' then
                    if start_vector_interrupt = '1' then
                        packetSwitchState <= PACKET_SWITCH_ACQUISITION;
                    end if;
                end if;

            --------------------------------------------------------------------------------
            -- OFFLOAD_CTRL :: 8-bits
            --------------------------------------------------------------------------------
            --  Dma config (Auto/Manual Config)
            --      |
            --      |        Device (I2C, SPI, PWM)
            --      |          ID
            --      |          ||
            --      |          ||
            --      V          VV
            --    | x | xxxx | xx | x | <<<---- OFFLOAD_CTRL : std_logic_vector(7 downto 0)
            --          ΛΛΛΛ        Λ
            --          ||||        |
            --          ||||        |
            --          ||||        |
            --       burst size    R/W (I2C, SPI)
            --       (I2C, SPI)
            --------------------------------------------------------------------------------
            when PACKET_SWITCH_ACQUISITION =>
                acceleration_trigger_bmi160_s1 <= '1';                 --  N/A   12    NA   R
                acceleration_offload_ctrl_bmi160_s1     <= "00110000"; -- | 0 | 1100 | 01 | 0 |
                acceleration_offload_register_bmi160_s1 <= "10010010"; -- 0x92 :: TODO -> Add 0x10 in case of Read !!!
                acceleration_offload_data_bmi160_s1     <= "00000000"; -- 0x00
                packetSwitchState <= PACKET_SWITCH_DONE;

            when PACKET_SWITCH_DONE =>
                trigger_i2c <= '0';
                trigger_pwm_m1 <= '0';
                acceleration_trigger_bmi160_s1 <= '0';
                trigger_bmi160_s1 <= '0';
                trigger_bmi160_s2 <= '0';
                trigger_external_spi <= '0';
                trigger_nRF905 <= '0';
                packetSwitchState <= PACKET_SWITCH_IDLE;

            when others =>
                packetSwitchState <= PACKET_SWITCH_IDLE;

        end case;

    end if;
end process;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                  //
-- //                  //
-- // [SENSOR] Primary //
-- //                  //
-- //                  //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-------------------------------------------------
-- Burst read is required
-- In order to read 6 Bytes
-- Starting from 0x32 ending
-- At 0x37 for x, y and z
-- Total Time: 67.5 + 135 = 202.5µs
-------------------------------------------------
i2c_Bus_primary: I2cController port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- in
    OFFLOAD_TRIGGER => trigger_i2c, -- i2c transfer ready to begin
    -- in
    KERNEL_INT => '0',
    -- out
    FPGA_INT => i2c_complete_long, -- SM is ready for SPI.1 transfer :: 1000*20ns interrupt
    FIFO_INT => open, -- TODO :: Store output data in secondary FIFO

    I2C_SCK => I2C_SCK,
    I2C_SDA => I2C_SDA,
    -- in
    OFFLOAD_ID => primary_offload_id, -- Device ID :: BMI160@0x69=1001011
    OFFLOAD_REGISTER => primary_offload_register, -- Device Register
    OFFLOAD_CONTROL => primary_offload_ctrl(0), -- For now :: Read/Write
    OFFLOAD_DATA => primary_offload_data, -- Write Data
    -- out
    OFFLOAD_WAIT => primary_offload_wait_i2c, -- Wait between consecutive i2c transfers
    FEEDBACK_DATA => data_i2c_feedback
);

BMI160_S1_primary: SpiController
generic map
(
    SM_OFFSET => 0,
    BYTE_INIT => 10,
    BYTE_BREAK => 50,
    BYTE_EXIT => 10
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_bmi160_s1,
    OFFLOAD_ID => (others => '0'), -- Not necessary for SPI
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_BMI160_S1_CS,
    CTRL_MISO => ctrl_BMI160_S1_MISO,
    CTRL_MOSI => ctrl_BMI160_S1_MOSI,
    CTRL_SCK => ctrl_BMI160_S1_SCLK,
    -- OUT
    SINGLE_COMPLETE => spi_bmi160_s1_complete_long,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_bmi160_s1_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_s1
);

BMI160_S2_primary: SpiController
generic map
(
    SM_OFFSET => 0,
    BYTE_INIT => 10,
    BYTE_BREAK => 50,
    BYTE_EXIT => 10
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_bmi160_s2,
    OFFLOAD_ID => (others => '0'), -- Not necessary for SPI
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_BMI160_S2_CS,
    CTRL_MISO => ctrl_BMI160_S2_MISO,
    CTRL_MOSI => ctrl_BMI160_S2_MOSI,
    CTRL_SCK => ctrl_BMI160_S2_SCLK,
    -- OUT
    SINGLE_COMPLETE => spi_bmi160_s2_complete_long,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_bmi160_s2_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_s2
);

EXTERNAL_SPI_primary: SpiController
generic map
(
    SM_OFFSET => 3,
    BYTE_INIT => 0,
    BYTE_BREAK => 0,
    BYTE_EXIT => 0
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_external_spi or trigger_vector_interrupt,
    OFFLOAD_ID => (others => '0'), -- Not necessary for SPI
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_EXTERNAL_CS,
    CTRL_MISO => ctrl_EXTERNAL_MISO,
    CTRL_MOSI => ctrl_EXTERNAL_MOSI,
    CTRL_SCK => ctrl_EXTERNAL_SCLK,
    -- OUT
    SINGLE_COMPLETE => spi_external_single_complete,
    BURST_COMPLETE => spi_external_burst_complete,
    BURST_DATA => spi_external_burst_data,
    SINGLE_DATA => spi_external_single_data,
    OFFLOAD_WAIT => spi_external_offload_wait
);

RF905_primary: SpiController
generic map
(
    SM_OFFSET => 0,
    BYTE_INIT => 10,
    BYTE_BREAK => 50,
    BYTE_EXIT => 10
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_nRF905,
    OFFLOAD_ID => (others => '0'), -- Not necessary for SPI
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_RF_CS,
    CTRL_MISO => ctrl_RF_MISO,
    CTRL_MOSI => ctrl_RF_MOSI,
    CTRL_SCK => ctrl_RF_SCLK,
    -- OUT
    SINGLE_COMPLETE => spi_nRF905_complete_long,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_rf_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_rf
);

primary_offload_wait <= primary_offload_wait_i2c or primary_offload_wait_spi_s1 or primary_offload_wait_spi_s2 or primary_offload_wait_spi_rf;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                      //
-- // [SENSOR] Acquisition //
-- //                      //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-------------------------------------------------------------
-- 0. Interrupt from sensor
-- 1. NoiseController
-- 2. PulseController
-- 3. PacketSwitch
-- 4. SpiController
-------------------------------------------------------------
BMI160_S1_acceleration: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => acceleration_trigger_bmi160_s1,
    OFFLOAD_ID => (others => '0'), -- Not necessary for SPI
    OFFLOAD_CONTROL => acceleration_offload_ctrl_bmi160_s1,
    OFFLOAD_REGISTER => acceleration_offload_register_bmi160_s1,
    OFFLOAD_DATA => acceleration_offload_data_bmi160_s1,
    -- SPI
    CTRL_CS => acceleration_ctrl_BMI160_S1_CS,
    CTRL_MISO => acceleration_ctrl_BMI160_S1_MISO,
    CTRL_MOSI => acceleration_ctrl_BMI160_S1_MOSI,
    CTRL_SCK => acceleration_ctrl_BMI160_S1_SCLK,
    -- OUT
    SINGLE_COMPLETE => open,
    BURST_COMPLETE => acceleration_interrupt_spi_bmi160_s1_burst, -- Every byte @ Burst
    BURST_DATA => acceleration_data_spi_bmi160_s1_burst,
    SINGLE_DATA => acceleration_data_spi_bmi160_s1_feedback,
    OFFLOAD_WAIT => acceleration_offload_wait_spi_s1 -- TODO :: Need wait to process individual Bytes
);
----------------------------------------------------------------------------------------------------------------------------
-- A5. FIFO Write & Read Control :: Checked against WAIT & BUSY conditions
----------------------------------------------------------------------------------------------------------------------------
read_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if rd_busy_job = '0' then
            --
            -- Write only when we are not reading
            --
            wr_busy_job <= acceleration_interrupt_spi_bmi160_s1_burst;
            --
            -- Start Reading when last busy complete
            -- And when threshold activates Reading process
            --
            if acceleration_offload_wait_spi_s1 = '0' and sheduler_timer_threshold = '1' then
                rd_busy_job <= '1';
            end if;
        elsif rd_busy_job = '1' then
            --
            -- Stop writing when we reading
            --
            wr_busy_job <= '0';
            if sheduler_timer_threshold = '0' then
                --
                -- Stop reading when threashold go down
                --
                rd_busy_job <= '0';
            end if;
        end if;
    end if;
end process;
-------------------------------------------------------------
-- A6. Write and read bursted data :: Using above conditions
-------------------------------------------------------------
InstantaneousSensorData_Fifo: FifoData
port map
(
    aclr  => global_fpga_reset,
    clock => CLOCK_50MHz,
    -- IN
    data  => acceleration_data_spi_bmi160_s1_burst,
    rdreq => rd_busy_job,
    wrreq => wr_busy_job,
    -- OUT
    empty => sheduler_fifo_empty,
    full  => sheduler_fifo_full,
    q     => sheduler_fifo_data_out
);
-------------------------------------------------------------
-- A7. State Machine
--
-- a. Addition of xyz components from FIFO
-- b. Setup xyz Dividers
-- c. Divide and wait for Results
-- d. Store Results in the Secondary DMA Fifo
-- e. Trigger DMA Offload Controller
-------------------------------------------------------------
acceleration_offset_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if sheduler_fifo_empty = '0' then
            if rd_busy_job = '1' then
                acceleration_offload_counter <= acceleration_offload_counter + '1';
                acceleration_offload_six <= acceleration_offload_six + 1;

                if acceleration_offload_six = 1 then
                    acceleration_offload_z_total <= acceleration_offload_z_total + acceleration_offload_z;
                    acceleration_offload_x(7 downto 0) <= sheduler_fifo_data_out;

                elsif acceleration_offload_six = 2 then
                    acceleration_offload_x(15 downto 8) <= sheduler_fifo_data_out;

                elsif acceleration_offload_six = 3 then
                    acceleration_offload_x_total <= acceleration_offload_x_total + acceleration_offload_x;
                    acceleration_offload_y(7 downto 0) <= sheduler_fifo_data_out;

                elsif acceleration_offload_six = 4 then
                    acceleration_offload_y(15 downto 8) <= sheduler_fifo_data_out;

                elsif acceleration_offload_six = 5 then
                    acceleration_offload_y_total <= acceleration_offload_y_total + acceleration_offload_y;
                    acceleration_offload_z(7 downto 0) <= sheduler_fifo_data_out;

                elsif acceleration_offload_six = 6 then
                    acceleration_offload_six <= 1;
                    acceleration_offload_pairs <= acceleration_offload_pairs + 1;
                    acceleration_offload_z(15 downto 8) <= sheduler_fifo_data_out;
                end if;

                if acceleration_offload_pairs = 0 then
                    acceleration_offload_pairs <= 1;
                end if;

            end if;
        elsif sheduler_fifo_empty = '1' then
            acceleration_offload_six <= 0;
            acceleration_offload_pairs <= 0;
            acceleration_offload_counter <= (others => '0');
        end if;

        case accelreation_state is
            when ACCELERATION_IDLE =>
                if rd_busy_job = '1' and sheduler_fifo_empty = '1' then
                    if acceleration_offload_counter /= 0 and acceleration_offload_pairs /= 0 then
                        -- Last z sample !!
                        acceleration_offload_z_total <= acceleration_offload_z_total + acceleration_offload_z;
                        acceleration_offload_x(7 downto 0) <= sheduler_fifo_data_out;
                        acceleration_offload_divisor_xyz <= std_logic_vector(to_unsigned(acceleration_offload_pairs, 21));
                        acceleration_offload_ready <= '1';
                        accelreation_state <= ACCELERATION_INIT;
                    end if;
                end if;

            when ACCELERATION_INIT =>
                acceleration_offload_ready <= '0';
                acceleration_offload_divident_x <= acceleration_offload_x_total;
                acceleration_offload_divident_y <= acceleration_offload_y_total;
                acceleration_offload_divident_z <= acceleration_offload_z_total;
                acceleration_offload_x <= (others => '0');
                acceleration_offload_y <= (others => '0');
                acceleration_offload_z <= (others => '0');
                acceleration_offload_x_total <= (others => '0');
                acceleration_offload_y_total <= (others => '0');
                acceleration_offload_z_total <= (others => '0');
                accelreation_state <= ACCELERATION_DIVIDE;

            when ACCELERATION_DIVIDE =>
                acceleration_offload_trigger <= '1';
                accelreation_state <= ACCELERATION_COMPUTATION;

            when ACCELERATION_COMPUTATION =>
                acceleration_offload_trigger <= '0';
                if acceleration_offload_division_complete_x = '1'
                and acceleration_offload_division_complete_y = '1'
                and  acceleration_offload_division_complete_z = '1' then
                    accelreation_state <= ACCELERATION_DATA_1;
                end if;

            when ACCELERATION_DATA_1 =>
                if acceleration_offload_reminder_x(4) = '1' then
                    acceleration_average_data_in <= acceleration_offload_quotient_x(7 downto 0) + '1';
                else
                    acceleration_average_data_in <= acceleration_offload_quotient_x(7 downto 0);
                end if;
                acceleration_average_WR <= '1';
                accelreation_state <= ACCELERATION_DATA_2;

            when ACCELERATION_DATA_2 =>
                acceleration_average_data_in <= acceleration_offload_quotient_x(15 downto 8);
                accelreation_state <= ACCELERATION_DATA_3;

            when ACCELERATION_DATA_3 =>
                if acceleration_offload_reminder_y(4) = '1' then
                    acceleration_average_data_in <= acceleration_offload_quotient_y(7 downto 0) + '1';
                else
                    acceleration_average_data_in <= acceleration_offload_quotient_y(7 downto 0);
                end if;
                accelreation_state <= ACCELERATION_DATA_4;

            when ACCELERATION_DATA_4 =>
                acceleration_average_data_in <= acceleration_offload_quotient_y(15 downto 8);
                accelreation_state <= ACCELERATION_DATA_5;

            when ACCELERATION_DATA_5 =>
                if acceleration_offload_reminder_z(4) = '1' then
                    acceleration_average_data_in <= acceleration_offload_quotient_z(7 downto 0) + '1';
                else
                    acceleration_average_data_in <= acceleration_offload_quotient_z(7 downto 0);
                end if;
                accelreation_state <= ACCELERATION_DATA_6;

            when ACCELERATION_DATA_6 =>
                acceleration_average_data_in <= acceleration_offload_quotient_z(15 downto 8);
                accelreation_state <= ACCELERATION_DATA_READY;

            when ACCELERATION_DATA_READY =>
                acceleration_average_WR <= '0';
                accelreation_state <= ACCELERATION_DMA_TRIGGER;

            when ACCELERATION_DMA_TRIGGER =>
                acceleration_offload_dma_trigger <= '1';
                accelreation_state <= ACCELERATION_DONE;

            when ACCELERATION_DONE =>
                acceleration_offload_dma_trigger <= '0';
                accelreation_state <= ACCELERATION_IDLE;

            when others =>
                accelreation_state <= ACCELERATION_IDLE;
        end case;

    end if;
end process;
-------------------------------------------------------------
-- A8. Division Algorithm
-------------------------------------------------------------
Acceleration_Divider_S1_x: Divider
generic map
(
    DEPTH => 21
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    TRIGGER => acceleration_offload_trigger,
    DIVIDENT => acceleration_offload_divident_x,
    DIVISOR => acceleration_offload_divisor_xyz,

    QUOTIENT => acceleration_offload_quotient_x,
    REMINDER => acceleration_offload_reminder_x,
    DIVISION_COMPLETE => acceleration_offload_division_complete_x
);

Acceleration_Divider_S1_y: Divider
generic map
(
    DEPTH => 21
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    TRIGGER => acceleration_offload_trigger,
    DIVIDENT => acceleration_offload_divident_y,
    DIVISOR => acceleration_offload_divisor_xyz,

    QUOTIENT => acceleration_offload_quotient_y,
    REMINDER => acceleration_offload_reminder_y,
    DIVISION_COMPLETE => acceleration_offload_division_complete_y
);

Acceleration_Divider_S1_z: Divider
generic map
(
    DEPTH => 21
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    TRIGGER => acceleration_offload_trigger,
    DIVIDENT => acceleration_offload_divident_z,
    DIVISOR => acceleration_offload_divisor_xyz,

    QUOTIENT => acceleration_offload_quotient_z,
    REMINDER => acceleration_offload_reminder_z,
    DIVISION_COMPLETE => acceleration_offload_division_complete_z
);
-------------------------------------------------------------
-- A9. Secondary DMA/SPI Fifo
-------------------------------------------------------------
AverageSensorData_Fifo: FifoData
port map
(
    aclr  => global_fpga_reset,
    clock => CLOCK_50MHz,
    -- IN
    data  => acceleration_average_data_in,
    rdreq => acceleration_average_RD, -- Must be offloaded
    wrreq => acceleration_average_WR,
    -- OUT
    empty => acceleration_average_empty,
    full  => acceleration_average_full,
    q     => acceleration_average_data_out
);
--------------------------------------------------------------------------
-- A10. Secondary DMA Offload Controller
--------------------------------------------------------------------------
MainSecondary_DmaOffloadController: DmaOffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- In
    OFFLOAD_INTERRUPT => acceleration_offload_dma_trigger,
    OFFLOAD_BIT_COUNT => sensor_fifo_bit_count, -- When BIT(8)
    OFFLOAD_FIFO_EMPTY => acceleration_average_empty,
    -- Out
    OFFLOAD_READ_ENABLE => acceleration_average_RD,
    OFFLOAD_SECONDARY_DMA_TRIGGER => secondary_dma_trigger_gpio_pulse_20ns,

    OFFLOAD_DEBUG => offload_debug
);

--------------------------------------------------------------------------
--
-- nRF905
--
--------------------------------------------------------------------------
NRF905_CSN <= ctrl_RF_CS;
ctrl_RF_MISO <= NRF905_MISO;
NRF905_MOSI <= ctrl_RF_MOSI;
NRF905_SCK <= ctrl_RF_SCLK;

NRF905_PWR_UP <= '1';
NRF905_TRX_CE <= '0';
NRF905_TX_EN <= 'Z';

--------------------------------------------------------------------------
--
-- BMI160
--
-- SCL :: SCK
-- SDA :: MOSI
-- CS  :: CS
-- SAO :: MISO
--
--------------------------------------------------------------------------

process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if enable_vector_interrupt = '1' then
            S1_BMI160_CS <= acceleration_ctrl_BMI160_S1_CS;
            acceleration_ctrl_BMI160_S1_MISO <= S1_BMI160_MISO;
            S1_BMI160_MOSI <= acceleration_ctrl_BMI160_S1_MOSI;
            S1_BMI160_SCLK <= acceleration_ctrl_BMI160_S1_SCLK;
        else
            S1_BMI160_CS <= ctrl_BMI160_S1_CS;
            ctrl_BMI160_S1_MISO <= S1_BMI160_MISO;
            S1_BMI160_MOSI <= ctrl_BMI160_S1_MOSI;
            S1_BMI160_SCLK <= ctrl_BMI160_S1_SCLK;
        end if;
    end if;
end process;

S2_BMI160_CS <= ctrl_BMI160_S2_CS;
ctrl_BMI160_S2_MISO <= S2_BMI160_MISO;
S2_BMI160_MOSI <= ctrl_BMI160_S2_MOSI;
S2_BMI160_SCLK <= ctrl_BMI160_S2_SCLK;

EXTERNAL_CS <= ctrl_EXTERNAL_CS;
ctrl_EXTERNAL_MISO <= EXTERNAL_MISO;
EXTERNAL_MOSI <= ctrl_EXTERNAL_MOSI;
EXTERNAL_SCLK <= ctrl_EXTERNAL_SCLK;

---------------------------------------------------------------
-- TODO :: Need Refactoring and Parametrization !!!
--
-- Hex range 0x00 ---> 0xFA
-- Dec range 0 ---> 250
--
-- Minimum pulse width ---> 1ms
-- Offset = 50000 * 20ns(clock tick) = 1ms
--
-- PWM Width = offset + vector*200
--
-- For 01100100'b = 64'h = 100'd
-- width = 50000 + 100*200 = 70000
-- 70000*10^-9 = 1.4ms
--
-- For 11111010'b = FA'h = 250'd
-- width = 50000 + 250*200 = 100000
-- 100000*10^-9 = 2ms
---------------------------------------------------------------
PwmController_module: PwmController
generic map
(
    BASE_PERIOD_MS => 20  -- 20ms Base Period
)
port map
(
    -- IN
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    OFFLOAD_TRIGGER => trigger_pwm_m1,
    FPGA_INT => pwm_m1_complete_long,

    PWM_VECTOR => primary_offload_data,
    -- OUT
    PWM_SIGNAL => PWM_SIGNAL
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //         //
-- //         //
-- // [DEBUG] //
-- //         //
-- //         //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

--looptrough_spi_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        NRF905_CSN <= synced_PRIMARY_CS;
--        PRIMARY_MISO <= NRF905_MISO;
--        NRF905_MOSI <= synced_PRIMARY_MOSI;
--        NRF905_SCK <= synced_PRIMARY_SCLK;
--    end if;
--end process;

--looptrough_spi_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        S1_BMI160_CS <= synced_PRIMARY_CS;
--        PRIMARY_MISO <= S1_BMI160_MISO;
--        S1_BMI160_MOSI <= synced_PRIMARY_MOSI;
--        S1_BMI160_SCLK <= synced_PRIMARY_SCLK;
--    end if;
--end process;

--LOGIC_CH1 <= '0';
--LOGIC_CH2 <= '0';
--LOGIC_CH3 <= '0';
--LOGIC_CH4 <= '0';
--LOGIC_CH5 <= '0';
--LOGIC_CH6 <= '0';
--LOGIC_CH7 <= '0';

------------------------------------------------------------------------------------------------------------------------------------------
--
--
-- NOTES
--
--
------------------------------------------------------------------------------------------------------------------------------------------
--
-- 0xFFFF (16-bit)
--
--     Binary: 1111 1111 1111 1111
--     Inverting: 0000 0000 0000 0000
--     Adding 1: 0000 0000 0000 0001
--     Decimal: 1, but since it was negative, it's -1
--
-- 0xFF0E (16-bit)
--
--     Binary: 1111 1111 0000 1110
--     Inverting: 0000 0000 1111 0001
--     Adding 1: 0000 0000 1111 0010
--     Decimal: 242, but negative → -242
--
-- 0x0057 (16-bit, MSB = 0, so it's positive)
--
--     Decimal: 87
--
------------------------------------------------------------------------------------------------------------------------------------------

LED_1 <= '0';
LED_2 <= '1';
LED_3 <= '1';
LED_4 <= '1';
LED_5 <= '1';
LED_6 <= '1';
LED_7 <= '1';
LED_8 <= '1';

end rtl;
