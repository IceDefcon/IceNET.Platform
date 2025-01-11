library ieee;
use ieee.std_logic_1164.all;

entity UartController is
port
(
    CLOCK_50MHz : in std_logic;

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
    UART_CONFIG,
    UART_WRITE,
    UART_DONE
);
signal uart_state: STATE := UART_IDLE;

signal tick_count : std_logic_vector(7 downto 0) := (others => '0');
signal tick_start : std_logic := '0';


begin

uart_loopthrough_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        UART_x86_TX <= UART_BBB_TX;
        UART_BBB_RX <= UART_x86_RX;
    end if;
end process;

uart_state_machine_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case uart_state is

            when UART_IDLE =>
                uart_state <= UART_INIT;

            when UART_INIT =>
                uart_state <= UART_CONFIG;

            when UART_CONFIG =>
                uart_state <= UART_WRITE;

            when UART_WRITE =>
                uart_state <= UART_DONE;

            when UART_DONE =>
                uart_state <= DEVICE_INIT;

            when others =>
                uart_state <= UART_IDLE;

        end case;
    end if;
end process;

end architecture;
