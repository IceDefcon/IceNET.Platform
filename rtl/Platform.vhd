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
-- PIN_A19 :: PIN_B19   | TIMER_INT_FROM_FPGA   ::                  | H7  :: H8
-- PIN_A18 :: PIN_B18   |                       ::                  | H9  :: H10
-- PIN_A17 :: PIN_B17   |                       ::                  | H11 :: H12
-- PIN_A16 :: PIN_B16   | SECONDARY_SCLK        ::                  | H13 :: H14
-- PIN_A15 :: PIN_B15   | SPI_INT_FROM_CPU      ::                  | H15 :: H16
-- PIN_A14 :: PIN_B14   | ----===[ 3V3 ]===---- :: SECONDARY_CS     | H17 :: H18
-- PIN_A13 :: PIN_B13   | PRIMARY_MOSI          ::                  | H19 :: H20
-- PIN_A10 :: PIN_B10   | PRIMARY_MISO          :: SECONDARY_MISO   | H21 :: H22
-- PIN_A9  :: PIN_B9    | PRIMARY_SCLK          :: PRIMARY_CS       | H23 :: H24
-- PIN_A8  :: PIN_B8    |                       ::                  | H25 :: H26
-- PIN_A7  :: PIN_B7    |                       ::                  | H27 :: H28
-- PIN_A6  :: PIN_B6    | SPI_INT_FROM_FPGA     ::                  | H29 :: H30
-- PIN_A5  :: PIN_B5    | WDG_INT_FROM_FPGA     ::                  | H31 :: H32
-- PIN_C3  :: PIN_C4    | CFG_INT_FROM_CPU      ::                  | H33 :: H34
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
-- PIN_M19  :: NOTUSED_53                   | PIN_M20  :: NOTUSED_54
-- PIN_N19  :: NOTUSED_51                   | PIN_N20  :: NOTUSED_52
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
-- PIN_AB14 :: I2C_ADXL345_INT_2            | PIN_AA14 :: S1_BMI160_MOSI
-- PIN_AB13 :: I2C_ADXL345_INT_1            | PIN_AA13 :: S1_BMI160_SCLK
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

    -----------------------------------------------------------------------------
    -- Kernel Communication
    -----------------------------------------------------------------------------
    SPI_INT_FROM_CPU : in std_logic; -- PIN_A5 :: GPIO12 :: HEADER_PIN_15
    SPI_INT_FROM_FPGA : out std_logic; -- PIN_A9 :: GPIO01 :: HEADER_PIN_29
    TIMER_INT_FROM_FPGA : out std_logic; -- PIN_A13 :: GPIO09 :: HEADER_PIN_07
    WDG_INT_FROM_FPGA : out std_logic; -- PIN_A20 :: GPIO11 :: HEADER_PIN_31
    CFG_INT_FROM_CPU : in std_logic; -- PIN_B4 :: GPIO13 :: HEADER_PIN_33
    PRIMARY_MOSI : in std_logic;  -- PIN_B6 :: H19 :: SPI0_MOSI
    PRIMARY_MISO : out std_logic; -- PIN_A8 :: H21 :: SPI0_MISO
    PRIMARY_SCLK : in std_logic;  -- PIN_B8 :: H23 :: SPI0_SCLK
    PRIMARY_CS : in std_logic;    -- PIN_A6 :: H24 :: SPI0_CS0
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
    --ADXL_INT1 : in std_logic; -- PIN_AB14
    --ADXL_INT2 : in std_logic; -- PIN_AA14
    -- UART
    UART_x86_TX : out std_logic; -- PIN_N19 :: FTDI Rx
    UART_x86_RX : in std_logic;  -- PIN_M19 :: FTDI Tx
    -- I2C Bus
    I2C_SDA : inout std_logic; -- PIN_Y21
    I2C_SCK : inout std_logic; -- PIN_Y22
    I2C_ADXL345_INT_1 : in std_logic; -- PIN_AB13
    I2C_ADXL345_INT_2 : in std_logic; -- PIN_AB14
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

-- SCK
-- MOSI
-- CS
-- MISO

----------------------------------------------------------------------------------------------------------------
-- Signals
----------------------------------------------------------------------------------------------------------------

