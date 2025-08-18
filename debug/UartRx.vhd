library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartRx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_ENABLE : in std_logic;
    READ_SYMBOL : out std_logic_vector(6 downto 0);
    READ_BUSY : out std_logic;

    FPGA_UART_RX : in std_logic
);
end UartRx;

architecture rtl of UartRx is

constant baudRate : std_logic_vector(7 downto 0) := "00011000"; -- 1/[25*20ns] ---> 2M Baud
constant bit_baud : integer range 0 to 32 := 25; -- 25*20ns ---> 2M Baud
constant bit_start : integer range 0 to 32 := 10;
constant bit_1 : integer range 0 to 32 := bit_start + bit_baud;
constant bit_2 : integer range 0 to 32 := bit_1 + bit_baud;
constant bit_3 : integer range 0 to 32 := bit_2 + bit_baud;
constant bit_4 : integer range 0 to 32 := bit_3 + bit_baud;
constant bit_5 : integer range 0 to 32 := bit_4 + bit_baud;
constant bit_6 : integer range 0 to 32 := bit_5 + bit_baud;
constant bit_7 : integer range 0 to 32 := bit_6 + bit_baud;
constant bit_stop : integer range 0 to 32 := bit_7 + bit_baud;

signal status_counter : std_logic_vector(8 downto 1) := (others)

signal bit_count : std_logic_vector(3 downto 0) := "0000";

type STATE is
(
    UART_IDLE,
    UART_CONFIG,
    UART_START,
    UART_READ,
    UART_STOP,
    UART_PAUSE,
    UART_DONE
);
signal uart_state: STATE := UART_IDLE;

--signal FPGA_UART_RX_d1 : std_logic := '0';
--signal FPGA_UART_RX_d2 : std_logic := '0';
--signal FPGA_UART_RX_d3 : std_logic := '0';
--signal FPGA_UART_RX_d4 : std_logic := '0';
--signal FPGA_UART_RX_delayed : std_logic := '0';

--signal REG_FPGA_UART_RX : std_logic_vector(2047 downto 0) := (others => '0');

--begin

--buffer_delay_process:
--process(CLOCK)
--begin
--    if rising_edge(CLOCK) then
--        FPGA_UART_RX_d1 <= FPGA_UART_RX;
--        FPGA_UART_RX_d2 <= FPGA_UART_RX_d1;
--        FPGA_UART_RX_d3 <= FPGA_UART_RX_d2;
--        FPGA_UART_RX_d4 <= FPGA_UART_RX_d3;
--        FPGA_UART_RX_delayed <= FPGA_UART_RX_d4;
--    end if;
--end process;

state_machine_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        uart_state <= UART_IDLE;
        --bit_number <= 0;
        --baud_count <= (others => '0');
        --uart_output <= '1';
        READ_BUSY <= '0';
    elsif rising_edge(CLOCK) then
        case uart_state is

            when UART_IDLE =>
                if READ_ENABLE = '1' then
                    uart_state <= UART_CONFIG;
                end if;

            when UART_CONFIG =>

            when UART_START =>

            when UART_READ =>

            when UART_STOP =>

            when UART_PAUSE =>

            when UART_DONE =>

            when others =>
                uart_state <= UART_IDLE;

        end case;


        if FPGA_UART_RX = '1' then
            detected_start <= '1';
        end if;

        if detected_start = '1' then
            status_counter <= status_counter + '1';
        elsif detected_start = '1' and detected_stop = '1' then
            status_counter <= (others => '0');
        end if;


    end if;
end process;
end architecture;
