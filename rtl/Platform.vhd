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
    NRF905_CSN : out std_logic     -- PIN_P1

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
-- Interrupt Vector Counter + signals
signal REG_primary_fifo_wr_en : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_0 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_1 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_2 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_3 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_4 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_5 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_6 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_7 : std_logic_vector(2047 downto 0) := (others => '0');
-- Interrupt Vector stage signals
signal FIFO_primary_fifo_wr_en  : std_logic := '0';
signal FIFO_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
signal STAGE_1_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
signal STAGE_2_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
-- Interrupt Vector out signals
signal interrupt_vector : std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_vector_busy : std_logic := '0';
signal interrupt_vector_enable : std_logic := '0';
-- Interrupt vector state machine
type VECTOR_TYPE is
(
    VECTOR_IDLE,
    VECTOR_RESERVED,            -- "0000"
    VECTOR_OFFLOAD_PRIMARY,     -- "0001"
    VECTOR_ENABLE,              -- "0010"
    VECTOR_DISABLE,             -- "0011"
    VECTOR_START,               -- "0100"
    VECTOR_STOP,                -- "0101"
    VECTOR_OFFLOAD_SECONDARY,   -- "0110"
    VECTOR_FAST,                -- "0111"
    VECTOR_SLOW,                -- "1000"
    VECTOR_UNUSED_09,           -- "1001"
    VECTOR_UNUSED_10,           -- "1010"
    VECTOR_UNUSED_11,           -- "1011"
    VECTOR_UNUSED_12,           -- "1100"
    VECTOR_UNUSED_13,           -- "1101"
    VECTOR_UNUSED_14,           -- "1110"
    VECTOR_UNUSED_15,           -- "1111"
    VECTOR_DONE
);
signal vector_state: VECTOR_TYPE := VECTOR_IDLE;
-- Interrupt vector interrupts
signal primary_offload_vector_interrupt : std_logic := '0';
signal enable_vector_interrupt : std_logic := '0';
signal start_vector_interrupt : std_logic := '0';
signal speed_vector_interrupt : std_logic := '0';
signal secondary_offload_vector_interrupt : std_logic := '0';
signal return_vector_extension : std_logic := '0';
signal secondary_dma_trigger_gpio_pulse : std_logic := '0';
-- Interrupt Help
signal data_vector_run : std_logic := '0';
signal data_vector_count : std_logic_vector(3 downto 0) := (others => '0');
-- Interrupt Vector signals
signal primary_conversion_run : std_logic := '0';
signal primary_conversion_reset : integer range 0 to 2048 := 0;
signal primary_conversion_count : integer range 0 to 256 := 0;
-- Fifo
signal primary_fifo_rd_en : std_logic := '0';
signal primary_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');
signal primary_fifo_full : std_logic := '0';
signal primary_fifo_empty : std_logic := '0';
-- Fifo
signal sensor_fifo_rdreq : std_logic := '0';
signal sensor_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');
signal sensor_fifo_full : std_logic := '0';
signal sensor_fifo_empty : std_logic := '0';
signal sensor_fifo_bit_count : std_logic_vector(3 downto 0) := (others => '0');
-- Primary Offload
signal primary_offload_ready : std_logic := '0';
signal primary_offload_id : std_logic_vector(6 downto 0) := (others => '0');
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
signal trigger_nRF905 : std_logic := '0';
signal trigger_pwm_m1 : std_logic := '0';
-- CTRL Mux
constant CTRL_I2C : std_logic_vector(1 downto 0) := "00";
constant CTRL_SPI : std_logic_vector(1 downto 0) := "01";
constant CTRL_PWM : std_logic_vector(1 downto 0) := "10";
-- ID Mux
constant ID_BMI160_S1 : std_logic_vector(6 downto 0) := "1000100"; -- "0010001"; -- Must be upside down :: Due to primary_offload_id for i2c
constant ID_BMI160_S2 : std_logic_vector(6 downto 0) := "0100100"; -- "0010010"; -- Must be upside down :: Due to primary_offload_id for i2c
constant ID_nRF905 : std_logic_vector(6 downto 0) := "0010100"; -- Must be upside down :: Same as upside down
-- Feedback interrupts
signal single_complete_i2c : std_logic := '0';
signal single_complete_nRF905 : std_logic := '0';
signal single_complete_bmi160_s1 : std_logic := '0';
signal single_complete_bmi160_s2 : std_logic := '0';
signal single_complete_pwm_m1 : std_logic := '0';
-- Feedback Pulse & Burst for S1
signal interrupt_spi_bmi160_s1_pulse : std_logic := '0';
-- Feedback Pulse & Burst for S2 + Data
signal interrupt_spi_bmi160_s2_pulse : std_logic := '0';
-- Feedback data
signal data_i2c_feedback : std_logic_vector(7 downto 0) := (others => '0');
signal data_spi_rf_feedback : std_logic_vector(7 downto 0) := "00010001";
signal data_spi_bmi160_s1_feedback : std_logic_vector(7 downto 0) := "00010101";
signal data_spi_bmi160_s2_feedback : std_logic_vector(7 downto 0) := "00010110";
signal data_pwm_feedback : std_logic_vector(7 downto 0) := "11000011";
-- Interrupts
signal feedback_interrupt_timer : std_logic_vector(12 downto 0) := (others => '0');
-- UART
signal uart_write_enable : std_logic := '0';
signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_write_busy : std_logic := '0';
-- UART Test Log
signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("1101", "1110"); -- 0xDE
signal UART_LOG_MESSAGE_KEY : UART_LOG_KEY := ("1010", "1101"); -- 0xAD
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("1100", "0000", "1101", "1110"); -- 0xC0DE
-- Test
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
signal test_timer : std_logic_vector(28 downto 0) := (others => '0');
signal test_ops : std_logic_vector(3 downto 0) := (others => '0');
signal test_flag : std_logic := '0';
--
--
-- Address ---> Row[23:11] : Bank[10:9] : Column[8:0]
-- Data ---> 0x5570
--
-- ADDR(23 downto 11); -- Row address
-- ADDR(10 downto 9); -- Bank address
-- ADDR(8 downto 0); -- Column address
--
signal TEST_RESET : std_logic := '1';
signal TEST_ADDR :  std_logic_vector(23 downto 0) := "000000000000000000000000";
signal TEST_WRITE_EN : std_logic := '0';
signal TEST_DATA_IN :  std_logic_vector(15 downto 0) := "0101010101110000";
signal TEST_READ_EN : std_logic := '0';
signal TEST_DATA_OUT : std_logic_vector(15 downto 0) := (others => '0');
signal TEST_BUSY : std_logic := '0';
-- PLL
signal CLOCk_133MHz : std_logic := '0';
signal CLOCK_Fast : std_logic := '0';
-- SPI Controller
signal spi_mux : std_logic_vector(3 downto 0) := "0000";
signal ctrl_CS : std_logic := '0';
signal ctrl_MISO : std_logic := '0';
signal ctrl_MOSI : std_logic := '0';
signal ctrl_SCLK : std_logic := '0';
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
-- Debug
signal acquisition_trigger_bmi160_s1 : std_logic := '0';
signal acquisition_offload_ctrl : std_logic_vector(7 downto 0) := (others => '0');
signal acquisition_offload_register : std_logic_vector(7 downto 0) := (others => '0');
signal acquisition_offload_data : std_logic_vector(7 downto 0) := (others => '0');
signal acquisition_offload_wait_spi_s1 : std_logic := '0';

