library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.Types.all;

entity UartDataAssembly is
port
(
    CLOCK_50MHz : in std_logic;

    UART_LOG_MESSAGE_ID : in UART_LOG_ID;
    UART_LOG_MESSAGE_DATA : in UART_LOG_DATA;

    WRITE_ENABLE : out std_logic;
    WRITE_SYMBOL : out std_logic_vector(6 downto 0);

    WRITE_BUSY : in std_logic
);
end UartDataAssembly;

architecture rtl of UartDataAssembly is

constant uart_length : integer := 8;

type uart_type is array (0 to 7) of std_logic_vector(6 downto 0);
signal uart_tx : uart_type;

type STATE is
(
    WRITE_IDLE,
    WRITE_INIT,
    WRITE_CONFIG,
    WRITE_WAIT,
    WRITE_TRANSFER,
    WRITE_CHECK,
    WRITE_DONE
);
signal uart_state: STATE := WRITE_IDLE;
signal delay_timer : std_logic_vector(25 downto 0) := (others => '0');
signal uart_byte : integer := 0;

begin

    uart_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case uart_state is
                when WRITE_IDLE =>
                    if delay_timer = "10111110101111000001111111" then
                        delay_timer <= (others => '0');
                        uart_state <= WRITE_INIT;
                    else
                        delay_timer <= delay_timer + '1';
                    end if;

                when WRITE_INIT =>
                    uart_tx(0) <= UART_LOG_MESSAGE_ID(0);
                    uart_tx(1) <= UART_LOG_MESSAGE_ID(1);
                    uart_tx(2) <= ASCII_SPACE;
                    uart_tx(3) <= UART_LOG_MESSAGE_DATA(0);
                    uart_tx(4) <= UART_LOG_MESSAGE_DATA(1);
                    uart_tx(5) <= UART_LOG_MESSAGE_DATA(2);
                    uart_tx(6) <= UART_LOG_MESSAGE_DATA(3);
                    uart_tx(7) <= ASCII_LF;
                    uart_state <= WRITE_CONFIG;

                when WRITE_CONFIG =>
                    WRITE_ENABLE <= '0';
                    if uart_byte = uart_length then
                        uart_byte <= 0;
                        uart_state <= WRITE_DONE;
                    else
                        uart_state <= WRITE_WAIT;
                    end if;

                when WRITE_WAIT =>
                    uart_state <= WRITE_TRANSFER;

                when WRITE_TRANSFER =>
                    if WRITE_BUSY = '0' then
                        WRITE_SYMBOL <= uart_tx(uart_byte);
                        uart_byte <= uart_byte + 1;
                        uart_state <= WRITE_CHECK;
                    end if;

                when WRITE_CHECK =>
                    WRITE_ENABLE <= '1';
                    uart_state <= WRITE_CONFIG;

                when WRITE_DONE =>
                    uart_state <= WRITE_IDLE;

                when others =>
                    uart_state <= WRITE_IDLE;

            end case;
        end if;
    end process;

end architecture;
