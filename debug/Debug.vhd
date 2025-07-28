library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

-----------------------------------------------------------------------------
--
-- PIN_B21  :: J504_CAT_CE          | PIN_B22  :: J504_CAT_MISO
-- PIN_C21  :: J504_CAT_MOSI        | PIN_C22  :: J504_CAT_CLK
-- PIN_D21  :: J504_EXT1            | PIN_D22  :: J504_EXT2
-- PIN_E21  :: J504_UART_TX         | PIN_E22  :: J504_UART_RX
--
-- ...
-- ...
-- ...
--
-- VIN     :: VIN
-- GND     :: GND
-- PIN_A20 :: PIN_B20
-- ______________________________________________________________________________________
--                      Λ                                                Λ
-- PIN_A19 :: PIN_B19   | TIMER_INT_FROM_FPGA   :: FPGA_UART_RX          | H7  :: H8
-- PIN_A18 :: PIN_B18   | ----===[ GND ]===---- :: FPGA_UART_TX          | H9  :: H10
-- PIN_A17 :: PIN_B17   |                       ::                       | H11 :: H12
-- PIN_A16 :: PIN_B16   | SECONDARY_SCLK        :: ----===[ GND ]===---- | H13 :: H14
-- PIN_A15 :: PIN_B15   | CTRL_F1               ::                       | H15 :: H16
-- PIN_A14 :: PIN_B14   | ----===[ 3V3 ]===---- :: SECONDARY_CS          | H17 :: H18
-- PIN_A13 :: PIN_B13   | PRIMARY_MOSI          :: ----===[ GND ]===---- | H19 :: H20
-- PIN_A10 :: PIN_B10   | PRIMARY_MISO          :: SECONDARY_MISO        | H21 :: H22
-- PIN_A9  :: PIN_B9    | PRIMARY_SCLK          :: PRIMARY_CS            | H23 :: H24
-- PIN_A8  :: PIN_B8    | ----===[ GND ]===---- ::                       | H25 :: H26
-- PIN_A7  :: PIN_B7    |                       ::                       | H27 :: H28
-- PIN_A6  :: PIN_B6    | SPI_INT_FROM_FPGA     :: ----===[ GND ]===---- | H29 :: H30
-- PIN_A5  :: PIN_B5    | WDG_INT_FROM_FPGA     :: RESET_FROM_CPU        | H31 :: H32
-- PIN_C3  :: PIN_C4    | CTRL_F2               :: ----===[ GND ]===---- | H33 :: H34
-- PIN_A4  :: PIN_B4    |                       ::                       | H35 :: H36
-- PIN_A3  :: PIN_B3    | SECONDARY_MOSI        ::                       | H37 :: H38
-- PIN_B2  :: PIN_B1    | ----===[ GND ]===---- ::                       | H39 :: H40
-- _____________________V________________________________________________V_______________
--

entity Debug is
Port
(
    CLOCK_50MHz : in std_logic; -- PIN_T2
    -----------------------------------------------------------------------------
    -- J504.B210
    -----------------------------------------------------------------------------
    J504_CAT_CE   : in std_logic; -- PIN_B21
    J504_CAT_MISO : in std_logic; -- PIN_B22
    J504_CAT_MOSI : in std_logic; -- PIN_C21
    J504_CAT_CLK  : in std_logic; -- PIN_C22
    J504_EXT1 : out std_logic; -- PIN_D21
    J504_EXT2 : out std_logic; -- PIN_D22
    J504_UART_TX : out std_logic; -- PIN_E21
    J504_UART_RX : in std_logic; -- PIN_E22
    -----------------------------------------------------------------------------
    -- JETSON.NANO
    -----------------------------------------------------------------------------
    PRIMARY_MOSI : in  std_logic; -- PIN_A13
    PRIMARY_MISO : out std_logic; -- PIN_A10
    PRIMARY_SCLK : in  std_logic; -- PIN_A9
    PRIMARY_CS   : in  std_logic; -- PIN_B9
    CTRL_F1 : in std_logic; -- PIN_A15
    CTRL_F2 : in std_logic; -- PIN_C3
    FPGA_UART_TX  : out std_logic; -- PIN_B18 :: H10 -> JetsonNano UART1_RXD
    FPGA_UART_RX  : in  std_logic; -- PIN_B19 :: H8  -> JetsonNano UART1_TXD
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
    LED_8 : out std_logic  -- PIN_N8
);
end Debug;