signal acquisition_interrupt_spi_bmi160_s1_feedback : std_logic := '0';
signal acquisition_interrupt_spi_bmi160_s1_feedback_short : std_logic := '0';
signal acquisition_interrupt_spi_bmi160_s1_burst : std_logic := '0';
signal acquisition_data_spi_bmi160_s1_burst : std_logic_vector(7 downto 0);
signal acquisition_data_spi_bmi160_s1_feedback : std_logic_vector(7 downto 0) := "00010101";
signal acquisition_ctrl_BMI160_S1_CS : std_logic := '0';
signal acquisition_ctrl_BMI160_S1_MISO : std_logic := '0';
signal acquisition_ctrl_BMI160_S1_MOSI : std_logic := '0';
signal acquisition_ctrl_BMI160_S1_SCLK : std_logic := '0';

type SENSOR_STATE is
(
    SENSOR_IDLE,
    SENSOR_ACQUISITION,
    SENSOR_DONE
);
signal s1_state: SENSOR_STATE := SENSOR_IDLE;

-- Debug
signal led_6_toggle : std_logic := '1';
signal led_7_toggle : std_logic := '1';
signal led_8_toggle : std_logic := '1';

signal feedbck_interrupt_logic : std_logic := '0';
signal offload_debug : std_logic := '0';

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
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;
    -- IN
    OFFLOAD_TRIGGER : in std_logic;
    OFFLOAD_ID : in std_logic_vector(6 downto 0);
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
    PULSE_LENGTH : integer := 50
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    INTERRUPT_SIGNAL : out std_logic
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

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
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
    -- OUT
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

    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    FIFO_FULL : in std_logic;
    FIFO_EMPTY : in std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(6 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : in std_logic
);
end component;

