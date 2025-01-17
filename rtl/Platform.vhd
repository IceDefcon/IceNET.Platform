library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.Types.all;

---------------------------------------------------------------------------
-- Author: Ice.Marek
-- IceNET Technology 2025
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
---------------------------------------------------------------------------
--
-- Pinout Communication
-- CPU <--> FPGA
--
-------------------------------------------------------------------------------------
-- PIN_A4  :: P9_11 :: UNUSED_01            | PIN_B4  :: P9_12 :: WDG_INT_FROM_CPU  |
-- PIN_C3  :: P9_13 :: UNUSED_03            | PIN_C4  :: P9_14 :: UNUSED_04         |
-- PIN_A5  :: P9_15 :: INT_FROM_CPU         | PIN_B5  :: P9_16 :: UNUSED_06         |
-- PIN_A6  :: P9_17 :: SPI0_CS0             | PIN_B6  :: P9_18 :: SPI0_D1           |
-- PIN_A7  :: P9_19 :: CAN_BBB_RX           | PIN_B7  :: P9_20 :: CAN_BBB_TX        |
-- PIN_A8  :: P9_21 :: SPI0_D0              | PIN_B8  :: P9_22 :: SPI0_SCLK         |
-- PIN_A9  :: P9_23 :: INT_FROM_FPGA        | PIN_B9  :: P9_24 :: UART_BBB_TX       |
-- PIN_A10 :: P9_25 :: UNUSED_15            | PIN_B10 :: P9_26 :: UART_BBB_RX       |
-- PIN_A13 :: P9_27 :: UNUSED_17            | PIN_B13 :: P9_28 :: SPI1_CS0          |
-- PIN_A14 :: P9_29 :: SPI1_D0              | PIN_B14 :: P9_30 :: SPI1_D1           |
-- PIN_A15 :: P9_31 :: SPI1_SCLK            | PIN_B15 :: P9_32 :: UNUSED_22         |
-- PIN_A16 :: P9_33 :: UNUSED_23            | PIN_B16 :: P9_34 :: UNUSED_24         |
-- PIN_A17 :: P9_35 :: UNUSED_25            | PIN_B17 :: P9_36 :: UNUSED_26         |
-- PIN_A18 :: P9_37 :: UNUSED_27            | PIN_B18 :: P9_38 :: UNUSED_28         |
-- PIN_A19 :: P9_39 :: UNUSED_29            | PIN_B19 :: P9_40 :: UNUSED_30         |
-- PIN_A20 :: P9_41 :: WDG_INT_FROM_FPGA    | PIN_B20 :: P9_42 :: UNUSED_32         |
-------------------------------------------------------------------------------------
-- PIN_AB13 :: NOTUSED_01                   | PIN_AA13 :: NOTUSED_02            |
-- PIN_AB14 :: NOTUSED_03                   | PIN_AA14 :: NOTUSED_04            |
-- PIN_AB15 :: NOTUSED_05                   | PIN_AA15 :: NOTUSED_06            |
-- PIN_AB16 :: NOTUSED_07                   | PIN_AA16 :: NOTUSED_08            |
-- PIN_AB17 :: NOTUSED_09                   | PIN_AA17 :: NOTUSED_10            |
-- PIN_AB18 :: NOTUSED_11                   | PIN_AA18 :: NOTUSED_12            |
-- PIN_AB19 :: NOTUSED_13                   | PIN_AA19 :: NOTUSED_14            |
-- PIN_AB20 :: NOTUSED_15                   | PIN_AA20 :: NOTUSED_16            |
-- PIN_Y21  :: NOTUSED_17                   | PIN_Y22  :: NOTUSED_18            |
-- PIN_W21  :: NOTUSED_19                   | PIN_W22  :: NOTUSED_20            |
-- PIN_V21  :: NOTUSED_21                   | PIN_V22  :: NOTUSED_22            |
-- PIN_U21  :: NOTUSED_23                   | PIN_U22  :: NOTUSED_24            |
-- PIN_R21  :: NOTUSED_25                   | PIN_R22  :: NOTUSED_26            |
-- PIN_P21  :: NOTUSED_27                   | PIN_P22  :: NOTUSED_28            |
-- PIN_N21  :: NOTUSED_29                   | PIN_N22  :: NOTUSED_30            |
-- PIN_M21  :: NOTUSED_31                   | PIN_M22  :: NOTUSED_32            |
-- PIN_L21  :: NOTUSED_33                   | PIN_L22  :: NOTUSED_34            |
-- PIN_K21  :: NOTUSED_35                   | PIN_K22  :: NOTUSED_36            |
-- PIN_J21  :: NOTUSED_37                   | PIN_J22  :: NOTUSED_38            |
-- PIN_H21  :: NOTUSED_39                   | PIN_H22  :: NOTUSED_40            |
-- PIN_F21  :: NOTUSED_41                   | PIN_F22  :: NOTUSED_42            |
-- PIN_E21  :: NOTUSED_43                   | PIN_E22  :: NOTUSED_44            |
-- PIN_D21  :: LOGIC_CH5                    | PIN_D22  :: LOGIC_CH6             |
-- PIN_C21  :: LOGIC_CH3                    | PIN_C22  :: LOGIC_CH4             |
-- PIN_B21  :: LOGIC_CH1                    | PIN_B22  :: LOGIC_CH2             |
-- PIN_N19  :: NOTUSED_51                   | PIN_N20  :: NOTUSED_52            |
-- PIN_M19  :: NOTUSED_53                   | PIN_M20  :: NOTUSED_54            |
---------------------------------------------------------------------------------


