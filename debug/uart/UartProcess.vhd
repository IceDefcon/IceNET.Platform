library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

entity UartProcess is
generic
(
    UART_CTRL : std_logic := '0';
    IRQ_VECTOR_SIZE : integer := 10
);
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    UART_LOG_TRIGGER : in std_logic;
    UART_LOG_VECTOR : in std_logic_vector(31 downto 0);

    SYNCED_UART_RX : in std_logic;
    SYNCED_UART_TX : out std_logic;

    WRITE_BUSY : out std_logic;

    VECTOR_INTERRUPT : out std_logic_vector(IRQ_VECTOR_SIZE - 1 downto 0)
);
end UartProcess;

architecture rtl of UartProcess is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

signal UART_LOG_MESSAGE_TRIGGER : std_logic := '0';
signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("0000", "0000");
signal UART_LOG_MESSAGE_KEY : UART_LOG_KEY := ("0000", "0000");
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("0000", "0000", "0000", "0000");

signal uart_write_valid : std_logic := '0';
signal uart_write_symbol : std_logic_vector(7 downto 0) := (others => '0');

signal uart_read_valid : std_logic := '0';
signal uart_read_symbol : std_logic_vector(7 downto 0) := (others => '0');
signal uart_read_busy : std_logic := '0';

signal UART_FEEDBACK_VECTOR : std_logic_vector(31 downto 0) := (others => '0');
signal UART_FEEDBACK_TRIGGER : std_logic := '0';

signal uart_trigger : std_logic := '0';
signal uart_counter : std_logic_vector(31 downto 0) := (others => '0');
signal uart_message : std_logic_vector(31 downto 0) := (others => '0');

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------
component UartDataAssembly
generic
(
    UART_CTRL : std_logic := '0'
);
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    UART_LOG_TRIGGER : in std_logic;
    UART_LOG_MESSAGE_ID : in UART_LOG_ID;
    UART_LOG_MESSAGE_KEY : in UART_LOG_KEY;
    UART_LOG_MESSAGE_DATA : in UART_LOG_DATA;

    WRITE_VALID : out std_logic;
    WRITE_SYMBOL : out std_logic_vector(7 downto 0);

    WRITE_BUSY : in std_logic
);
end component;

component UartTx
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector(7 downto 0);
    WRITE_BUSY : out std_logic;

    SYNCED_UART_TX : out std_logic
);
end component;

component UartRx
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_VALID : out std_logic;
    READ_SYMBOL : out std_logic_vector(7 downto 0);
    READ_BUSY : out std_logic;

    SYNCED_UART_RX : in std_logic
);
end component;

component UartIrqController
generic
(
    VECTOR_SIZE : integer := 10
);
port
(
    CLOCK : in  std_logic;
    RESET : in  std_logic;

    VECTOR_TRIGGER : in std_logic;
    VECTOR_BYTE : in std_logic_vector(7 downto 0);

    VECTOR_INTERRUPT : out std_logic_vector(VECTOR_SIZE - 1 downto 0);

    PARAMETER_READY : out std_logic_vector(PARAMETER_NUMBER - 1 downto 0);
    PARAMETER_MATRIX : out PARAMETER_ARRAY;

    FEEDBACK_DATA : out std_logic_vector(31 downto 0);
    FEEDBACK_TRIGGER : out std_logic
);
end component;

begin

------------------------------------------------------------------------------------------------------------
-- UART Debug
------------------------------------------------------------------------------------------------------------

--debug_trigger_process:
--process(CLOCK)
--begin
--    if rising_edge(CLOCK) then
--        if uart_counter = "10111110101111000010000000" then
--            uart_counter <= (others => '0');
--            uart_trigger <= '1';
--        else
--            uart_counter <= uart_counter + '1';
--            uart_trigger <= '0';
--        end if;
--    end if;
--end process;

--uart_message <= x"DEADC0DE";