component SensorFifo_OffloadController
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    OFFLOAD_DELAY_SWITCH  : in  std_logic;

    OFFLOAD_IRQ_VECTOR  : in  std_logic;
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
    PULSE_LENGTH => 50 -- 50 * 20ns = 1us Interrupt Pulse
)
port map
(
	CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

	INTERRUPT_SIGNAL => WDG_INT_FROM_FPGA
);

TimerInterrupt: InterruptGenerator
generic map
(
    PERIOD_MS => 20, -- Every 20ms
    PULSE_LENGTH => 50 -- 50 * 20ns = 1us Interrupt Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    INTERRUPT_SIGNAL => TIMER_INT_FROM_FPGA
);

feedback_interrupts_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then

        if secondary_dma_trigger_gpio_pulse = '1' then -- TODO :: Extension
            return_vector_extension <= '1';
        end if;

        if single_complete_i2c = '1'
        or single_complete_pwm_m1 = '1'
        or single_complete_bmi160_s1 = '1'
        or single_complete_bmi160_s2 = '1'
        or single_complete_nRF905 = '1'
        or return_vector_extension = '1' -- TODO :: Extension
        then
            if feedback_interrupt_timer = "1001110001000" then -- 5000 * 20 = 100us interrupt pulse back to CPU
                SPI_INT_FROM_FPGA <= '0';
                feedbck_interrupt_logic <= '0';
                return_vector_extension <= '0';
            else
                SPI_INT_FROM_FPGA <= '1';
                feedbck_interrupt_logic <= '1';
                feedback_interrupt_timer <= feedback_interrupt_timer + '1';
            end if;
        else
            SPI_INT_FROM_FPGA <= '0';
            feedbck_interrupt_logic <= '0';
            feedback_interrupt_timer <= (others => '0'); -- Reseting timer here :: When FPGA_INT goto '0'
        end if;
    end if;
end process;

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
            secondary_parallel_MISO <= sensor_fifo_data_out;
        elsif single_complete_i2c = '1'  then
            secondary_parallel_MISO <= data_i2c_feedback;
        elsif single_complete_pwm_m1 = '1' then
            secondary_parallel_MISO <= data_pwm_feedback;
        elsif single_complete_bmi160_s1 = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s1_feedback;
        elsif single_complete_bmi160_s2 = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s2_feedback;
        elsif single_complete_nRF905 = '1' then
            secondary_parallel_MISO <= data_spi_rf_feedback;
        end if;
    end if;