entity Platform is
port
(
	-- FPGA Reference Clock
    CLOCK_50MHz : in std_logic; -- PIN_T2
    -- Debug LED's
    LED_1 : out std_logic; -- PIN_U7
    LED_2 : out std_logic; -- PIN_U8
    LED_3 : out std_logic; -- PIN_R7
    LED_4 : out std_logic; -- PIN_T8
    LED_5 : out std_logic; -- PIN_R8
    LED_6 : out std_logic; -- PIN_P8
    LED_7 : out std_logic; -- PIN_M8
    LED_8 : out std_logic; -- PIN_N8
    -- Debug Buttons
    BUTTON_1 : in std_logic; -- PIN_H20
    BUTTON_2 : in std_logic; -- PIN_K19
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic; -- PIN_K18

    ---------------------------------------------------------------------------------------------------
    --
    -- Line 0
    --
    ---------------------------------------------------------------------------------------------------
    -- Interrupts
    INT_FROM_CPU : in std_logic;    -- PIN_A5 :: P9_15
    INT_FROM_FPGA : out std_logic;  -- PIN_A9 :: P9_23

    -- BBB SPI0
    PRIMARY_CS : in std_logic;    -- PIN_A6 :: P9_17 :: SPI0_CS0
    PRIMARY_MISO : out std_logic; -- PIN_A8 :: P9_21 :: SPI0_D0
    PRIMARY_MOSI : in std_logic;  -- PIN_B6 :: P9_18 :: SPI0_D1
    PRIMARY_SCLK : in std_logic;  -- PIN_B8 :: P9_22 :: SPI0_SCLK
    -- I2C BMI160 + ADXL345
    I2C_SDA : inout std_logic; -- PIN_AB13
    I2C_SCK : inout std_logic; -- PIN_AA13
    -- Current Debug SPI Driver
    NRF905_TX_EN : out std_logic;   -- PIN_F1
    NRF905_TRX_CE : out std_logic;  -- PIN_H2
    NRF905_PWR_UP : out std_logic;  -- PIN_H1
    NRF905_uCLK : in std_logic;     -- PIN_J2
    NRF905_CD : in std_logic;       -- PIN_J1
    NRF905_AM : in std_logic;       -- PIN_M2
    NRF905_DR : in std_logic;       -- PIN_M1
    NRF905_MISO : in std_logic;     -- PIN_N2
    NRF905_MOSI : out std_logic;    -- PIN_N1
    NRF905_SCK : out std_logic;     -- PIN_P2
    NRF905_CSN : out std_logic;     -- PIN_P1
    -- PWM
    PWM_SIGNAL : out std_logic; -- PIN_R1
    -- BBB SPI1
    SECONDARY_CS : in std_logic;    -- PIN_B13 :: P9_28 :: SPI1_CS0
    SECONDARY_MISO : out std_logic; -- PIN_A14 :: P9_29 :: SPI1_D0
    SECONDARY_MOSI : in std_logic;  -- PIN_B14 :: P9_30 :: SPI1_D1
    SECONDARY_SCLK : in std_logic;  -- PIN_A15 :: P9_31 :: SPI1_SCLK
    -- Watchdog signal
    WDG_INT_FROM_FPGA : out std_logic; -- PIN_A20 :: P9_41
    WDG_INT_FROM_CPU : in std_logic; -- PIN_B4 :: P9_12

    UART_BBB_TX : in std_logic;  -- PIN_B9 :: P9_24
    UART_BBB_RX : out std_logic; -- PIN_B10 :: P9_26
    UART_x86_TX : out std_logic; -- PIN_N19 :: FTDI Rx
    UART_x86_RX : in std_logic;  -- PIN_M19 :: FTDI Tx
    --
    -- BBB to MPP
    --
    -- Chip CAN_H :: Blue  ---> MPP :: CAN_P :: Yellow
    -- Chip CAN_L :: White ---> MPP :: CAN_N :: Blue
    CAN_BBB_TX : in std_logic;  -- PIN_B7 :: P9_20
    CAN_BBB_RX : out std_logic; -- PIN_A7 :: P9_19
    CAN_MPP_TX : out std_logic; -- PIN_N20 :: MPP Tx
    CAN_MPP_RX : in std_logic;  -- PIN_M20 :: MPP Rx
    -- Locic
    LOGIC_CH1 : out std_logic; -- PIN_B21
    LOGIC_CH2 : out std_logic; -- PIN_B22
    LOGIC_CH3 : out std_logic; -- PIN_C21
    LOGIC_CH4 : out std_logic; -- PIN_C22
    LOGIC_CH5 : out std_logic; -- PIN_D21
    LOGIC_CH6 : out std_logic; -- PIN_D22
    --
    -- Tri-state Unused Pins :: For the PCB Safety
    --
    UNUSED_01 : inout std_logic; -- PIN_A4
    UNUSED_03 : inout std_logic; -- PIN_C3
    UNUSED_04 : inout std_logic; -- PIN_C4
    UNUSED_06 : inout std_logic; -- PIN_B5
    UNUSED_15 : inout std_logic; -- PIN_A10
    UNUSED_17 : inout std_logic; -- PIN_A13
    UNUSED_22 : inout std_logic; -- PIN_B15 :: VDD_ADC :: Not Connected
    UNUSED_23 : inout std_logic; -- PIN_A16
    UNUSED_24 : inout std_logic; -- PIN_B16 :: GND_ADC :: Not Connected
    UNUSED_25 : inout std_logic; -- PIN_A17
    UNUSED_26 : inout std_logic; -- PIN_B17
    UNUSED_27 : inout std_logic; -- PIN_A18
    UNUSED_28 : inout std_logic; -- PIN_B18
    UNUSED_29 : inout std_logic; -- PIN_A19
    UNUSED_30 : inout std_logic; -- PIN_B19
    UNUSED_32 : inout std_logic; -- PIN_B20
    -----------------------------------------------------
    --
    -- 256Mbit SDRAM
    --
    -----------------------------------------------------
    -- Row Address: RA0 ~ RA12, Column Address: CA0 ~ CA8
    -- Auto-precharge flag: A10
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
    -- Main System Clock
    CLK_SDRAM : out std_logic; -- PIN_Y6
    -- Bank Address
    BA0 : out std_logic; -- A13 :: PIN_Y1
    BA1 : out std_logic; -- A14 :: PIN_W2
    -- When active (low): The memory controller sends the column address on the address bus
    CAS : out std_logic; -- PIN_AA4
    -- Clock Enable:
    CKE : out std_logic; -- PIN_W6
    -- When active (low): The memory controller sends the row address on the address bus
    RAS : out std_logic; -- PIN_AB3
    -- When active (low): The operation is a write (data will be written to the DRAM)
    -- When inactive (high): The operation is a read (data will be read from the DRAM)
    WE : out std_logic; -- PIN_AB4
    -- Chip Select: Enables or disables all inputs except CLK, CKE and DQM
    CS : out std_logic; -- PIN_AA3
    -- Data Input / Output: Multiplexed data input / output pin
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
    -- Data Mask: Controls output buffers in read mode and masks input data in write mode
    LDQM : inout std_logic; -- PIN_AA5
    UDQM : inout std_logic -- PIN_W7
);
end Platform;

