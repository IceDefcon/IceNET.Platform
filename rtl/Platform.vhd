library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

----------------------------------------
-- Author: Ice.Marek
-- IceNET Technology 2024
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
----------------------------------------
entity Platform is
port
(
	-- FPGA Reference Clock
    CLOCK_50MHz : in std_logic; -- PIN_T2
    -- BBB SPI0
    MAIN_CS : in std_logic;    -- PIN_A5   :: BBB P9_17 :: PULPLE :: SPI0_CS0
    MAIN_MISO : out std_logic; -- PIN_A6   :: BBB P9_21 :: BROWN  :: SPI0_D0
    MAIN_MOSI : in std_logic;  -- PIN_A7   :: BBB P9_18 :: BLUE   :: SPI0_D1
    MAIN_SCLK : in std_logic;  -- PIN_A8   :: BBB P9_22 :: BLACK  :: SPI0_SCLK
    -- BBB SPI1
    SECOND_CS : in std_logic;    -- PIN_B5 :: BBB P9_28 :: ORANGE :: SPI1_CS0
    SECOND_MISO : out std_logic; -- PIN_B6 :: BBB P9_29 :: BLUE   :: SPI1_D0
    SECOND_MOSI : in std_logic;  -- PIN_B7 :: BBB P9_30 :: YELOW  :: SPI1_D1
    SECOND_SCLK : in std_logic;  -- PIN_B8 :: BBB P9_31 :: GREEN  :: SPI1_SCLK
    -- Bypass
    BYPASS_CS : out std_logic;  -- PIN_A15 :: YELLOW :: CS
    BYPASS_MISO : in std_logic; -- PIN_A16 :: ORANGE :: SA0
    BYPASS_MOSI : out std_logic;  -- PIN_A17 :: RED    :: SDX
    BYPASS_SCLK : out std_logic;  -- PIN_A18 :: BROWN  :: SCX
    -- I2C Gyroscope
    I2C_SDA : inout std_logic; -- PIN_A9  :: BBB P9_20 :: CPU.BLUE <> FPGA.BLUE <> GYRO.WHITE
    I2C_SCK : inout std_logic; -- PIN_A10 :: BBB P9_19 :: CPU.ORANGE <> FPGA.GREEN <> GYRO.PURPLE
	-- Interrupts 
    FPGA_INT : out std_logic;  -- PIN_A3 :: BBB P9_12 :: BLACK
    KERNEL_INT : in std_logic; -- PIN_A4 :: BBB P9_14 :: WHITE
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
    BUTTON_1 : in std_logic; -- PIN_H20  :: Reset
    BUTTON_2 : in std_logic; -- PIN_K19  :: Doesnt Work :: Incorrect Schematic or Broken Button
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic  -- PIN_K18
);
end Platform;

architecture rtl of Platform is

----------------------------------------------------------------------------------------------------------------
-- Signals
----------------------------------------------------------------------------------------------------------------

-- SM Init
signal system_start : std_logic := '0';
-- SM Reset
signal reset_button : std_logic := '0';
--SM Parameters
constant smStartDelay : std_logic_vector(26 downto 0):= "101111101011110000011111111";
constant smStateDelay : std_logic_vector(24 downto 0):= "1011111010111100000111111";
-- SM Status Register
signal status_sck : std_logic_vector(3 downto 0) := "0000";
signal status_sda : std_logic_vector(3 downto 0) := "0000";

-- Interrupt Pulse Generator
signal interrupt_divider : integer := 2;
signal interrupt_period : std_logic_vector(25 downto 0) := "10111110101111000001111111"; -- [50*10^6 - 1]
signal interrupt_length : std_logic_vector(3 downto 0) := "1111";
signal interrupt_signal : std_logic := '0';

-- I2C Return Data
signal return_data : std_logic_vector(7 downto 0) := "00011000";
signal second_data : std_logic_vector(7 downto 0) := "00011110";

-- SPI Kernel Feedback Data
signal mainSpiDataFeedback_MISO : std_logic := '0';
signal secondSpiDataFeedback_MISO : std_logic := '0';

-- Delay Timers
signal system_timer : std_logic_vector(26 downto 0) := (others => '0');
signal init_timer : std_logic_vector(24 downto 0) := (others => '0');
signal config_timer : std_logic_vector(24 downto 0) := (others => '0');
signal send_timer : std_logic_vector(24 downto 0) := (others => '0');	
signal done_timer : std_logic_vector(24 downto 0) := (others => '0');

-- Process Timers
signal status_timer : std_logic_vector(15 downto 0) := (others => '0');
signal sck_timer : std_logic_vector(7 downto 0) := (others => '0');
signal sda_timer : std_logic_vector(8 downto 0) := (others => '0');
signal sck_timer_toggle : std_logic := '0';

-- Parametric offset
signal sda_offset : std_logic_vector(15 downto 0) := (others => '0');

--I2c state machine
type STATE is 
(
	IDLE,
	INIT,
	CONFIG,
	SEND,
	DONE
);
signal state_current, state_next: STATE := IDLE;