end process;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //
-- // Interrupt Vector
-- //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-----------------------------------
--
-- TODO
--
-- Need to be put
-- Into the module
--
-----------------------------------
interrupt_vector_process:
process(CLOCK_50MHz, global_fpga_reset)
begin
    if global_fpga_reset = '1' then
        primary_offload_vector_interrupt <= '0';
        secondary_offload_vector_interrupt <= '0';
        vector_state <= VECTOR_IDLE;
        enable_vector_interrupt <= '0';
        start_vector_interrupt <= '0';
        led_7_toggle <= '1';
        led_8_toggle <= '1';
    elsif rising_edge(CLOCK_50MHz) then

        REG_primary_fifo_wr_en <= REG_primary_fifo_wr_en(2046 downto 0) & primary_conversion_complete;
        REG_primary_parallel_MOSI_7 <= REG_primary_parallel_MOSI_0(2046 downto 0) & primary_parallel_MOSI(7);
        REG_primary_parallel_MOSI_6 <= REG_primary_parallel_MOSI_1(2046 downto 0) & primary_parallel_MOSI(6);
        REG_primary_parallel_MOSI_5 <= REG_primary_parallel_MOSI_2(2046 downto 0) & primary_parallel_MOSI(5);
        REG_primary_parallel_MOSI_4 <= REG_primary_parallel_MOSI_3(2046 downto 0) & primary_parallel_MOSI(4);
        REG_primary_parallel_MOSI_3 <= REG_primary_parallel_MOSI_4(2046 downto 0) & primary_parallel_MOSI(3);
        REG_primary_parallel_MOSI_2 <= REG_primary_parallel_MOSI_5(2046 downto 0) & primary_parallel_MOSI(2);
        REG_primary_parallel_MOSI_1 <= REG_primary_parallel_MOSI_6(2046 downto 0) & primary_parallel_MOSI(1);
        REG_primary_parallel_MOSI_0 <= REG_primary_parallel_MOSI_7(2046 downto 0) & primary_parallel_MOSI(0);

        if primary_conversion_complete = '1' or REG_primary_fifo_wr_en(2047) = '1' then
            if primary_conversion_count < 2 then
                STAGE_2_primary_parallel_MOSI <= primary_parallel_MOSI;
                STAGE_1_primary_parallel_MOSI <= STAGE_2_primary_parallel_MOSI;
            end if;
            primary_conversion_reset <= 0;
            primary_conversion_run <= '1';
            primary_conversion_count <= primary_conversion_count + 1;
        else
            if primary_conversion_reset = 2000 then
                interrupt_vector_busy <= '0';
                primary_conversion_run <= '0';
                primary_conversion_reset <= 0;
                primary_conversion_count <= 0;
                STAGE_2_primary_parallel_MOSI <= (others => '0');
                STAGE_1_primary_parallel_MOSI <= (others => '0');
            else
                primary_conversion_reset <= primary_conversion_reset + 1;
            end if;
        end if;

        if STAGE_1_primary_parallel_MOSI(7) =  '1'  ---------------------------------
        and STAGE_1_primary_parallel_MOSI(2) =  '1' ----===[ IRQ Vector Base ]===----
        and STAGE_1_primary_parallel_MOSI(1) =  '1' ---------------------------------
        and STAGE_2_primary_parallel_MOSI = "10101111" -- [Vector, 0xAF, 0xAE, 0xAD]
        and interrupt_vector_busy = '0'
        then
            interrupt_vector <= STAGE_1_primary_parallel_MOSI(6 downto 3);
            interrupt_vector_enable <= '1';
            interrupt_vector_busy <= '1';
        else
            interrupt_vector <= "0000";
            interrupt_vector_enable <= '0';
        end if;

        if interrupt_vector_busy = '1' then
            FIFO_primary_parallel_MOSI <= (others => '0');
            FIFO_primary_fifo_wr_en <= '0';
        else
            FIFO_primary_parallel_MOSI <= REG_primary_parallel_MOSI_0(2047) & REG_primary_parallel_MOSI_1(2047) &
                                            REG_primary_parallel_MOSI_2(2047) & REG_primary_parallel_MOSI_3(2047) &
                                            REG_primary_parallel_MOSI_4(2047) & REG_primary_parallel_MOSI_5(2047) &
                                            REG_primary_parallel_MOSI_6(2047) & REG_primary_parallel_MOSI_7(2047);

            FIFO_primary_fifo_wr_en <= REG_primary_fifo_wr_en(2047);
        end if;

        case vector_state is

            when VECTOR_IDLE =>
                if interrupt_vector = "0001" then
                    vector_state <= VECTOR_OFFLOAD_PRIMARY;
                elsif interrupt_vector = "0010" then
                    vector_state <= VECTOR_ENABLE;
                elsif interrupt_vector = "0011" then
                    vector_state <= VECTOR_DISABLE;
                elsif interrupt_vector = "0100" then
                    vector_state <= VECTOR_START;
                elsif interrupt_vector = "0101" then
                    vector_state <= VECTOR_STOP;
                elsif interrupt_vector = "0110" then
                    vector_state <= VECTOR_OFFLOAD_SECONDARY;
                elsif interrupt_vector = "0111" then
                    vector_state <= VECTOR_FAST;
                elsif interrupt_vector = "1000" then
                    vector_state <= VECTOR_SLOW;
                elsif interrupt_vector = "1001" then
                    vector_state <= VECTOR_UNUSED_09;
                elsif interrupt_vector = "1010" then
                    vector_state <= VECTOR_UNUSED_10;
                elsif interrupt_vector = "1011" then
                    vector_state <= VECTOR_UNUSED_11;
                elsif interrupt_vector = "1100" then
                    vector_state <= VECTOR_UNUSED_12;
                elsif interrupt_vector = "1101" then
                    vector_state <= VECTOR_UNUSED_13;
                elsif interrupt_vector = "1110" then
                    vector_state <= VECTOR_UNUSED_14;
                elsif interrupt_vector = "1111" then
                    vector_state <= VECTOR_UNUSED_15;
                end if;

            when VECTOR_RESERVED =>
                vector_state <= VECTOR_DONE;
            when VECTOR_OFFLOAD_PRIMARY =>
                primary_offload_vector_interrupt <= '1';
                vector_state <= VECTOR_DONE;
            when VECTOR_ENABLE =>
                enable_vector_interrupt <= '1';
                led_8_toggle <= '0';
                vector_state <= VECTOR_DONE;
            when VECTOR_DISABLE =>
                enable_vector_interrupt <= '0';
                led_8_toggle <= '1';
                vector_state <= VECTOR_DONE;
            when VECTOR_START =>
                start_vector_interrupt <= '1';
                led_7_toggle <= '0';
                vector_state <= VECTOR_DONE;
            when VECTOR_STOP =>
                start_vector_interrupt <= '0';
                led_7_toggle <= '1';
                vector_state <= VECTOR_DONE;
            when VECTOR_OFFLOAD_SECONDARY =>
                secondary_offload_vector_interrupt <= '1';
                vector_state <= VECTOR_DONE;
            when VECTOR_FAST =>
                speed_vector_interrupt <= '1';
                vector_state <= VECTOR_DONE;
            when VECTOR_SLOW =>
                speed_vector_interrupt <= '0';
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_09 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_10 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_11 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_12 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_13 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_14 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_UNUSED_15 =>
                vector_state <= VECTOR_DONE;
            when VECTOR_DONE =>
                primary_offload_vector_interrupt <= '0';
                secondary_offload_vector_interrupt <= '0';
                vector_state <= VECTOR_IDLE;
            when others =>
                vector_state <= VECTOR_IDLE;
        end case;

    end if;