architecture rtl of Platform is

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
-- PacketSwitch
signal switch_i2c_ready : std_logic := '0';
signal switch_pwm_ready : std_logic := '0';
-- Feedback interrupts
signal interrupt_i2c_feedback : std_logic := '0';
signal interrupt_pwm_feedback : std_logic := '0';
-- Feedback data
signal data_i2c_feedback : std_logic_vector(7 downto 0) := (others => '0');
signal data_pwm_feedback : std_logic_vector(7 downto 0) := "11111101";
-- Debounce signals
signal interrupt_from_cpu : std_logic := '0';
-- Interrupts
signal interrupt_feedback_signal : std_logic := '0';
signal interrupt_feedback_count : std_logic_vector(7 downto 0) := (others => '0');
-- UART
signal uart_write_enable : std_logic := '0';
signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_write_busy : std_logic := '0';
-- UART Test Log
signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("0101", "1100"); -- 0x5C
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("0111", "1010", "0001", "1011"); -- 0x7A1B
-- Signals for SDRAM Address and Data Mapping
signal SD_BANK : std_logic_vector(1 downto 0);
signal SD_ADDRESS : std_logic_vector(12 downto 0);
signal SD_DATA : std_logic_vector(15 downto 0);
-- Test
type TEST_STATE is
(
    TEST_IDLE,
    TEST_INIT,
    TEST_CONFIG,
    TEST_READ,
    TEST_WRITE,
    TEST_DONE
);

