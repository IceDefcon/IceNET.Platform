library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartController is
port
(
    CLOCK_50MHz : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_LAST  in std_logic;

    UART_BBB_TX : in std_logic;
    UART_BBB_RX : out std_logic;

    UART_x86_TX : out std_logic;
    UART_x86_RX : in std_logic
);
end UartController;

architecture rtl of UartController is

type STATE is
(
    UART_IDLE,
    UART_INIT,
    UART_START,
    UART_WRITE,
    UART_STOP,
    UART_DONE
);
signal uart_state: STATE := UART_IDLE;

constant baud_rate : std_logic_vector(7 downto 0) := "00110110"; -- 1/54*20ns ---> 925926 baud rate

signal bit_count : integer := 0;
signal stop_count : std_logic_vector(12 downto 0) := (others => '0');
signal tick_count : std_logic_vector(7 downto 0) := (others => '0');

signal test_tick : std_logic := '1';
signal test_data : std_logic_vector(6 downto 0) := "1000001";

begin

state_machine_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case uart_state is

            when UART_IDLE =>
                if WRITE_ENABLE = '1' then
                    uart_state <= UART_INIT;
                end if;

            when UART_INIT =>
                if stop_count = "1001110001000" then
                    stop_count <= (others => '0');
                    uart_state <= UART_START;
                else
                    stop_count <= stop_count + '1';
                end if;

            when UART_START =>
                test_tick <= '0';
                if tick_count = baud_rate then
                    tick_count <= (others => '0');
                    uart_state <= UART_WRITE;
                else
                    tick_count <= tick_count + '1';
                end if;

            when UART_WRITE =>
                if bit_count = 7 then
                    uart_state <= UART_STOP;
                    bit_count <= 0;
                else
                    if tick_count = baud_rate then
                        tick_count <= (others => '0');
                        bit_count <= bit_count + 1;
                    else
                        tick_count <= tick_count + '1';
                    end if;
                end if;

                test_tick <= test_data(bit_count);

            when UART_STOP =>
                test_tick <= '0';
                if tick_count = baud_rate then
                    tick_count <= (others => '0');
                    uart_state <= UART_DONE;
                else
                    tick_count <= tick_count + '1';
                end if;

            when UART_DONE =>
                test_tick <= '1';
                uart_state <= UART_IDLE;

            when others =>
                uart_state <= UART_IDLE;

        end case;

        UART_x86_TX <= test_tick;
        UART_BBB_RX <= UART_x86_RX;

    end if;
end process;

end architecture;