end process;

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
    -- OUT
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
    RESET => TEST_RESET,

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
    ADDR => TEST_ADDR,
    DATA_IN => TEST_DATA_IN,
    DATA_OUT => TEST_DATA_OUT,
    READ_EN => TEST_READ_EN,
    WRITE_EN => TEST_WRITE_EN,
    BUSY => TEST_BUSY
);

--process (CLOCK_133MHz, test_ram_state)
--begin
--    if rising_edge(CLOCK_133MHz) then

--        case (test_ram_state) is

--            when TEST_IDLE =>
--                if test_timer = "10111110101111000001111111111" then
--                    test_ram_state <= TEST_INIT;
--                elsif test_timer = "10111110101111000001111111111" - "1011111010111100000111111111" then
--                    TEST_RESET <= '0';
--                    test_timer <= test_timer + '1';
--                else
--                    test_timer <= test_timer + '1';
--                end if;

--            when TEST_INIT =>
--                test_ram_state <= TEST_WRITE;

--            when TEST_CONFIG =>
--                TEST_READ_EN <= '0';
--                TEST_WRITE_EN <= '0';
--                if TEST_BUSY = '0' then
--                    if test_flag = '0' then
--                        if test_ops = "1001" then
--                            test_ops <= "0000";
--                            test_flag <= '1';
--                            test_ram_state <= TEST_CONFIG;
--                            TEST_ADDR <= "000000000000000000000000";
--                        else
--                            test_ops <= test_ops + '1';
--                            test_ram_state <= TEST_WRITE;
--                        end if;
--                    elsif test_flag = '1' then
--                        if test_ops = "1010" then
--                            test_ops <= "0000";
--                            test_flag <= '0';
--                            test_ram_state <= TEST_DONE;
--                        else
--                            test_ops <= test_ops + '1';
--                            test_ram_state <= TEST_READ;
--                        end if;
--                    end if;
--                end if;

