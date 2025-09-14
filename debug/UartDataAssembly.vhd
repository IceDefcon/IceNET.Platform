library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.UartTypes.all;

entity UartDataAssembly is
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
end UartDataAssembly;

architecture rtl of UartDataAssembly is

constant uart_length : integer := 8;

type uart_type is array (0 to 7) of std_logic_vector(6 downto 0);
signal uart_tx : uart_type;

type ASSEMBLER_STATE is
(
    ASSEMBLER_IDLE,
    ASSEMBLER_INIT,
    ASSEMBLER_CONFIG,
    ASSEMBLER_WAIT,
    ASSEMBLER_TRANSFER,
    ASSEMBLER_CHECK,
    ASSEMBLER_DONE
);
signal uart_state: ASSEMBLER_STATE := ASSEMBLER_IDLE;
signal uart_byte : integer := 0;

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

signal synced_UART_LOG_TRIGGER : std_logic := '0';
begin

DelaySynchroniser_LOG_TRIGGER: DelaySynchroniser
generic map
(
    SYNCHRONIZATION_DEPTH => 2
)
port map
(
    CLOCK => CLOCK,
    RESET => RESET,

    ASYNC_INPUT => UART_LOG_TRIGGER,
    SYNC_OUTPUT => synced_UART_LOG_TRIGGER
);

    uart_process:
    process(CLOCK, RESET)
    begin
        if RESET = '1' then
            uart_state <= ASSEMBLER_IDLE;
            uart_tx <= (others => (others => '0'));
            uart_byte <= 0;
            WRITE_ENABLE <= '0';
            WRITE_SYMBOL <= (others => '0');
        elsif rising_edge(CLOCK) then

            case uart_state is
                when ASSEMBLER_IDLE =>
                    if synced_UART_LOG_TRIGGER = '1' then
                        uart_state <= ASSEMBLER_INIT;
                    end if;

                when ASSEMBLER_INIT =>
                    if UART_CTRL = '1' then
                        uart_tx(0) <= HEX_TO_ASCII(UART_LOG_MESSAGE_ID(0));
                        uart_tx(1) <= HEX_TO_ASCII(UART_LOG_MESSAGE_ID(1));
                        uart_tx(2) <= HEX_TO_ASCII(UART_LOG_MESSAGE_KEY(0));
                        uart_tx(3) <= HEX_TO_ASCII(UART_LOG_MESSAGE_KEY(1));
                        uart_tx(4) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(0));
                        uart_tx(5) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(1));
                        uart_tx(6) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(2));
                        uart_tx(7) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(3));
                        uart_state <= ASSEMBLER_CONFIG;
                    elsif UART_CTRL = '0' then
                        uart_tx(0) <= HEX_TO_ASCII(UART_LOG_MESSAGE_ID(0));
                        uart_tx(1) <= HEX_TO_ASCII(UART_LOG_MESSAGE_ID(1));
                        uart_tx(2) <= HEX_TO_ASCII(UART_LOG_MESSAGE_KEY(0));
                        uart_tx(3) <= HEX_TO_ASCII(UART_LOG_MESSAGE_KEY(1));
                        uart_tx(4) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(0));
                        uart_tx(5) <= HEX_TO_ASCII(UART_LOG_MESSAGE_DATA(1));
                        uart_tx(6) <= ASCII_CR;
                        uart_tx(7) <= ASCII_LF;
                        uart_state <= ASSEMBLER_CONFIG;
                    end if;

                when ASSEMBLER_CONFIG =>
                    WRITE_ENABLE <= '0';
                    if uart_byte = uart_length then
                        uart_byte <= 0;
                        uart_state <= ASSEMBLER_DONE;
                    else
                        uart_state <= ASSEMBLER_WAIT;
                    end if;

                when ASSEMBLER_WAIT =>
                    uart_state <= ASSEMBLER_TRANSFER;

                when ASSEMBLER_TRANSFER =>
                    if WRITE_BUSY = '0' then
                        WRITE_SYMBOL <= uart_tx(uart_byte);
                        uart_byte <= uart_byte + 1;
                        uart_state <= ASSEMBLER_CHECK;
                    end if;

                when ASSEMBLER_CHECK =>
                    WRITE_ENABLE <= '1';
                    uart_state <= ASSEMBLER_CONFIG;

                when ASSEMBLER_DONE =>
                    uart_state <= ASSEMBLER_IDLE;

                when others =>
                    uart_state <= ASSEMBLER_IDLE;

            end case;
        end if;
    end process;

end architecture;
