library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

---------------------------------------------------------------------------
-- Author: Ice.Marek
-- IceNET Technology 2024
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
---------------------------------------------------------------------------
--
-- Pinout Communication
-- CPU <--> FPGA
--
---------------------------------------------------------------------------------
-- PIN_A4  :: P9_11 :: UNUSED_01            | PIN_B4  :: P9_12 :: UNUSED_02     |
-- PIN_C3  :: P9_13 :: UNUSED_03            | PIN_C4  :: P9_14 :: UNUSED_04     |
-- PIN_A5  :: P9_15 :: INT_FROM_CPU         | PIN_B5  :: P9_16 :: UNUSED_06     |
-- PIN_A6  :: P9_17 :: SPI0_CS0             | PIN_B6  :: P9_18 :: SPI0_D1       |
-- PIN_A7  :: P9_19 :: CAN_BBB_RX           | PIN_B7  :: P9_20 :: CAN_BBB_TX    |
-- PIN_A8  :: P9_21 :: SPI0_D0              | PIN_B8  :: P9_22 :: SPI0_SCLK     |
-- PIN_A9  :: P9_23 :: INT_FROM_FPGA        | PIN_B9  :: P9_24 :: UART_BBB_TX   |
-- PIN_A10 :: P9_25 :: UNUSED_15            | PIN_B10 :: P9_26 :: UART_BBB_RX   |
-- PIN_A13 :: P9_27 :: UNUSED_17            | PIN_B13 :: P9_28 :: SPI1_CS0      |
-- PIN_A14 :: P9_29 :: SPI1_D0              | PIN_B14 :: P9_30 :: SPI1_D1       |
-- PIN_A15 :: P9_31 :: SPI1_SCLK            | PIN_B15 :: P9_32 :: UNUSED_22     |
-- PIN_A16 :: P9_33 :: UNUSED_23            | PIN_B16 :: P9_34 :: UNUSED_24     |
-- PIN_A17 :: P9_35 :: UNUSED_25            | PIN_B17 :: P9_36 :: UNUSED_26     |
-- PIN_A18 :: P9_37 :: UNUSED_27            | PIN_B18 :: P9_38 :: UNUSED_28     |
-- PIN_A19 :: P9_39 :: UNUSED_29            | PIN_B19 :: P9_40 :: UNUSED_30     |
-- PIN_A20 :: P9_41 :: WATCHDOG_INTERRUPT   | PIN_B20 :: P9_42 :: UNUSED_32     |
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
    WATCHDOG_INTERRUPT : out std_logic; -- PIN_A20 :: P9_41

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
    --
    -- Tri-state Unused Pins :: For the PCB Safety
    --
    UNUSED_01 : inout std_logic; -- PIN_A4
    UNUSED_02 : inout std_logic; -- PIN_B4
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
    UNUSED_32 : inout std_logic -- PIN_B20
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

component OffloadController
port
(    
    CLOCK_50MHz : in std_logic;

    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0)
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

component UartController
port
(
    CLOCK_50MHz : in std_logic;

    UART_BBB_TX : in std_logic;
    UART_BBB_RX : out std_logic;

    UART_x86_TX : out std_logic;
    UART_x86_RX : in std_logic
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

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin

----------------------------------------------------------
-- Tri-state Unused Pins :: For the PCB Safety
----------------------------------------------------------
UNUSED_01 <= 'Z';
UNUSED_02 <= 'Z';
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
	INTERRUPT_SIGNAL => WATCHDOG_INTERRUPT
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

OffloadController_module: OffloadController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    OFFLOAD_INTERRUPT => offload_interrupt,
    FIFO_DATA => primary_fifo_data_out,
    FIFO_READ_ENABLE => primary_fifo_rd_en,

    OFFLOAD_READY => open, --offload_ready,
    OFFLOAD_ID => open, --offload_id,
    OFFLOAD_REGISTER => open, --offload_register,
    OFFLOAD_CTRL => open, --offload_ctrl,
    OFFLOAD_DATA => open --offload_data
);

UartController_module: UartController
port map
(
    CLOCK_50MHz => CLOCK_50MHz,

    UART_BBB_TX => UART_BBB_TX,
    UART_BBB_RX => UART_BBB_RX,

    UART_x86_TX => UART_x86_TX,
    UART_x86_RX => UART_x86_RX
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

end rtl;
