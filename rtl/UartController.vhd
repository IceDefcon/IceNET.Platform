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

begin

uart_loopthrough_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        UART_x86_TX <= UART_BBB_TX;
        UART_BBB_RX <= UART_x86_RX;
    end if;
end process;

end architecture;
