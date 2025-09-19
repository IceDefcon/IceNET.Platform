library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;

entity UartTx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector(7 downto 0);
    WRITE_BUSY : out std_logic;

    FPGA_UART_TX : out std_logic
);
end UartTx;

architecture rtl of UartTx is

constant baudRate : integer := 25; -- 1/[25*4ns] ---> 2M @ 50Mhz

type STATE is
(
    UART_IDLE,
    UART_CONFIG,
    UART_START,
    UART_WRITE,
    UART_STOP,
    UART_PAUSE,
    UART_DONE
);
signal uart_state: STATE := UART_IDLE;

signal bit_number : integer := 0;
signal baud_count : integer range 0 to 256 := 0;
signal uart_output : std_logic := '1';

begin

state_machine_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        uart_state <= UART_IDLE;
        bit_number <= 0;
        baud_count <= 0;
        uart_output <= '1';
        FPGA_UART_TX <= '1';
        WRITE_BUSY <= '0';
    elsif rising_edge(CLOCK) then
        case uart_state is

            when UART_IDLE =>
                if WRITE_ENABLE = '1' then
                    uart_state <= UART_CONFIG;
                end if;

            when UART_CONFIG =>
                WRITE_BUSY <= '1';
                uart_state <= UART_START;

            when UART_START =>
                uart_output <= '0';
                if baud_count = baudRate then
                    baud_count <= 0;
                    uart_state <= UART_WRITE;
                else
                    baud_count <= baud_count + 1;
                end if;

            when UART_WRITE =>
                if bit_number = 7 then
                    uart_output <= '0';
                    uart_state <= UART_STOP;
                    bit_number <= 0;
                else
                    if baud_count = baudRate then
                        baud_count <= 0;
                        bit_number <= bit_number + 1;
                    else
                        baud_count <= baud_count + 1;
                    end if;

                    uart_output <= WRITE_SYMBOL(bit_number);

                end if;

            when UART_STOP =>
                if baud_count = baudRate then
                    uart_output <= '1';
                    baud_count <= 0;
                    uart_state <= UART_PAUSE;
                else
                    baud_count <= baud_count + 1;
                end if;

            when UART_PAUSE =>
                if baud_count = baudRate then
                    uart_output <= '1';
                    baud_count <= 0;
                    uart_state <= UART_DONE;
                else
                    baud_count <= baud_count + 1;
                end if;

            when UART_DONE =>
                WRITE_BUSY <= '0';
                uart_state <= UART_IDLE;

            when others =>
                uart_state <= UART_IDLE;

        end case;

        FPGA_UART_TX <= uart_output;

    end if;
end process;

end architecture;
