library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartController is
port
(
    CLOCK_50MHz : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_DATA : in std_logic_vector(6 downto 0);
    WRITE_LAST : in std_logic;

    UART_x86_TX : out std_logic;
    UART_x86_RX : in std_logic;

    UART_BUSY : out std_logic
);
end UartController;

architecture rtl of UartController is

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

constant baud_rate : std_logic_vector(7 downto 0) := "00011000"; -- 1/[25*20ns] ---> 2M Baud Rate

signal bit_count : integer := 0;
signal pause_count : std_logic_vector(12 downto 0) := (others => '0');
signal tick_count : std_logic_vector(7 downto 0) := (others => '0');

signal uart_out : std_logic := '1';

begin

state_machine_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case uart_state is

            when UART_IDLE =>
                if WRITE_ENABLE = '1' then
                    uart_state <= UART_CONFIG;
                end if;

            when UART_CONFIG =>
                if pause_count = "1001110001000" then
                    UART_BUSY <= '1';
                    pause_count <= (others => '0');
                    uart_state <= UART_START;
                else
                    pause_count <= pause_count + '1';
                end if;

            when UART_START =>
                uart_out <= '0';
                if tick_count = baud_rate then
                    tick_count <= (others => '0');
                    uart_state <= UART_WRITE;
                else
                    tick_count <= tick_count + '1';
                end if;

            when UART_WRITE =>
                if bit_count = 7 then
                    uart_out <= '0';
                    uart_state <= UART_STOP;
                    bit_count <= 0;
                else
                    if tick_count = baud_rate then
                        tick_count <= (others => '0');
                        bit_count <= bit_count + 1;
                    else
                        tick_count <= tick_count + '1';
                    end if;

                    uart_out <= WRITE_DATA(bit_count);

                end if;

            when UART_STOP =>
                if tick_count = baud_rate then
                    uart_out <= '1';
                    tick_count <= (others => '0');
                    uart_state <= UART_PAUSE;
                else
                    tick_count <= tick_count + '1';
                end if;

            when UART_PAUSE =>
                if tick_count = baud_rate then
                    uart_out <= '1';
                    tick_count <= (others => '0');
                    uart_state <= UART_DONE;
                else
                    tick_count <= tick_count + '1';
                end if;

            when UART_DONE =>
                UART_BUSY <= '0';
                uart_state <= UART_IDLE;

            when others =>
                uart_state <= UART_IDLE;

        end case;

        UART_x86_TX <= uart_out;

    end if;
end process;

end architecture;
