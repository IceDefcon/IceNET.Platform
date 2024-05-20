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
    PRIMARY_CS : in std_logic;    -- PIN_A5   :: BBB P9_17 :: PULPLE :: SPI0_CS0
    PRIMARY_MISO : out std_logic; -- PIN_A6   :: BBB P9_21 :: BROWN  :: SPI0_D0
    PRIMARY_MOSI : in std_logic;  -- PIN_A7   :: BBB P9_18 :: BLUE   :: SPI0_D1
    PRIMARY_SCLK : in std_logic;  -- PIN_A8   :: BBB P9_22 :: BLACK  :: SPI0_SCLK
    -- BBB SPI1
    SECONDARY_CS : in std_logic;    -- PIN_B5 :: BBB P9_28 :: ORANGE :: SPI1_CS0
    SECONDARY_MISO : out std_logic; -- PIN_B6 :: BBB P9_29 :: BLUE   :: SPI1_D0
    SECONDARY_MOSI : in std_logic;  -- PIN_B7 :: BBB P9_30 :: YELOW  :: SPI1_D1
    SECONDARY_SCLK : in std_logic;  -- PIN_B8 :: BBB P9_31 :: GREEN  :: SPI1_SCLK
    -- Bypass
    BYPASS_CS : out std_logic;  -- PIN_A15 :: YELLOW :: CS
    BYPASS_MISO : in std_logic; -- PIN_A16 :: ORANGE :: SA0
    BYPASS_MOSI : out std_logic;  -- PIN_A17 :: RED    :: SDX
    BYPASS_SCLK : out std_logic;  -- PIN_A18 :: BROWN  :: SCX
    -- I2C BMI160
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

-- SM Reset
signal reset_button : std_logic := '0';
-- Interrupt Pulse Generator
signal interrupt_divider : integer := 2;
signal interrupt_period : std_logic_vector(25 downto 0) := "10111110101111000001111111"; -- [50*10^6 - 1]
signal interrupt_length : std_logic_vector(3 downto 0) := "1111";
signal interrupt_signal : std_logic := '0';
-- Spi.0 Primary
signal primary_ready_MISO : std_logic := '0';
signal primary_parallel_MISO : std_logic_vector(7 downto 0) := "00011000"; -- 0x18 [Preamble]
signal primary_parallel_MOSI : std_logic_vector(7 downto 0) := "00100100"; -- 0x42
-- Spi.1 Secondary
signal secondary_ready_MISO : std_logic := '0';
signal secondary_parallel_MISO : std_logic_vector(7 downto 0) := "00011110"; -- 0xE1
signal secondary_parallel_MOSI : std_logic_vector(7 downto 0) := "00011110"; -- 0xE1
-- BMI160 Gyroscope registers
signal mag_z_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_z_7_0 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_y_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_y_7_0 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_x_15_8 : std_logic_vector(7 downto 0):= (others => '0');
signal mag_x_7_0 : std_logic_vector(7 downto 0):= (others => '0');
-- FIFO
constant primary_fifo_WIDTH : integer := 8;
constant primary_fifo_DEPTH : integer := 16;
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
signal offload_reset : std_logic := '0';
signal offload_ready : std_logic := '0';
signal offload_ctrl : std_logic_vector(7 downto 0) := (others => '0');
signal offload_register : std_logic_vector(7 downto 0) := (others => '0');
signal offload_data : std_logic_vector(7 downto 0) := (others => '0');
type STATE is 
(
    IDLE,
    DELAY_INIT,
    DELAY_CONFIG,
    READ_REGISTER,
    WRITE_CHECK,
    READ_CONTROL_DONE,
    READ_CONTROL,
    READ_DATA
);
signal offload_state: STATE := IDLE;

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

component SpiProcessing
Port 
(
    CLOCK : in  std_logic;

    CS : in std_logic;
    SCLK : in std_logic;

    SPI_INT : out std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);

    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic
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

    SPI_INT : in std_logic;
    KERNEL_INT : in std_logic;
    FPGA_INT : out std_logic;
    FIFO_INT : out std_logic;

    I2C_SCK : inout std_logic;
    I2C_SDA : inout std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_COTROL : in std_logic;
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

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