signal test_ram_state : TEST_STATE := TEST_IDLE;
-- Test
signal test_timer : std_logic_vector(26 downto 0) := (others => '0');
signal test_flag : std_logic := '0';
signal test_ops : integer := 0;
--
--
-- Address ---> Row[23:11] : Bank[10:9] : Column[8:0]
-- Data ---> 0x5570
--
signal TEST_ADDR :  std_logic_vector(23 downto 0) := "000000000000100000000011";
signal TEST_WRITE_EN : std_logic := '0';
signal TEST_DATA_IN :  std_logic_vector(15 downto 0) := "0101010101110000";
signal TEST_READ_EN : std_logic := '0';
signal TEST_DATA_OUT : std_logic_vector(15 downto 0) := (others => '0');
signal TEST_READY : std_logic := '0';
-- PLL
signal clock_100MHz : std_logic := '0';

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
    OFFLOAD_COTROL : in std_logic;
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : out std_logic;
    DATA : out std_logic_vector(7 downto 0)
);
end component;

--
-- Fifo cannot be state machine
-- controlled due to continuous
-- read procedure during offload
--
component FifoController
generic
(
    WIDTH : integer := 8;
    DEPTH : integer := 16
);
port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;
    -- In
    DATA_IN : in  std_logic_vector(7 downto 0);
    WRITE_EN : in  std_logic;
    READ_EN : in  std_logic;
    -- Out
    DATA_OUT : out std_logic_vector(7 downto 0);
    FULL : out std_logic;
    EMPTY : out std_logic
);
end component;