-- Buttons
signal reset_button : std_logic := '0';
-- Spi.0 Primary
signal primary_conversion_complete : std_logic := '0';
signal primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
-- Spi.1 Secondary
signal secondary_parallel_MISO : std_logic_vector(7 downto 0) := (others => '0');
-- BMI160 Gyroscope registers
signal mag_z_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_z_7_0 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_y_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_y_7_0 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_x_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_x_7_0 : std_logic_vector(7 downto 0):= (others => '0');
-- FIFO
constant FIFO_WIDTH : integer := 8;
constant FIFO_DEPTH : integer := 32;
signal primary_fifo_data_in : std_logic_vector(7 downto 0) := (others => '0');
signal primary_fifo_wr_en : std_logic := '0';
signal primary_fifo_rd_en : std_logic := '0';
signal primary_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');
signal primary_fifo_full : std_logic := '0';
signal primary_fifo_empty : std_logic := '0';
-- Kernel interrupt
signal kernel_interrupt : std_logic := '0';
signal kernel_interrupt_stop : std_logic := '0';
-- Offload
signal offload_interrupt : std_logic := '0';
signal offload_ready : std_logic := '0';
signal offload_id : std_logic_vector(6 downto 0) := (others => '0');
signal offload_register : std_logic_vector(7 downto 0) := (others => '0');
signal offload_ctrl : std_logic_vector(7 downto 0) := (others => '0');
signal offload_data : std_logic_vector(7 downto 0) := (others => '0');
signal offload_wait : std_logic := '0';
signal offload_wait_i2c : std_logic := '0';
signal offload_wait_spi_s1 : std_logic := '0';
signal offload_wait_spi_s2 : std_logic := '0';
signal offload_wait_spi_rf : std_logic := '0';
-- PacketSwitch
signal switch_i2c_ready : std_logic := '0';
signal switch_bmi160_s1_ready : std_logic := '0';
signal switch_bmi160_s2_ready : std_logic := '0';
signal switch_bmi160_s3_ready : std_logic := '0';
signal switch_spi_RF_ready : std_logic := '0';
signal switch_pwm_ready : std_logic := '0';
constant SWITCH_I2C : std_logic_vector(1 downto 0) := "00";
constant SWITCH_SPI : std_logic_vector(1 downto 0) := "01";
constant SWITCH_PWM : std_logic_vector(1 downto 0) := "10";
-- SPI Mux
constant SWITCH_BMI160_S1 : std_logic_vector(6 downto 0) := "1000100"; -- "0010001"; -- Must be upside down :: Due to offload_id for i2c
constant SWITCH_BMI160_S2 : std_logic_vector(6 downto 0) := "0100100"; -- "0010010"; -- Must be upside down :: Due to offload_id for i2c
constant SWITCH_BMI160_S3 : std_logic_vector(6 downto 0) := "1100100"; -- "0010011"; -- Must be upside down :: Due to offload_id for i2c
constant SWITCH_nRF905 : std_logic_vector(6 downto 0) := "0010100"; -- Must be upside down :: Same as upside down
-- Feedback interrupts
signal interrupt_i2c_feedback : std_logic := '0';
signal interrupt_spi_rf_feedback : std_logic := '0';
signal interrupt_spi_bmi160_s1_feedback : std_logic := '0';
signal interrupt_spi_bmi160_s2_feedback : std_logic := '0';
signal interrupt_spi_bmi160_s3_feedback : std_logic := '0';
signal interrupt_pwm_feedback : std_logic := '0';
-- Feedback data
signal data_i2c_feedback : std_logic_vector(7 downto 0) := (others => '0');
signal data_spi_rf_feedback : std_logic_vector(7 downto 0) := "00010001";
signal data_spi_bmi160_s1_feedback : std_logic_vector(7 downto 0) := "00010101";
signal data_spi_bmi160_s2_feedback : std_logic_vector(7 downto 0) := "00010110";
signal data_spi_bmi160_s3_feedback : std_logic_vector(7 downto 0) := "00010111";
signal data_pwm_feedback : std_logic_vector(7 downto 0) := "11000011";
-- Debounce signals
signal interrupt_from_cpu : std_logic := '0';
-- Interrupts
signal feedback_interrupt_timer : std_logic_vector(5 downto 0) := (others => '0');
-- UART
signal uart_write_enable : std_logic := '0';
signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_write_busy : std_logic := '0';
-- UART Test Log
signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("0101", "1100"); -- 0x5C
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("0111", "1010", "0001", "1011"); -- 0x7A1B
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
    clock : in  std_logic;
    button_in : in  std_logic;
    button_out : out std_logic
);
end component;

