library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

entity UartProcess is
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
signal uart_read_symbol : std_logic_vector(6 downto 0) := (others => '0');
signal uart_read_busy : std_logic := '0';

signal uart_debug_vector : std_logic_vector(5 downto 0) := (others => '0');

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
    READ_SYMBOL : out std_logic_vector(6 downto 0);
    READ_BUSY : out std_logic;

    FPGA_UART_RX : in std_logic;
    UART_DEBUG  : out std_logic_vector(5 downto 0)
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

    VECTOR_INTERRUPT : out std_logic_vector(5 downto 0)
);
end component;

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

UART_LOG_MESSAGE_ID(0) <= UART_LOG_VECTOR(31 downto 28);
UART_LOG_MESSAGE_ID(1) <= UART_LOG_VECTOR(27 downto 24);
UART_LOG_MESSAGE_KEY(0) <= UART_LOG_VECTOR(23 downto 20);
UART_LOG_MESSAGE_KEY(1) <= UART_LOG_VECTOR(19 downto 16);
UART_LOG_MESSAGE_DATA(0) <= UART_LOG_VECTOR(15 downto 12);
UART_LOG_MESSAGE_DATA(1) <= UART_LOG_VECTOR(11 downto 8);
UART_LOG_MESSAGE_DATA(2) <= UART_LOG_VECTOR(7 downto 4);
UART_LOG_MESSAGE_DATA(3) <= UART_LOG_VECTOR(3 downto 0);

UartDataAssembly_module: UartDataAssembly
generic map
(
    UART_CTRL => '1'
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    UART_LOG_TRIGGER => UART_LOG_TRIGGER,
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

    WRITE_ENABLE => uart_read_enable,
    WRITE_SYMBOL => uart_read_symbol,
    WRITE_BUSY => uart_write_busy,

    FPGA_UART_TX => UART_PROCESS_TX
);

WRITE_BUSY <= uart_write_busy;

UartRx_module: UartRx
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    READ_ENABLE => uart_read_enable,
    READ_SYMBOL => uart_read_symbol,
    READ_BUSY => uart_read_busy,

    FPGA_UART_RX => UART_PROCESS_RX,
    UART_DEBUG => uart_debug_vector
);

IRQ_CONTROLLER_module: IRQ_CONTROLLER
generic map
(
    VECTOR_SIZE => 6
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    TRIGGER => uart_read_enable,
    COMMAND => uart_read_symbol,

    VECTOR_INTERRUPT => DEBUG_INTERRUPT
);

UART_DEBUG <= uart_debug_vector;

end architecture;

