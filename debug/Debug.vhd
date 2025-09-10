library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

-----------------------------------------------------------------------------
--
-- PIN_B21  :: J504_IN1     | PIN_B22  :: J504_IN2
-- PIN_C21  :: J504_IN3     | PIN_C22  :: J504_IN4
-- PIN_D21  :: J504_IN5     | PIN_D22  :: J504_IN6
-- PIN_E21  :: J504_OUT1    | PIN_E22  :: J504_OUT2
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
    J504_IN1 : in std_logic; -- PIN_B21
    J504_IN2 : in std_logic; -- PIN_B22
    J504_IN3 : in std_logic; -- PIN_C21
    J504_IN4 : in std_logic; -- PIN_C22
    J504_IN5 : in std_logic; -- PIN_D21
    J504_IN6 : in std_logic; -- PIN_D22
    J504_OUT1 : out std_logic; -- PIN_E21
    J504_OUT2 : out std_logic; -- PIN_E22
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
    LED_8 : out std_logic; -- PIN_N8
    -----------------------------------------------------------------------------
    -- UART
    -----------------------------------------------------------------------------
    DEBUG_UART_TX : out std_logic; -- PIN_P1
    DEBUG_UART_RX : in std_logic; -- PIN_R1

    DEBUG_PIN_5 : out std_logic; -- PIN_D2
    DEBUG_PIN_4 : out std_logic; -- PIN_F2
    DEBUG_PIN_3 : out std_logic; -- PIN_H2
    DEBUG_PIN_2 : out std_logic; -- PIN_J2
    DEBUG_PIN_1 : out std_logic; -- PIN_M2
    DEBUG_PIN_0 : out std_logic  -- PIN_N2
);
end Debug;

architecture rtl of Debug is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

signal global_reset : std_logic := '1';

signal synced_J504_IN1 : std_logic := '0';
signal synced_J504_IN2 : std_logic := '0';
signal synced_J504_IN3 : std_logic := '0';
signal synced_J504_IN4 : std_logic := '0';
signal synced_J504_IN5 : std_logic := '0';
signal synced_J504_IN6 : std_logic := '0';

signal short_J504_IN1 : std_logic := '0';
signal short_J504_IN2 : std_logic := '0';
signal short_J504_IN3 : std_logic := '0';
signal short_J504_IN4 : std_logic := '0';
signal short_J504_IN5 : std_logic := '0';
signal short_J504_IN6 : std_logic := '0';

signal uart_busy : std_logic := '0';
signal uart_trigger : std_logic := '0';
signal uart_counter : std_logic_vector(31 downto 0) := (others => '0');
signal uart_message : std_logic_vector(31 downto 0) := (others => '0');

signal uart_debug_vector : std_logic_vector(5 downto 0) := (others => '0');

signal debug_led : std_logic_vector(5 downto 0) := (others => '0');

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------

component TimedReset
Port
(
    CLOCK : in  std_logic;
    TIMED_RESET : out std_logic
);
end component;

component DelaySynchroniser
generic
(
    SYNCHRONIZATION_DEPTH : integer := 2
);
Port
(
    CLOCK : in  std_logic;
    RESET : in std_logic;

    ASYNC_INPUT : in std_logic;
    SYNC_OUTPUT : out std_logic
);
end component;

component PulseController
generic
(
    PULSE_LENGTH : integer := 1
);
Port
(
    CLOCK : in  std_logic;
    RESET : in std_logic;

    ENABLE_CONTROLLER : in std_logic;

    INPUT_PULSE : in std_logic;
    OUTPUT_PULSE : out std_logic
);
end component;

component UartProcess
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    UART_LOG_TRIGGER : in std_logic;
    UART_LOG_VECTOR : in std_logic_vector(31 downto 0);

    UART_PROCESS_RX : in std_logic;
    UART_PROCESS_TX : out std_logic;

    WRITE_BUSY : out std_logic;

    DEBUG_INTERRUPT : out std_logic_vector(5 downto 0);
    UART_DEBUG  : out std_logic_vector(5 downto 0)
);
end component;

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

-- Kernel GPIO Control
J504_OUT1 <= CTRL_F1;
J504_OUT2 <= CTRL_F2;

------------------------------------------------------------------------------------------------------------
-- Global Reset
------------------------------------------------------------------------------------------------------------