component SpiConverter
Port
(
    CLOCK : in  std_logic;

    CS : in std_logic;
    SCLK : in std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);
    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic;

    CONVERSION_COMPLETE : out std_logic
);
end component;

component SpiController
Port
(
    CLOCK_50MHz : in  std_logic;

    OFFLOAD_INT : in std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : out std_logic;

    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;

    FPGA_INT : out std_logic;
    FEEDBACK_DATA : out std_logic_vector(7 downto 0)
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
    INTERRUPT_SIGNAL : out std_logic
);
end component;

component I2cController
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    OFFLOAD_INT : in std_logic;
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
-- 8-Bit
-- 256 Deepth
------------------------------------------------------
component Fifo
port
(
    clock : IN STD_LOGIC;

    data  : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdreq : IN STD_LOGIC;
    wrreq : IN STD_LOGIC;

    empty : OUT STD_LOGIC;
    full : OUT STD_LOGIC;
    q : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
    usedw : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
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
    OFFLOAD_RESET : in std_logic;

    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(6 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : in std_logic
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

    OFFLOAD_INT : in std_logic;
    FPGA_INT : out std_logic;

    PWM_VECTOR : in std_logic_vector(7 downto 0);

    PWM_SIGNAL : out std_logic
);
end component;

component UartDataTransfer
port
(
    CLOCK_50MHz : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector;

    UART_x86_TX : out std_logic;
    UART_x86_RX : in std_logic;

    WRITE_BUSY : out std_logic
);
end component;

component UartDataAssembly
port
(
    CLOCK_50MHz : in std_logic;

    UART_LOG_MESSAGE_ID : in UART_LOG_ID;
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
    INPUT_PULSE : in std_logic;
    OUTPUT_PULSE : out std_logic
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

DebounceController_module: DebounceController
generic map
(
    PERIOD => 50000, -- 50Mhz :: 50000*20ns = 1ms
    SM_OFFSET => 3
)
port map
(
    clock => CLOCK_50MHz,
    button_in => BUTTON_1,
    button_out => reset_button
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //          //
-- //          //
-- // [SPI] IO //
-- //          //
-- //          //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

primarySpiConverter_module: SpiConverter port map
(
	CLOCK => CLOCK_50MHz,

	CS => PRIMARY_CS,
	SCLK => PRIMARY_SCLK, -- Kernel Master always initialise SPI transfer

	SERIAL_MOSI => PRIMARY_MOSI, -- in :: Data from Kernel to Serialize
	PARALLEL_MOSI => primary_parallel_MOSI, -- out :: Serialized Data from Kernel to FIFO
	PARALLEL_MISO => "00011000", -- in :: 0x18 Hard coded Feedback to Serialize
	SERIAL_MISO => PRIMARY_MISO, -- out :: 0x18 Serialized Hard coded Feedback to Kernel

    CONVERSION_COMPLETE => primary_conversion_complete -- Out :: Data byte is ready [FIFO Write Enable]
);

secondarySpiConverter_module: SpiConverter port map
(
    CLOCK => CLOCK_50MHz,

    CS => SECONDARY_CS,
    SCLK => SECONDARY_SCLK, -- Kernel Master always initialise SPI transfer

    SERIAL_MOSI => SECONDARY_MOSI, -- in :: Serialized Feedback from Kernel :: Set in Kernel to 0x81
    PARALLEL_MOSI => open, -- out :: Not in use !
    PARALLEL_MISO => secondary_parallel_MISO, -- in :: Parallel Data from the packet switch
    SERIAL_MISO => SECONDARY_MISO, -- out :: Serialized Data from the packet switch

    CONVERSION_COMPLETE => open -- out :: Not in use !
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                  //
-- //                  //
-- // [INT] Interrupts //
-- //                  //
-- //                  //
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
	INTERRUPT_SIGNAL => WDG_INT_FROM_FPGA
);

TimerInterrupt: InterruptGenerator
generic map
(
    PERIOD_MS => 20, -- Every 20ms :: This could be absolutely minimum
    PULSE_LENGTH => 50 -- 50 * 20ns = 1us Interrupt Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    INTERRUPT_SIGNAL => TIMER_INT_FROM_FPGA
);

feedback_interrupts_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if interrupt_i2c_feedback = '1'
        or interrupt_pwm_feedback = '1'
        or interrupt_spi_bmi160_s1_feedback = '1'
        or interrupt_spi_bmi160_s2_feedback = '1'
        or interrupt_spi_bmi160_s3_feedback = '1'
        or interrupt_spi_rf_feedback = '1'
        then
            if feedback_interrupt_timer = "110010" then -- 50 * 20 = 1000ns = 1us interrupt pulse back to CPU
                SPI_INT_FROM_FPGA <= '0';
            else
                SPI_INT_FROM_FPGA <= '1';
                feedback_interrupt_timer <= feedback_interrupt_timer + '1';
            end if;
        else
            SPI_INT_FROM_FPGA <= '0';
            feedback_interrupt_timer <= (others => '0');
        end if;
    end if;
end process;

feedback_data_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if interrupt_i2c_feedback = '1'  then
            secondary_parallel_MISO <= data_i2c_feedback;
        elsif interrupt_pwm_feedback = '1' then
            secondary_parallel_MISO <= data_pwm_feedback;
        elsif interrupt_spi_bmi160_s1_feedback = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s1_feedback;
        elsif interrupt_spi_bmi160_s2_feedback = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s2_feedback;
        elsif interrupt_spi_bmi160_s3_feedback = '1' then
            secondary_parallel_MISO <= data_spi_bmi160_s3_feedback;
        elsif interrupt_spi_rf_feedback = '1' then
            secondary_parallel_MISO <= data_spi_rf_feedback;
        end if;
    end if;
end process;

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                //
-- //                //
-- // [FIFO] Control //
-- //                //
-- //                //
-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

fifo_pre_process: -- Long interrupt signal from kernel to be cut in FPGA down to 20ns pulse
process(CLOCK_50MHz, primary_parallel_MOSI, primary_conversion_complete, kernel_interrupt, interrupt_from_cpu)
begin
    if rising_edge(CLOCK_50MHz) then

        -- 1st
        interrupt_from_cpu <= SPI_INT_FROM_CPU;

        -- 2nd
        if interrupt_from_cpu = '1' and kernel_interrupt_stop = '0' then
            kernel_interrupt <= '1';
            kernel_interrupt_stop <= '1';
        elsif interrupt_from_cpu = '0' then -- resest stop when debounced long interrupt from kernel goes down
            kernel_interrupt_stop <= '0';
        else
            kernel_interrupt <= '0'; -- go down straight after 20ns
        end if;

        primary_fifo_data_in <= primary_parallel_MOSI;
        primary_fifo_wr_en <= primary_conversion_complete;
        offload_interrupt <= kernel_interrupt;
    end if;
end process;

Fifo_module: Fifo
port map
(
    clock => CLOCK_50MHz,
    -- IN
    data  => primary_fifo_data_in,
    rdreq => primary_fifo_rd_en,
    wrreq => primary_fifo_wr_en,
    -- OUT
    empty => primary_fifo_empty,
    full => primary_fifo_full,
    q => primary_fifo_data_out,
    usedw => open
);

OffloadController_module: OffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    OFFLOAD_RESET => '0',

    OFFLOAD_INTERRUPT => offload_interrupt,
    FIFO_DATA => primary_fifo_data_out,
    FIFO_READ_ENABLE => primary_fifo_rd_en,

    OFFLOAD_READY => offload_ready,
    OFFLOAD_ID => offload_id,
    OFFLOAD_CTRL => offload_ctrl,
    OFFLOAD_REGISTER => offload_register,
    OFFLOAD_DATA => offload_data,

    OFFLOAD_WAIT => offload_wait
);

-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
-- //                  //
-- //                  //
-- // [RAM] Controller //
-- //                  //
-- //                  //
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

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    UART_x86_TX => UART_x86_TX,
    UART_x86_RX => UART_x86_RX,

    WRITE_BUSY => uart_write_busy
);

UartDataAssembly_module: UartDataAssembly
port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    UART_LOG_MESSAGE_ID => UART_LOG_MESSAGE_ID,
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
        if offload_ready = '1' then
            if offload_ctrl(2 downto 1) = SWITCH_I2C then
                switch_i2c_ready <= '1';
            elsif offload_ctrl(2 downto 1) = SWITCH_SPI then
                if offload_id = SWITCH_BMI160_S1 then
                    switch_bmi160_s1_ready <= '1';
                elsif offload_id = SWITCH_BMI160_S2 then
                    switch_bmi160_s2_ready <= '1';
                elsif offload_id = SWITCH_BMI160_S3 then
                    switch_bmi160_s3_ready <= '1';
                elsif offload_id = SWITCH_nRF905 then
                    switch_spi_RF_ready <= '1';
                else
                    switch_bmi160_s1_ready <= '0';
                    switch_bmi160_s2_ready <= '0';
                    switch_bmi160_s3_ready <= '0';
                    switch_spi_RF_ready <= '0';
                end if;
            elsif offload_ctrl(2 downto 1) = SWITCH_PWM then
                switch_pwm_ready <= '1';
            else
                switch_i2c_ready <= '0';
                switch_pwm_ready <= '0';
                switch_bmi160_s1_ready <= '0';
                switch_bmi160_s2_ready <= '0';
                switch_bmi160_s3_ready <= '0';
                switch_spi_RF_ready <= '0';
            end if;
        else
            switch_i2c_ready <= '0';
            switch_pwm_ready <= '0';
            switch_bmi160_s1_ready <= '0';
            switch_bmi160_s2_ready <= '0';
            switch_bmi160_s3_ready <= '0';
            switch_spi_RF_ready <= '0';
        end if;
    end if;
end process;

-------------------------------------------------
-- Burst read is required
-- In order to read 6 Bytes
-- Starting from 0x32 ending
-- At 0x37 for x, y and z
-- Total Time: 67.5 + 135 = 202.5µs
-------------------------------------------------
I2cController_module: I2cController port map
(
    CLOCK => CLOCK_50MHz,
    RESET => reset_button,

    -- in
    OFFLOAD_INT => switch_i2c_ready, -- i2c transfer ready to begin
    -- in
    KERNEL_INT => '0',
    -- out
    FPGA_INT => interrupt_i2c_feedback, -- SM is ready for SPI.1 transfer :: 1000*20ns interrupt
    FIFO_INT => open, -- TODO :: Store output data in secondary FIFO

    I2C_SCK => I2C_SCK,
    I2C_SDA => I2C_SDA,
    -- in
    OFFLOAD_ID => offload_id, -- Device ID :: BMI160@0x69=1001011 :: ADXL345@0x53=1100101
    OFFLOAD_REGISTER => offload_register, -- Device Register
    OFFLOAD_CONTROL => offload_ctrl(0), -- For now :: Read/Write
    OFFLOAD_DATA => offload_data, -- Write Data
    -- out
    OFFLOAD_WAIT => offload_wait_i2c, -- Wait between consecutive i2c transfers
    FEEDBACK_DATA => data_i2c_feedback
);

SpiController_BMI160_S1_module: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    OFFLOAD_INT => switch_bmi160_s1_ready,

    OFFLOAD_ID => offload_id(0) & offload_id(1) &
                offload_id(2) & offload_id(3) &
                offload_id(4) & offload_id(5) &
                offload_id(6), -- Turn back around for SPI

    OFFLOAD_CONTROL => offload_ctrl,
    OFFLOAD_REGISTER => offload_register,
    OFFLOAD_DATA => offload_data,

    OFFLOAD_WAIT => offload_wait_spi_s1,

    -- Master SPI interface
    CTRL_CS => ctrl_BMI160_S1_CS,
    CTRL_MISO => ctrl_BMI160_S1_MISO,
    CTRL_MOSI => ctrl_BMI160_S1_MOSI,
    CTRL_SCK => ctrl_BMI160_S1_SCLK,

    FPGA_INT => interrupt_spi_bmi160_s1_feedback,
    FEEDBACK_DATA => data_spi_bmi160_s1_feedback
);

