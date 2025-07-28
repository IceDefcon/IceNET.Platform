library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

entity UartProcess is
port
(
    CLOCK_40MHz : in std_logic;
    RESET : in std_logic;

    UART_LOG_TRIGGER : in std_logic;
    UART_LOG_VECTOR : in std_logic_vector(31 downto 0);

    UART_PROCESS_RX : in std_logic;
    UART_PROCESS_TX : out std_logic
);
end UartProcess;

architecture rtl of UartProcess is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

    constant baudRate : std_logic_vector(7 downto 0) := "00011000"; -- 1/[25*20ns] ---> 2M Baud Rate

    signal UART_LOG_MESSAGE_ID : UART_LOG_ID := ("0000", "0000");
    signal UART_LOG_MESSAGE_KEY : UART_LOG_KEY := ("0000", "0000");
    signal UART_LOG_MESSAGE_DATA : UART_LOG_DATA := ("0000", "0000", "0000", "0000");

    signal uart_trigger_pulse : std_logic := '0';
    signal uart_trigger_stop : std_logic := '0';

    signal uart_write_enable : std_logic := '0';
    signal uart_write_symbol : std_logic_vector(6 downto 0) := (others => '0');
    signal uart_write_busy : std_logic := '0';

    signal uart_process_enable : std_logic := '0';
    signal uart_process_symbol : std_logic_vector(6 downto 0) := (others => '0');

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------

    component UartDataTransfer
    port
    (
        CLOCK_40MHz : in std_logic;
        RESET : in std_logic;

        WRITE_ENABLE : in std_logic;
        WRITE_SYMBOL : in std_logic_vector;

        FPGA_UART_RX : in std_logic;
        FPGA_UART_TX : out std_logic;

        WRITE_BUSY : out std_logic
    );
    end component;

    component UartDataAssembly
    generic
    (
        UART_CTRL : std_logic := '1'
    );
    port
    (
        CLOCK_40MHz : in std_logic;
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

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

short_process:
process(CLOCK_40MHz)
begin
    if rising_edge(CLOCK_40MHz) then
        if UART_LOG_TRIGGER = '1' and uart_trigger_stop = '0' then
            uart_trigger_pulse <= '1';
            uart_trigger_stop <= '1';
        elsif UART_LOG_TRIGGER = '1' and uart_trigger_stop = '1' then
            uart_trigger_pulse <= '0';
        else
            uart_trigger_stop <= '0';
        end if;
    end if;
end process;

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
    UART_CTRL => '0'
)
port map
(
    CLOCK_40MHz => CLOCK_40MHz,
    RESET => RESET,

    UART_LOG_TRIGGER => uart_trigger_pulse,
    UART_LOG_MESSAGE_ID => UART_LOG_MESSAGE_ID,
    UART_LOG_MESSAGE_KEY => UART_LOG_MESSAGE_KEY,
    UART_LOG_MESSAGE_DATA => UART_LOG_MESSAGE_DATA,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    WRITE_BUSY => uart_write_busy
);

UartDataTransfer_module: UartDataTransfer
port map
(
    CLOCK_40MHz => CLOCK_40MHz,
    RESET => RESET,

    WRITE_ENABLE => uart_write_enable,
    WRITE_SYMBOL => uart_write_symbol,

    FPGA_UART_RX => UART_PROCESS_RX,
    FPGA_UART_TX => UART_PROCESS_TX,

    WRITE_BUSY => uart_write_busy
);

end architecture;