-- Status Indicators
signal isIDLE : std_logic := '0';
signal isINIT : std_logic := '0';
signal isCONFIG : std_logic := '0';
signal isDEVICE : std_logic := '0';
signal isDONE : std_logic := '0';

-- L3G4200D Gyroscope registers
signal OUT_X_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_X_H : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Y_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Y_H : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Z_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Z_H : std_logic_vector(7 downto 0):= (others => '0');

----------------------------------------------------------------------------------------------------------------
-- COMPONENTS DECLARATION
----------------------------------------------------------------------------------------------------------------
component Debounce
port
(
	clock : in  std_logic;
	button_in_1 : in  std_logic;
	button_in_2 : in  std_logic;
	button_in_3 : in  std_logic;
	button_in_4 : in  std_logic;
	button_out_1 : out std_logic;
	button_out_2 : out std_logic;
	button_out_3 : out std_logic;
	button_out_4 : out std_logic
);
end component;

component SpiDataFeedback
Port 
(
    CLOCK : in  std_logic;
    SCLK : in std_logic;
    DATA : in std_logic_vector(7 downto 0);
    synced_miso : out std_logic
);
end component;

component InterruptPulse
Port 
(
    CLOCK : in  std_logic;
    interrupt_period : in  std_logic_vector(25 downto 0);
    interrupt_length : in  std_logic_vector(3 downto 0);
    interrupt_signal : out std_logic
);
end component;

component I2cStateMachine
port
(    
    CLOCK : in std_logic;
    RESET : in std_logic;

    CPU_INT : in std_logic;
    KERNEL_INT : in std_logic;
    FPGA_INT : out std_logic;

    I2C_SCK : inout std_logic;
    I2C_SDA : inout std_logic;

    DATA : out std_logic_vector(7 downto 0);

    LED_1 : out std_logic;
    LED_2 : out std_logic;
    LED_3 : out std_logic;
    LED_4 : out std_logic;
    LED_5 : out std_logic;
    LED_6 : out std_logic;
    LED_7 : out std_logic;
    LED_8 : out std_logic
);
end component;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin

Debounce_module: Debounce port map 
(
	clock => CLOCK_50MHz,
	button_in_1 => BUTTON_1,
	button_in_2 => BUTTON_2,
	button_in_3 => BUTTON_3,
	button_in_4 => BUTTON_4,
	button_out_1 => reset_button,
	button_out_2 => open,
	button_out_3 => open,
	button_out_4 => open
);

mainSpiDataFeedback_module: SpiDataFeedback port map 
(
	CLOCK => CLOCK_50MHz,
	SCLK => MAIN_SCLK,
	DATA => return_data,
	synced_miso => mainSpiDataFeedback_MISO
);

MAIN_MISO <= mainSpiDataFeedback_MISO;

secondSpiDataFeedback_module: SpiDataFeedback port map 
(
	CLOCK => CLOCK_50MHz,
	SCLK => SECOND_SCLK,
	DATA => second_data,
	synced_miso => secondSpiDataFeedback_MISO
);

SECOND_MISO <= secondSpiDataFeedback_MISO;

--BYPASS_CS <= SECOND_CS;
--SECOND_MISO <= BYPASS_MISO;
--BYPASS_MOSI <= SECOND_MOSI;
--BYPASS_SCLK <= SECOND_SCLK;

------------------------------------------------------
-- Interrupt pulse :: 0x2FAF07F/50 MHz
-- (49999999 + 1)/50000000 Hz = 1 sec
--
-- Divide 0 :: 50000000 >> 0 :: 50000000*20ns = 1000ms
-- Divide 1 :: 50000000 >> 1 :: 25000000*20ns = 500ms
-- Divide 2 :: 50000000 >> 2 :: 12500000*20ns = 250ms
-- Divide 3 :: 50000000 >> 3 :: 6250000*20ns = 125ms
-- Divide 4 :: 50000000 >> 4 :: 3125000*20ns = 62.5ms
--
-- Interrupt length :: 0xF
-- 16 * 20ns = 320 ns
------------------------------------------------------
Interrupt_module: InterruptPulse port map 
(
	CLOCK => CLOCK_50MHz,
	interrupt_period => std_logic_vector(unsigned(interrupt_period) srl interrupt_divider),
	interrupt_length => interrupt_length,
	interrupt_signal => interrupt_signal
);

I2cStateMachine_module: I2cStateMachine port map 
(
	CLOCK => CLOCK_50MHz,
	RESET => reset_button,

    CPU_INT => '0',
    KERNEL_INT => KERNEL_INT,
    FPGA_INT => FPGA_INT,

	I2C_SCK => I2C_SCK,
	I2C_SDA => I2C_SDA,

	DATA => return_data,

	LED_1 => LED_1,
	LED_2 => LED_2,
	LED_3 => LED_3,
	LED_4 => LED_4,
	LED_5 => LED_5,
	LED_6 => LED_6,
	LED_7 => LED_7,
	LED_8 => LED_8
);

-----------------------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------------------
--FPGA_INT <= interrupt_signal;

end rtl;