SpiController_BMI160_S2_module: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    OFFLOAD_INT => switch_bmi160_s2_ready,

    OFFLOAD_ID => offload_id(0) & offload_id(1) &
                offload_id(2) & offload_id(3) &
                offload_id(4) & offload_id(5) &
                offload_id(6), -- Turn back around for SPI

    OFFLOAD_CONTROL => offload_ctrl,
    OFFLOAD_REGISTER => offload_register,
    OFFLOAD_DATA => offload_data,

    OFFLOAD_WAIT => offload_wait_spi_s2,

    -- Master SPI interface
    CTRL_CS => ctrl_BMI160_S2_CS,
    CTRL_MISO => ctrl_BMI160_S2_MISO,
    CTRL_MOSI => ctrl_BMI160_S2_MOSI,
    CTRL_SCK => ctrl_BMI160_S2_SCLK,

    FPGA_INT => interrupt_spi_bmi160_s2_feedback,
    FEEDBACK_DATA => data_spi_bmi160_s2_feedback
);

SpiController_RF_module: SpiController port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    OFFLOAD_INT => switch_spi_RF_ready,

    OFFLOAD_ID => offload_id(0) & offload_id(1) &
                offload_id(2) & offload_id(3) &
                offload_id(4) & offload_id(5) &
                offload_id(6), -- Turn back around for SPI

    OFFLOAD_CONTROL => offload_ctrl,
    OFFLOAD_REGISTER => offload_register,
    OFFLOAD_DATA => offload_data,

    OFFLOAD_WAIT => offload_wait_spi_rf,

    -- Master SPI interface
    CTRL_CS => ctrl_RF_CS,
    CTRL_MISO => ctrl_RF_MISO,
    CTRL_MOSI => ctrl_RF_MOSI,
    CTRL_SCK => ctrl_RF_SCLK,

    FPGA_INT => interrupt_spi_rf_feedback,
    FEEDBACK_DATA => data_spi_rf_feedback
);