component SDRAM_Controller
Port
(
    CLK         : in  std_logic;
    CLK_SLOW    : in  std_logic;
    RESET       : in  std_logic;

    -- SDRAM Interface
    A           : out std_logic_vector(12 downto 0);  -- Address Bus
    BA          : out std_logic_vector(1 downto 0);   -- Bank Address
    CLK_SDRAM   : out std_logic;
    CKE         : out std_logic;
    CS          : out std_logic;
    RAS         : out std_logic;
    CAS         : out std_logic;
    WE          : out std_logic;
    DQ          : inout std_logic_vector(15 downto 0); -- Data Bus
    LDQM        : out std_logic;
    UDQM        : out std_logic;

    -- User Interface
    ADDR        : in  std_logic_vector(23 downto 0);
    DATA_IN     : in  std_logic_vector(15 downto 0);
    DATA_OUT    : out std_logic_vector(15 downto 0);
    READ_EN     : in  std_logic;
    WRITE_EN    : in  std_logic;
    READY       : out std_logic
);
end component;

component OffloadController
port
(
    CLOCK_50MHz : in std_logic;

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

component CanController
port
(
    CLOCK_50MHz : in std_logic;

    CAN_BBB_TX : in std_logic;
    CAN_BBB_RX : out std_logic;

    CAN_MPP_TX : out std_logic;
    CAN_MPP_RX : in std_logic
);
end component;

--component SDRAM_CLOAK
--port
--(
--    ref_clk_clk        : in  std_logic := 'X'; -- clk
--    ref_reset_reset    : in  std_logic := 'X'; -- reset
--    sys_clk_clk        : out std_logic;        -- clk
--    sdram_clk_clk      : out std_logic;        -- clk
--    reset_source_reset : out std_logic         -- reset
--);
--end component SDRAM_CLOAK;

component SDRAM_PLL
port
(
    areset  : IN STD_LOGIC := '0';
    inclk0  : IN STD_LOGIC := '0';
    c0      : OUT STD_LOGIC;
    locked  : OUT STD_LOGIC
);
end component SDRAM_PLL;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin

----------------------------------------------------------
-- Tri-state Unused Pins :: For the PCB Safety
----------------------------------------------------------
UNUSED_01 <= 'Z';
UNUSED_03 <= 'Z';
UNUSED_04 <= 'Z';
UNUSED_06 <= 'Z';
UNUSED_15 <= 'Z';
UNUSED_17 <= 'Z';
UNUSED_22 <= 'Z';
UNUSED_23 <= 'Z';
UNUSED_24 <= 'Z';
UNUSED_25 <= 'Z';
UNUSED_26 <= 'Z';
UNUSED_27 <= 'Z';
UNUSED_28 <= 'Z';
UNUSED_29 <= 'Z';
UNUSED_30 <= 'Z';
UNUSED_32 <= 'Z';
----------------------------------------------------------

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

-- Watchdog interrupt signal
InterruptGenerator_module: InterruptGenerator
generic map
(
    PERIOD_MS => 100,
    PULSE_LENGTH => 50 -- 50 * 20ns = 100ns Interrupt Pulse
)
port map
(
	CLOCK_50MHz => CLOCK_50MHz,
	INTERRUPT_SIGNAL => WDG_INT_FROM_FPGA
);

--
-- Long interrupt signal from kernel
-- To be cut in FPGA down to 20ns pulse
--
fifo_pre_process:
process(CLOCK_50MHz, primary_parallel_MOSI, primary_conversion_complete, kernel_interrupt, interrupt_from_cpu)
begin
    if rising_edge(CLOCK_50MHz) then

        -- 1st
        interrupt_from_cpu <= INT_FROM_CPU;

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

---------------------------------------
--
-- Fifo to store bytes from Kernel SPI
--
-- Byte[0] :: READ_CONTROL
-- Byte[1] :: READ_ID
-- Byte[2] :: READ_REGISTER
-- Byte[3] :: READ_DATA
--
-- TODO :: CHECKSUM
--
---------------------------------------
FifoController_module: FifoController
generic map
(
    WIDTH => FIFO_WIDTH,
    DEPTH => FIFO_DEPTH
)
port map
(
    CLOCK_50MHz => CLOCK_50MHz,
    RESET => '0',
    -- IN
    DATA_IN  => primary_fifo_data_in,
    WRITE_EN => primary_fifo_wr_en,
    READ_EN => primary_fifo_rd_en,
    -- OUT
    DATA_OUT => primary_fifo_data_out,
    FULL => primary_fifo_full,
    EMPTY => primary_fifo_empty
);

SDRAM_Controller_module: SDRAM_Controller
port map
(
    CLK => clock_100MHz,
    CLK_SLOW => CLOCK_50MHz,
    RESET => '0',

    -- SDRAM Interface
    A => SD_ADDRESS,
    BA => SD_BANK,
    CLK_SDRAM => CLK_SDRAM,
    CKE => CKE,
    CS => CS,
    RAS => RAS,
    CAS => CAS,
    WE => WE,
    DQ => SD_DATA,
    LDQM => LDQM,
    UDQM => UDQM,

    -- User Interface
    ADDR => TEST_ADDR,
    DATA_IN => TEST_DATA_IN,
    DATA_OUT => TEST_DATA_OUT,
    READ_EN => TEST_READ_EN,
    WRITE_EN => TEST_WRITE_EN,
    READY => TEST_READY
);



-----------------------------------------------------------------------------------------------
-- Map to output
-----------------------------------------------------------------------------------------------

BA0 <= SD_BANK(0);
BA1 <= SD_BANK(1);

A0 <= SD_ADDRESS(0);
A1 <= SD_ADDRESS(1);
A2 <= SD_ADDRESS(2);
A3 <= SD_ADDRESS(3);
A4 <= SD_ADDRESS(4);
A5 <= SD_ADDRESS(5);
A6 <= SD_ADDRESS(6);
A7 <= SD_ADDRESS(7);
A8 <= SD_ADDRESS(8);
A9 <= SD_ADDRESS(9);
A10 <= SD_ADDRESS(10);
A11 <= SD_ADDRESS(11);
A12 <= SD_ADDRESS(12);

D0 <= SD_DATA(0);
D1 <= SD_DATA(1);
D2 <= SD_DATA(2);
D3 <= SD_DATA(3);
D4 <= SD_DATA(4);
D5 <= SD_DATA(5);
D6 <= SD_DATA(6);
D7 <= SD_DATA(7);
D8 <= SD_DATA(8);
D9 <= SD_DATA(9);
D10 <= SD_DATA(10);
D11 <= SD_DATA(11);
D12 <= SD_DATA(12);
D13 <= SD_DATA(13);
D14 <= SD_DATA(14);
D15 <= SD_DATA(15);

OffloadController_module: OffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,

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

CanController_module: CanController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    CAN_BBB_TX => CAN_BBB_TX,
    CAN_BBB_RX => CAN_BBB_RX,

    CAN_MPP_TX => CAN_MPP_TX,
    CAN_MPP_RX => CAN_MPP_RX
);