TimedReset_Main: TimedReset port map
(
    CLOCK => CLOCK_50MHz,
    TIMED_RESET => global_reset
);

------------------------------------------------------------------------------------------------------------
-- Synchroniser
------------------------------------------------------------------------------------------------------------

DelaySynchroniser_J504_IN1: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN1,
    SYNC_OUTPUT => synced_J504_IN1
);

DelaySynchroniser_J504_IN2: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN2,
    SYNC_OUTPUT => synced_J504_IN2
);

DelaySynchroniser_J504_IN3: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN3,
    SYNC_OUTPUT => synced_J504_IN3
);

DelaySynchroniser_J504_IN4: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN4,
    SYNC_OUTPUT => synced_J504_IN4
);

DelaySynchroniser_J504_IN5: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN5,
    SYNC_OUTPUT => synced_J504_IN5
);

DelaySynchroniser_J504_IN6: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ASYNC_INPUT => J504_IN6,
    SYNC_OUTPUT => synced_J504_IN6
);

------------------------------------------------------------------------------------------------------------
-- Pulse Contoller
------------------------------------------------------------------------------------------------------------

PulseController_J504_IN1: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN1,
    OUTPUT_PULSE => short_J504_IN1
);

PulseController_J504_IN2: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN2,
    OUTPUT_PULSE => short_J504_IN2
);

PulseController_J504_IN3: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN3,
    OUTPUT_PULSE => short_J504_IN3
);

PulseController_J504_IN4: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN4,
    OUTPUT_PULSE => short_J504_IN4
);

PulseController_J504_IN5: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN5,
    OUTPUT_PULSE => short_J504_IN5
);

PulseController_J504_IN6: PulseController
generic map
(
    PULSE_LENGTH => 1 -- 1*20ns Pulse
)
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,

    ENABLE_CONTROLLER => '1',

    INPUT_PULSE => synced_J504_IN6,
    OUTPUT_PULSE => short_J504_IN6
);

------------------------------------------------------------------------------------------------------------
-- UART Contoller
------------------------------------------------------------------------------------------------------------

trigger_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if uart_counter = "10111110101111000010000000" then
            uart_counter <= (others => '0');
            uart_trigger <= '1';
        else
            uart_counter <= uart_counter + '1';
            uart_trigger <= '0';
        end if;
    end if;
end process;

uart_message <= x"DEADC0DE";

UartProcess_Module: UartProcess
port map
(
    CLOCK => CLOCK_50MHz,
    RESET => global_reset,
    -- IN
    UART_LOG_TRIGGER => uart_trigger,
    UART_LOG_VECTOR => uart_message,
    -- BOTH
    UART_PROCESS_RX => FPGA_UART_RX,
    UART_PROCESS_TX => FPGA_UART_TX,
    -- OUT
    WRITE_BUSY => uart_busy,

    DEBUG_INTERRUPT => debug_led,
    UART_DEBUG => uart_debug_vector
);


------------------------------------------------------------------------------------------------------------
-- DEBUG
------------------------------------------------------------------------------------------------------------

LED_process:
process(CLOCK_50MHz, global_reset)
begin
    if rising_edge(CLOCK_50MHz) then
        if global_reset = '1' then 
            LED_1 <= '1';
            LED_2 <= '1';
            LED_3 <= '1';
            LED_4 <= '1';
            LED_5 <= '1';
            LED_6 <= '1';
            LED_7 <= '1';
            LED_8 <= '1';
        else
            LED_1 <= '1';
            LED_2 <= not debug_led(0);
            LED_3 <= not debug_led(1);
            LED_4 <= not debug_led(2);
            LED_5 <= not debug_led(3);
            LED_6 <= not debug_led(4);
            LED_7 <= not debug_led(5);
            LED_8 <= '1';
        end if;
    end if;
end process;

DEBUG_PIN_5 <= uart_debug_vector(5);
DEBUG_PIN_4 <= uart_debug_vector(4);
DEBUG_PIN_3 <= uart_debug_vector(3);
DEBUG_PIN_2 <= uart_debug_vector(2);
DEBUG_PIN_1 <= uart_debug_vector(1);
DEBUG_PIN_0 <= uart_debug_vector(0);

end rtl;