offload_wait <= offload_wait_i2c or offload_wait_spi_s1 or offload_wait_spi_s2 or offload_wait_spi_rf;

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
S1_BMI160_CS <= ctrl_BMI160_S1_CS;
ctrl_BMI160_S1_MISO <= S1_BMI160_MISO;
S1_BMI160_MOSI <= ctrl_BMI160_S1_MOSI;
S1_BMI160_SCLK <= ctrl_BMI160_S1_SCLK;

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

    OFFLOAD_INT => switch_pwm_ready,
    FPGA_INT => interrupt_pwm_feedback,

    PWM_VECTOR => offload_data,
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

PulseController_module: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    INPUT_PULSE => CFG_INT_FROM_CPU,
    OUTPUT_PULSE => open
);

------------------------------------------------------------------------------------------------------------------------------------------
--
--
-- DEBUG
--
--
------------------------------------------------------------------------------------------------------------------------------------------

--looptrough_spi_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        NRF905_CSN <= PRIMARY_CS;
--        PRIMARY_MISO <= NRF905_MISO;
--        NRF905_MOSI <= PRIMARY_MOSI;
--        NRF905_SCK <= PRIMARY_SCLK;
--    end if;
--end process;

--looptrough_spi_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        S1_BMI160_CS <= PRIMARY_CS;
--        PRIMARY_MISO <= S1_BMI160_MISO;
--        S1_BMI160_MOSI <= PRIMARY_MOSI;
--        S1_BMI160_SCLK <= PRIMARY_SCLK;
--    end if;
--end process;

--logic_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        LOGIC_CH1 <= interrupt_i2c_feedback;
--        LOGIC_CH2 <= '0';
--        LOGIC_CH3 <= '0';
--        LOGIC_CH4 <= '0';
--    end if;
--end process;

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
LED_3 <= '0';
LED_4 <= '1';
LED_5 <= '0';
LED_6 <= '1';
LED_7 <= '0';
LED_8 <= '1';

--looptrough_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        S1_BMI160_SCLK <= PRIMARY_SCLK;
--        S1_BMI160_MOSI <= PRIMARY_MOSI;
--        S1_BMI160_CS <= PRIMARY_CS;
--        PRIMARY_MISO <= S1_BMI160_MISO;
--    end if;
--end process;


end rtl;
