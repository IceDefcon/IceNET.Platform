library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Temp is
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
end entity Temp;

architecture rtl of Temp is

type INTERRUPT is
(
    IDLE,
    PRODUCE,
    DONE
);
signal temp_state: INTERRUPT := IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case temp_state is
                when IDLE =>

                when PRODUCE =>

                when DONE =>

                when others =>
                    temp_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
