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

    UART_PROCESS_RX : in std_logic;
    UART_PROCESS_TX : out std_logic;

    VECTOR_INTERRUPT : out std_logic_vector(IRQ_VECTOR_SIZE - 1 downto 0)
);
end UartProcess;

architecture rtl of UartProcess is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

--constant baudRate : std_logic_vector(7 downto 0) := "00010011"; -- 1/[20*25ns] ---> 2M Baud Rate @ 40Mhz
constant baudRate : std_logic_vector(7 downto 0) := "00011000"; -- 1/[25*20ns] ---> 2M Baud Rate @ 50Mhz

signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("0000", "0000");
signal UART_LOG_MESSAGE_KEY : UART_LOG_KEY := ("0000", "0000");
signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("0000", "0000", "0000", "0000");

signal uart_write_enable : std_logic := '0';
signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_write_busy : std_logic := '0';

signal uart_process_enable : std_logic := '0';
signal uart_process_symbol : std_logic_vector(6 downto 0) := (others => '0');

signal uart_read_enable : std_logic := '0';
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

component UartTx
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector(6 downto 0);
    WRITE_BUSY : out std_logic;

    FPGA_UART_TX : out std_logic
);
end component;

component UartRx
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_ENABLE : out std_logic;
    READ_SYMBOL : out std_logic_vector(7 downto 0);
    READ_BUSY : out std_logic;

    FPGA_UART_RX : in std_logic
);
end component;

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

    WRITE_ENABLE : out std_logic;
    WRITE_SYMBOL : out std_logic_vector(6 downto 0);

    WRITE_BUSY : in std_logic
);
end component;

component IRQ_CONTROLLER
generic
(
    VECTOR_SIZE : integer := 4
);
port
(
    CLOCK : in  std_logic;
    RESET : in  std_logic;

    TRIGGER : in std_logic;
    COMMAND : in std_logic_vector(6 downto 0);

    VECTOR_INTERRUPT : out std_logic_vector(VECTOR_SIZE - 1 downto 0)
);
end component;

component UartOffloadController
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    UART_SYMBOL_READY : in std_logic;
    UART_SYMBOL_BYTE : in std_logic_vector(7 downto 0);
    UART_READ_BUSY : in std_logic;

    OFFLOAD_DATA_BYTE : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA_READY : out std_logic;

    FEEDBACK_DATA : out std_logic_vector(31 downto 0);
    FEEDBACK_TRIGGER : out std_logic
);
end component;

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

debug_trigger_process:
process(CLOCK)
begin
    if rising_edge(CLOCK) then
        if uart_counter = "10111110101111000010000000" then
            uart_counter <= (others => '0');
            uart_trigger <= '1';
        else
            uart_counter <= uart_counter + '1';
            uart_trigger <= '0';
        end if;
    end if;
end process;

--uart_message <= x"DEADC0DE";

--UART_LOG_MESSAGE_ID(0) <= uart_message(31 downto 28);
--UART_LOG_MESSAGE_ID(1) <= uart_message(27 downto 24);
--UART_LOG_MESSAGE_KEY(0) <= uart_message(23 downto 20);
--UART_LOG_MESSAGE_KEY(1) <= uart_message(19 downto 16);
--UART_LOG_MESSAGE_DATA(0) <= uart_message(15 downto 12);
--UART_LOG_MESSAGE_DATA(1) <= uart_message(11 downto 8);
--UART_LOG_MESSAGE_DATA(2) <= uart_message(7 downto 4);
--UART_LOG_MESSAGE_DATA(3) <= uart_message(3 downto 0);

UART_LOG_MESSAGE_ID(0) <= UART_FEEDBACK_VECTOR(31 downto 28);
UART_LOG_MESSAGE_ID(1) <= UART_FEEDBACK_VECTOR(27 downto 24);
UART_LOG_MESSAGE_KEY(0) <= UART_FEEDBACK_VECTOR(23 downto 20);
UART_LOG_MESSAGE_KEY(1) <= UART_FEEDBACK_VECTOR(19 downto 16);
UART_LOG_MESSAGE_DATA(0) <= UART_FEEDBACK_VECTOR(15 downto 12);
UART_LOG_MESSAGE_DATA(1) <= UART_FEEDBACK_VECTOR(11 downto 8);
UART_LOG_MESSAGE_DATA(2) <= UART_FEEDBACK_VECTOR(7 downto 4);
UART_LOG_MESSAGE_DATA(3) <= UART_FEEDBACK_VECTOR(3 downto 0);

UartDataAssembly_module: UartDataAssembly
generic map
(
    UART_CTRL => '1'
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    UART_LOG_TRIGGER => UART_FEEDBACK_TRIGGER,
    UART_LOG_MESSAGE_ID => UART_LOG_MESSAGE_ID,
    UART_LOG_MESSAGE_KEY => UART_LOG_MESSAGE_KEY,
    UART_LOG_MESSAGE_DATA => UART_LOG_MESSAGE_DATA,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    WRITE_BUSY => uart_write_busy
);

UartTx_module: UartTx
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,
    WRITE_BUSY => uart_write_busy,

    FPGA_UART_TX => UART_PROCESS_TX
);

UartRx_module: UartRx
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    READ_ENABLE => uart_read_enable,
    READ_SYMBOL => uart_read_symbol,
    READ_BUSY => uart_read_busy,

    FPGA_UART_RX => UART_PROCESS_RX
);

IRQ_CONTROLLER_module: IRQ_CONTROLLER
generic map
(
    VECTOR_SIZE => IRQ_VECTOR_SIZE
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    TRIGGER => uart_read_enable,
    COMMAND => uart_read_symbol(6 downto 0),

    VECTOR_INTERRUPT => VECTOR_INTERRUPT
);

UartOffloadController_module: UartOffloadController
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    UART_SYMBOL_READY => uart_read_enable,
    UART_SYMBOL_BYTE => uart_read_symbol,
    UART_READ_BUSY => uart_read_busy,

    OFFLOAD_DATA_BYTE => open,
    OFFLOAD_DATA_READY => open,

    FEEDBACK_DATA => UART_FEEDBACK_VECTOR,
    FEEDBACK_TRIGGER => UART_FEEDBACK_TRIGGER
);

end architecture;