architecture rtl of Debug is

signal global_reset : std_logic := '1';

signal parallel_J504_CAT_MOSI : std_logic_vector(7 downto 0) := (others => '0');
signal parallel_J504_CAT_MISO : std_logic_vector(7 downto 0) := (others => '0');

signal parallel_MOSI_complete : std_logic := '0';
signal parallel_MISO_complete : std_logic := '0';

signal UART_LOG_TRIGGER : std_logic := '0';
signal UART_LOG_MESSAGE : std_logic_vector(31 downto 0) := (others => '0');

component TimedReset
Port
(
    MAIN_CLOCK : in  std_logic;
    TIMED_RESET : out std_logic
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

component UartProcess
port
(
    CLOCK_40MHz : in std_logic;
    RESET : in std_logic;

    UART_LOG_TRIGGER : in std_logic;
    UART_LOG_VECTOR : in std_logic_vector(31 downto 0);

    UART_PROCESS_RX : in std_logic;
    UART_PROCESS_TX : out std_logic
);
end component;

begin

    -- Console UART
    J504_UART_TX <= FPGA_UART_RX;
    FPGA_UART_TX <= J504_UART_RX;
    -- Kernel GPIO Control
    J504_EXT1 <= CTRL_F1;
    J504_EXT2 <= CTRL_F2;

    LED_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then
            if global_reset = '0' then
                LED_1 <= '0';
                LED_2 <= '1';
                LED_3 <= '1';
                LED_4 <= '0';
                LED_5 <= '0';
                LED_6 <= '1';
                LED_7 <= '1';
                LED_8 <= '0';
            else
                LED_1 <= '1';
                LED_2 <= '1';
                LED_3 <= '1';
                LED_4 <= '1';
                LED_5 <= '1';
                LED_6 <= '1';
                LED_7 <= '1';
                LED_8 <= '1';
            end if;
        end if;
    end process;

TimedReset_Main: TimedReset port map
(
    MAIN_CLOCK => CLOCK_50MHz,
    TIMED_RESET => global_reset
);

SpiConverter_MOSI: SpiConverter port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    CS => '0',
    SCLK => J504_CAT_CLK, -- Kernel Master always initialise SPI transfer

    SERIAL_MOSI => J504_CAT_MOSI, -- in :: Data from Kernel to Serialize
    PARALLEL_MOSI => parallel_J504_CAT_MOSI, -- out :: Serialized Data
    PARALLEL_MISO => "00011000", -- in :: 0x18 Hard coded Feedback to Serialize
    SERIAL_MISO => open, -- out :: 0x18 Serialized Hard coded Feedback to Kernel

    CONVERSION_BIT_COUNT => open,
    CONVERSION_COMPLETE => parallel_MOSI_complete -- Out :: Data byte is ready [FIFO Write Enable]
);

SpiConverter_MISO: SpiConverter port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    CS => '0',
    SCLK => J504_CAT_CLK,

    SERIAL_MOSI => J504_CAT_MISO, -- in :: Data from Kernel to Serialize
    PARALLEL_MOSI => parallel_J504_CAT_MISO, -- out :: Serialized Data
    PARALLEL_MISO => "00011000", -- in :: 0x18 Hard coded Feedback to Serialize
    SERIAL_MISO => open, -- out :: 0x18 Serialized Hard coded Feedback to Kernel

    CONVERSION_BIT_COUNT => open,
    CONVERSION_COMPLETE => parallel_MISO_complete -- Out :: Data byte is ready [FIFO Write Enable]
);

UART_LOG_TRIGGER <= parallel_MISO_complete or parallel_MOSI_complete;
UART_LOG_MESSAGE <= "00000000" & parallel_J504_CAT_MOSI & "00000000" & parallel_J504_CAT_MISO;

--UartProcess_Module: UartProcess
--port map
--(
--    CLOCK_40MHz => CLOCK_50MHz,
--    RESET => global_reset,

--    UART_LOG_TRIGGER => UART_LOG_TRIGGER,
--    UART_LOG_VECTOR => UART_LOG_MESSAGE,

--    UART_PROCESS_RX => FPGA_UART_RX,
--    UART_PROCESS_TX => FPGA_UART_TX
--);

end rtl;

