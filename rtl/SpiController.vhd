library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SpiController is
generic
(
    PERIOD : integer := 20;
    LENGTH : integer := 16
);
Port
(
    CLOCK_50MHz : in  std_logic;
    INTERRUPT_SIGNAL : out std_logic
);
end entity SpiController;

architecture rtl of SpiController is

type SPI is
(
    IDLE,
    PRODUCE,
    DONE
);
signal spi_state: SPI := IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case spi_state is
                when IDLE =>

                when PRODUCE =>

                when DONE =>

                when others =>
                    spi_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