--SDRAM_CLOAK_module: SDRAM_CLOAK
--port map
--(
--    ref_clk_clk => CLOCK_50MHz,
--    ref_reset_reset => '0',
--    sys_clk_clk => open,
--    sdram_clk_clk => CLK_SDRAM,
--    reset_source_reset => open
--);

SDRAM_PLL_module: SDRAM_PLL
port map
(
    areset => '0',
    inclk0 => CLOCK_50MHz,
    c0 => clock_100MHz,
    locked => open
);

PacketSwitch:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if offload_ready = '1' then
            if offload_ctrl(1) = '1' then
                switch_pwm_ready <= '1';
            else
                switch_i2c_ready <= '1';
            end if;
        else
            switch_i2c_ready <= '0';
            switch_pwm_ready <= '0';
        end if;
    end if;
end process;

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
	OFFLOAD_COTROL => offload_ctrl(0), -- For now :: Read/Write
    OFFLOAD_DATA => offload_data, -- Write Data
    -- out
    OFFLOAD_WAIT => offload_wait, -- Wait between consecutive i2c transfers
	DATA => data_i2c_feedback
);

--
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
--
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

return_interrupts_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if interrupt_i2c_feedback = '1' or interrupt_pwm_feedback = '1' then
            if interrupt_feedback_count = "11111010" then -- 250 * 20 = 5000ns = 5us interrupt pulse back to CPU
                INT_FROM_FPGA <= '0';
            else
                INT_FROM_FPGA <= '1';
                interrupt_feedback_count <= interrupt_feedback_count + '1';
            end if;
        else
            INT_FROM_FPGA <= '0';
            interrupt_feedback_count <= (others => '0');
        end if;
    end if;