--            when TEST_WRITE =>
--                TEST_WRITE_EN <= '1';
--                TEST_DATA_IN <= TEST_DATA_IN + '1';
--                TEST_ADDR <= TEST_ADDR + '1';
--                test_ram_state <= TEST_WAIT;

--            when TEST_READ =>
--                TEST_READ_EN <= '1';
--                TEST_ADDR <= TEST_ADDR + '1';
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

        case s1_state is
            --------------------------------------------------------------------------------
            -- OFFLOAD_ID :: 7-bits
            --------------------------------------------------------------------------------
            -- |0123|456| :: Other way around
            --------------------------------------------------------------------------------
            -- |1000|100| :: BMI160_S1 :: 0x11
            -- |0100|100| :: BMI160_S2 :: 0x12
            --------------------------------------------------------------------------------
            when SENSOR_IDLE =>
                if primary_offload_ready = '1' then
                    if primary_offload_ctrl(2 downto 1) = CTRL_I2C then
                        trigger_i2c <= '1';
                    elsif primary_offload_ctrl(2 downto 1) = CTRL_SPI then
                        if primary_offload_id = ID_BMI160_S1 then
                            trigger_bmi160_s1 <= '1';
                        elsif primary_offload_id = ID_BMI160_S2 then
                            trigger_bmi160_s2 <= '1';
                        elsif primary_offload_id = ID_nRF905 then
                            trigger_nRF905 <= '1';
                        else
                            acquisition_trigger_bmi160_s1 <= '0';
                            trigger_bmi160_s1 <= '0';
                            trigger_bmi160_s2 <= '0';
                            trigger_nRF905 <= '0';
                        end if;
                    elsif primary_offload_ctrl(2 downto 1) = CTRL_PWM then
                        trigger_pwm_m1 <= '1';
                    end if;
                    s1_state <= SENSOR_DONE;
                elsif s1_bmi160_int_1_DataReady = '1' then
                    if start_vector_interrupt = '1' then
                        s1_state <= SENSOR_ACQUISITION;
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
            when SENSOR_ACQUISITION =>
                acquisition_trigger_bmi160_s1 <= '1';         -- N/A   12    NA   R
                acquisition_offload_ctrl           <= "00110000"; -- | 0 | 1100 | 01 | 0 |
                acquisition_offload_register       <= "10010010"; -- 0x92 :: TODO -> Add 0x10 in case of Read !!!
                acquisition_offload_data           <= "00000000"; -- 0x00
                s1_state <= SENSOR_DONE;

            when SENSOR_DONE =>
                trigger_i2c <= '0';
                trigger_pwm_m1 <= '0';
                acquisition_trigger_bmi160_s1 <= '0';
                trigger_bmi160_s1 <= '0';
                trigger_bmi160_s2 <= '0';
                trigger_nRF905 <= '0';
                s1_state <= SENSOR_IDLE;

            when others =>
                s1_state <= SENSOR_IDLE;

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
    FPGA_INT => single_complete_i2c, -- SM is ready for SPI.1 transfer :: 1000*20ns interrupt
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