component fifo
generic 
(
    WIDTH   : integer := 8;
    DEPTH   : integer := 16
);
port 
(
    clk      : in  std_logic;
    reset    : in  std_logic;
    data_in  : in  std_logic_vector(7 downto 0);
    wr_en    : in  std_logic;
    rd_en    : in  std_logic;
    data_out : out std_logic_vector(7 downto 0);
    full     : out std_logic;
    empty    : out std_logic
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

primarySpiProcessing_module: SpiProcessing port map 
(
	CLOCK => CLOCK_50MHz,

	CS => PRIMARY_CS,
	SCLK => PRIMARY_SCLK,

    -- out
	SPI_INT => primary_ready_MISO,

	SERIAL_MOSI => PRIMARY_MOSI, -- in
	PARALLEL_MOSI => primary_parallel_MOSI, -- out

	PARALLEL_MISO => primary_parallel_MISO, -- in
	SERIAL_MISO => PRIMARY_MISO -- out
);

secondarySpiProcessing_module: SpiProcessing port map 
(
    CLOCK => CLOCK_50MHz,

    CS => SECONDARY_CS,
    SCLK => SECONDARY_SCLK,

    SPI_INT => secondary_ready_MISO, -- out

    SERIAL_MOSI => SECONDARY_MOSI, -- in
    PARALLEL_MOSI => secondary_parallel_MOSI, -- out

    PARALLEL_MISO => secondary_parallel_MISO, -- in
    SERIAL_MISO => SECONDARY_MISO -- out
);

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

    -- in
    SPI_INT => offload_ready, -- i2c transfer ready to begin
    -- in
    KERNEL_INT => '0',
    -- out
    FPGA_INT => FPGA_INT, -- SM is ready for SPI.1 transfer :: 1000*20ns interrupt
    FIFO_INT => open,

	I2C_SCK => I2C_SCK,
	I2C_SDA => I2C_SDA,
    -- in
	OFFLOAD_ID => "1001011", -- Device ID
	OFFLOAD_REGISTER => offload_register, -- Device Register
	OFFLOAD_COTROL => offload_ctrl(0), -- For now :: Read/Write
    OFFLOAD_DATA => offload_data, -- Write Data
    -- out
	DATA => secondary_parallel_MISO,

	LED_1 => LED_1,
	LED_2 => LED_2,
	LED_3 => LED_3,
	LED_4 => LED_4,
	LED_5 => LED_5,
	LED_6 => open,
	LED_7 => open,
	LED_8 => open
);

-- Convert KERNEL_INT into 20n pulse
kernel_int_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if KERNEL_INT = '1' and kernel_interrupt_stop = '0' then
            kernel_interrupt <= '1';
            kernel_interrupt_stop <= '1';
        elsif KERNEL_INT = '0' then
            kernel_interrupt_stop <= '0';
        else
            kernel_interrupt <= '0';
        end if;
    end if;
end process;

primary_fifo_write_spi_process:
process(CLOCK_50MHz, primary_parallel_MOSI, primary_ready_MISO, kernel_interrupt)
begin
    if rising_edge(CLOCK_50MHz) then
        primary_fifo_data_in <= primary_parallel_MOSI;
        primary_fifo_wr_en <= primary_ready_MISO;
        offload_interrupt <= kernel_interrupt;
    end if;
end process;

---------------------------------------
--
-- TODO
--
-- Fifo to store bytes from Kernel SPI
--
-- Byte[0] Ctrl Byte :: RW, MR, RC
-- Byte[1] Address of Device
-- Byte[2] Address of Register
-- Byte[3] Address of Register
-- ...
-- Byte[n-1] Checksum :: chk = b[0] ^ b[1] ^ b[2] ^ ... b[n-1]
--
---------------------------------------
primary_fifo_module: fifo
generic map 
(
    WIDTH => primary_fifo_WIDTH,
    DEPTH => primary_fifo_DEPTH
)
port map 
(
    -- IN
    clk      => CLOCK_50MHz,
    reset    => '0',
    data_in  => primary_fifo_data_in,
    wr_en    => primary_fifo_wr_en,
    rd_en    => primary_fifo_rd_en,
    -- OUT
    data_out => primary_fifo_data_out,
    full     => primary_fifo_full,
    empty    => primary_fifo_empty
);

process (CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case offload_state is

            when IDLE =>
                offload_ready <= '0';
                if offload_interrupt = '1' then
                    offload_state <= DELAY_INIT;
                else
                    offload_state <= IDLE;
                end if;

            when DELAY_INIT =>
                primary_fifo_rd_en <= '1';
                offload_state <= DELAY_CONFIG;
 
            when DELAY_CONFIG =>
                primary_fifo_rd_en <= '1';
                offload_state <= READ_REGISTER;

            When READ_REGISTER =>
                primary_fifo_rd_en <= '0';
                offload_register <= primary_fifo_data_out; -- Register
                offload_state <= WRITE_CHECK;

            When WRITE_CHECK =>
                if primary_fifo_data_out(0) = '0' then
                    primary_fifo_rd_en <= '0';
                    offload_state <= READ_CONTROL_DONE;
                elsif primary_fifo_data_out(0) = '1' then
                    primary_fifo_rd_en <= '1';
                    offload_state <= READ_CONTROL;
                end if;

            when READ_CONTROL_DONE => 
                offload_ctrl <= primary_fifo_data_out; -- Control
                offload_ready <= '1';
                offload_state <= IDLE;

            when READ_CONTROL =>
                primary_fifo_rd_en <= '0';
                offload_ctrl <= primary_fifo_data_out; -- Control
                offload_state <= READ_DATA;

            when READ_DATA =>
                primary_fifo_rd_en <= '0';
                offload_data <= primary_fifo_data_out; -- Data
                offload_ready <= '1';
                offload_state <= IDLE;

            when others =>
                offload_state <= IDLE;

        end case;
    end if;
end process;




-----------------------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------------------
--FPGA_INT <= interrupt_signal;

end rtl;