end process;

return_data_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if interrupt_i2c_feedback = '1'  then
            secondary_parallel_MISO <= data_i2c_feedback;
        elsif interrupt_pwm_feedback = '1' then
            secondary_parallel_MISO <= data_pwm_feedback;
        end if;
    end if;
end process;

logic_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        LOGIC_CH1 <= PRIMARY_CS;
        LOGIC_CH2 <= PRIMARY_MOSI;
        LOGIC_CH3 <= PRIMARY_SCLK;
        LOGIC_CH4 <= INT_FROM_CPU;
    end if;
end process;

--uart_loopthrough_process:
--process(CLOCK_50MHz)
--begin
--    if rising_edge(CLOCK_50MHz) then
--        UART_x86_TX <= UART_BBB_TX;
--        UART_BBB_RX <= UART_x86_RX;
--    end if;
--end process;

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

--NRF905_PWR_UP <= '1';
--NRF905_TRX_CE <= '0';
--NRF905_TX_EN <= 'Z';

process (CLOCK_100MHz, test_ram_state)
begin
    if rising_edge(CLOCK_100MHz) then

        case (test_ram_state) is

            when TEST_IDLE =>
                if test_timer = "101111101011110000011111111" then
                    test_ram_state <= TEST_INIT;
                else
                    test_timer <= test_timer + '1';
                end if;

            when TEST_INIT =>
                test_ram_state <= TEST_WRITE;

            when TEST_CONFIG =>
                TEST_READ_EN <= '0';
                TEST_WRITE_EN <= '0';
                if test_flag = '0' then
                    if test_ops = 3 then
                        test_ops <= 0;
                        test_flag <= '1';
                        test_ram_state <= TEST_CONFIG;
                    else
                        test_ops <= test_ops + 1;
                        test_ram_state <= TEST_WRITE;
                    end if;
                elsif test_flag = '1' then
                    if test_ops = 4 then
                        test_ops <= 0;
                        test_flag <= '0';
                        test_ram_state <= TEST_DONE;
                    else
                        test_ops <= test_ops + 1;
                        test_ram_state <= TEST_READ;
                    end if;
                end if;

            when TEST_WRITE =>
                TEST_WRITE_EN <= '1';
                TEST_DATA_IN <= TEST_DATA_IN + '1';
                TEST_ADDR <= TEST_ADDR + "100000000000";
                test_ram_state <= TEST_CONFIG;

            when TEST_READ =>
                TEST_READ_EN <= '1';
                TEST_ADDR <= TEST_ADDR - "100000000000";
                test_ram_state <= TEST_CONFIG;

            when TEST_DONE =>
                test_ram_state <= TEST_DONE;

        end case;
    end if;
end process;

end rtl;