BMI160_S1_primary: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_bmi160_s1,
    OFFLOAD_ID => "0000000",
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_BMI160_S1_CS,
    CTRL_MISO => ctrl_BMI160_S1_MISO,
    CTRL_MOSI => ctrl_BMI160_S1_MOSI,
    CTRL_SCK => ctrl_BMI160_S1_SCLK,
    -- OUT
    SINGLE_COMPLETE => single_complete_bmi160_s1,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_bmi160_s1_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_s1
);

BMI160_S2_primary: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_bmi160_s2,
    OFFLOAD_ID => "0000000",
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_BMI160_S2_CS,
    CTRL_MISO => ctrl_BMI160_S2_MISO,
    CTRL_MOSI => ctrl_BMI160_S2_MOSI,
    CTRL_SCK => ctrl_BMI160_S2_SCLK,
    -- OUT
    SINGLE_COMPLETE => single_complete_bmi160_s2,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_bmi160_s2_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_s2
);

RF905_primary: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => trigger_nRF905,
    OFFLOAD_ID => "0000000",
    OFFLOAD_CONTROL => primary_offload_ctrl,
    OFFLOAD_REGISTER => primary_offload_register,
    OFFLOAD_DATA => primary_offload_data,
    -- SPI
    CTRL_CS => ctrl_RF_CS,
    CTRL_MISO => ctrl_RF_MISO,
    CTRL_MOSI => ctrl_RF_MOSI,
    CTRL_SCK => ctrl_RF_SCLK,
    -- OUT
    SINGLE_COMPLETE => single_complete_nRF905,
    BURST_COMPLETE => open,
    BURST_DATA => open,
    SINGLE_DATA => data_spi_rf_feedback,
    OFFLOAD_WAIT => primary_offload_wait_spi_rf
);

primary_offload_wait <= primary_offload_wait_i2c or primary_offload_wait_spi_s1 or primary_offload_wait_spi_s2 or primary_offload_wait_spi_rf;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                      //
-- //                      //
-- // [SENSOR] Acquisition //
-- //                      //
-- //                      //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BMI160_S1_acquisition: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- IN
    OFFLOAD_TRIGGER => acquisition_trigger_bmi160_s1,
    OFFLOAD_ID => "0000000",
    OFFLOAD_CONTROL => acquisition_offload_ctrl,
    OFFLOAD_REGISTER => acquisition_offload_register,
    OFFLOAD_DATA => acquisition_offload_data,
    -- SPI
    CTRL_CS => acquisition_ctrl_BMI160_S1_CS,
    CTRL_MISO => acquisition_ctrl_BMI160_S1_MISO,
    CTRL_MOSI => acquisition_ctrl_BMI160_S1_MOSI,
    CTRL_SCK => acquisition_ctrl_BMI160_S1_SCLK,
    -- OUT
    SINGLE_COMPLETE => acquisition_interrupt_spi_bmi160_s1_feedback,
    BURST_COMPLETE => acquisition_interrupt_spi_bmi160_s1_burst,
    BURST_DATA => acquisition_data_spi_bmi160_s1_burst,
    SINGLE_DATA => acquisition_data_spi_bmi160_s1_feedback,
    OFFLOAD_WAIT => acquisition_offload_wait_spi_s1 -- TODO :: Need wait to process individual Bytes
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                    //
-- //                    //
-- // [FIFO] Sensor Data //
-- //                    //
-- //                    //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