--UART_LOG_MESSAGE_ID(0) <= uart_message(31 downto 28);
--UART_LOG_MESSAGE_ID(1) <= uart_message(27 downto 24);
--UART_LOG_MESSAGE_KEY(0) <= uart_message(23 downto 20);
--UART_LOG_MESSAGE_KEY(1) <= uart_message(19 downto 16);
--UART_LOG_MESSAGE_DATA(0) <= uart_message(15 downto 12);
--UART_LOG_MESSAGE_DATA(1) <= uart_message(11 downto 8);
--UART_LOG_MESSAGE_DATA(2) <= uart_message(7 downto 4);
--UART_LOG_MESSAGE_DATA(3) <= uart_message(3 downto 0);

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------
uart_switch_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        UART_LOG_MESSAGE_TRIGGER <= '0';
        UART_LOG_MESSAGE_ID(0) <= "0000";
        UART_LOG_MESSAGE_ID(1) <= "0000";
        UART_LOG_MESSAGE_KEY(0) <= "0000";
        UART_LOG_MESSAGE_KEY(1) <= "0000";
        UART_LOG_MESSAGE_DATA(0) <= "0000";
        UART_LOG_MESSAGE_DATA(1) <= "0000";
        UART_LOG_MESSAGE_DATA(2) <= "0000";
        UART_LOG_MESSAGE_DATA(3) <= "0000";
    elsif rising_edge(CLOCK) then
        if UART_LOG_TRIGGER = '1' then
            UART_LOG_MESSAGE_TRIGGER <= '1';
            UART_LOG_MESSAGE_ID(0) <= UART_LOG_VECTOR(31 downto 28);
            UART_LOG_MESSAGE_ID(1) <= UART_LOG_VECTOR(27 downto 24);
            UART_LOG_MESSAGE_KEY(0) <= UART_LOG_VECTOR(23 downto 20);
            UART_LOG_MESSAGE_KEY(1) <= UART_LOG_VECTOR(19 downto 16);
            UART_LOG_MESSAGE_DATA(0) <= UART_LOG_VECTOR(15 downto 12);
            UART_LOG_MESSAGE_DATA(1) <= UART_LOG_VECTOR(11 downto 8);
            UART_LOG_MESSAGE_DATA(2) <= UART_LOG_VECTOR(7 downto 4);
            UART_LOG_MESSAGE_DATA(3) <= UART_LOG_VECTOR(3 downto 0);
        elsif UART_FEEDBACK_TRIGGER = '1' then
            UART_LOG_MESSAGE_TRIGGER <= '1';
            UART_LOG_MESSAGE_ID(0) <= UART_FEEDBACK_VECTOR(31 downto 28);
            UART_LOG_MESSAGE_ID(1) <= UART_FEEDBACK_VECTOR(27 downto 24);
            UART_LOG_MESSAGE_KEY(0) <= UART_FEEDBACK_VECTOR(23 downto 20);
            UART_LOG_MESSAGE_KEY(1) <= UART_FEEDBACK_VECTOR(19 downto 16);
            UART_LOG_MESSAGE_DATA(0) <= UART_FEEDBACK_VECTOR(15 downto 12);
            UART_LOG_MESSAGE_DATA(1) <= UART_FEEDBACK_VECTOR(11 downto 8);
            UART_LOG_MESSAGE_DATA(2) <= UART_FEEDBACK_VECTOR(7 downto 4);
            UART_LOG_MESSAGE_DATA(3) <= UART_FEEDBACK_VECTOR(3 downto 0);
        else
            UART_LOG_MESSAGE_TRIGGER <= '0';
        end if;
    end if;
end process;

UartDataAssembly_module: UartDataAssembly
generic map
(
    UART_CTRL => UART_CTRL
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    UART_LOG_TRIGGER => UART_LOG_MESSAGE_TRIGGER,
    UART_LOG_MESSAGE_ID => UART_LOG_MESSAGE_ID,
    UART_LOG_MESSAGE_KEY => UART_LOG_MESSAGE_KEY,
    UART_LOG_MESSAGE_DATA => UART_LOG_MESSAGE_DATA,

    WRITE_VALID => uart_write_valid,
    WRITE_SYMBOL => uart_write_symbol,

    WRITE_BUSY => uart_write_busy
);

UartTx_module: UartTx
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    WRITE_ENABLE => uart_write_valid,
    WRITE_SYMBOL => uart_write_symbol,
    WRITE_BUSY => uart_write_busy,

    SYNCED_UART_TX => SYNCED_UART_TX
);

WRITE_BUSY <= uart_write_busy;

UartRx_module: UartRx
port map
(
    CLOCK => CLOCK,
    RESET => RESET,
    -- OUT
    READ_VALID => uart_read_valid,
    READ_SYMBOL => uart_read_symbol,
    READ_BUSY => uart_read_busy,
    -- IN
    SYNCED_UART_RX => SYNCED_UART_RX
);

UartIrqController_module: UartIrqController
generic map
(
    VECTOR_SIZE => IRQ_VECTOR_SIZE
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,
    -- IN
    VECTOR_TRIGGER => uart_read_valid,
    VECTOR_BYTE => uart_read_symbol,
    -- OUT
    VECTOR_INTERRUPT => VECTOR_INTERRUPT,
    -- OUT
    PARAMETER_READY => open,
    PARAMETER_MATRIX => open,
    -- OUT
    FEEDBACK_DATA => UART_FEEDBACK_VECTOR,
    FEEDBACK_TRIGGER => UART_FEEDBACK_TRIGGER
);

end architecture;