offload_FifoData:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if secondary_offload_vector_interrupt = '1' and data_vector_run <= '0' then
            data_vector_count <= "0000";
            data_vector_run <= '1';
        elsif data_vector_run = '1' and data_vector_count < "1011" then
            data_vector_count <= data_vector_count + '1';
        else
            data_vector_run <= '0';
        end if;
    end if;
end process;

SensorData_Fifo: FifoData
port map
(
    aclr  => global_fpga_reset,
    clock => CLOCK_50MHz,
    -- IN
    data  => acquisition_data_spi_bmi160_s1_burst,
    -- OUT
    rdreq => data_vector_run or sensor_fifo_rdreq,
    wrreq => acquisition_interrupt_spi_bmi160_s1_burst,
    -- OUT
    empty => sensor_fifo_empty,
    full  => sensor_fifo_full,
    q     => sensor_fifo_data_out
);

Accqusition_pulse: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,

    ENABLE_CONTROLLER => enable_vector_interrupt,

    INPUT_PULSE => acquisition_interrupt_spi_bmi160_s1_feedback,
    OUTPUT_PULSE => acquisition_interrupt_spi_bmi160_s1_feedback_short
);


OffloadController_secondary: SensorFifo_OffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => global_fpga_reset,
    -- In
    OFFLOAD_DELAY_SWITCH => speed_vector_interrupt,
    -- In
    OFFLOAD_IRQ_VECTOR => acquisition_interrupt_spi_bmi160_s1_feedback_short,
    OFFLOAD_BIT_COUNT => sensor_fifo_bit_count,
    OFFLOAD_FIFO_EMPTY => sensor_fifo_empty,
    -- Out
    OFFLOAD_READ_ENABLE => sensor_fifo_rdreq,
    OFFLOAD_SECONDARY_DMA_TRIGGER => secondary_dma_trigger_gpio_pulse,

    OFFLOAD_DEBUG => offload_debug
);

-------------------------------------
--
-- nRF905
--
-------------------------------------
NRF905_CSN <= ctrl_RF_CS;
ctrl_RF_MISO <= NRF905_MISO;
NRF905_MOSI <= ctrl_RF_MOSI;
NRF905_SCK <= ctrl_RF_SCLK;

NRF905_PWR_UP <= '1';
NRF905_TRX_CE <= '0';
NRF905_TX_EN <= 'Z';

-------------------------------------
--
-- BMI160
--
-- SCL :: SCK
-- SDA :: MOSI
-- CS  :: CS
-- SAO :: MISO
--
-------------------------------------

process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if enable_vector_interrupt = '1' then
            S1_BMI160_CS <= acquisition_ctrl_BMI160_S1_CS;
            acquisition_ctrl_BMI160_S1_MISO <= S1_BMI160_MISO;
            S1_BMI160_MOSI <= acquisition_ctrl_BMI160_S1_MOSI;
            S1_BMI160_SCLK <= acquisition_ctrl_BMI160_S1_SCLK;
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
    FPGA_INT => single_complete_pwm_m1,

    PWM_VECTOR => primary_offload_data,
    -- OUT
    PWM_SIGNAL => PWM_SIGNAL
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                          //
-- //                          //
-- // [SENSOR] Data Processing //
-- //                          //
-- //                          //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




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

--LOGIC_CH1 <= acquisition_ctrl_BMI160_S1_SCLK;
--LOGIC_CH2 <= SECONDARY_SCLK;
--LOGIC_CH3 <= SECONDARY_MOSI;
--LOGIC_CH4 <= feedbck_interrupt_logic;
--LOGIC_CH5 <= offload_debug;
--LOGIC_CH6 <= sensor_fifo_empty;
--LOGIC_CH7 <= sensor_fifo_full;

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
LED_6 <= led_6_toggle;
LED_7 <= led_7_toggle;
LED_8 <= led_8_toggle;

end rtl;
