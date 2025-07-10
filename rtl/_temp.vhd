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
    RESET : in  std_logic;

    INTERRUPT_SIGNAL : out std_logic
);
end entity Temp;

architecture rtl of Temp is

type TEMP_SM is
(
    TEMP_IDLE,
    TEMP_PRODUCE,
    TEMP_DONE
);
signal temp_state: TEMP_SM := TEMP_IDLE;

begin

    temp_process:
    process(CLOCK_50MHz)
    begin
        if RESET = '1' then
            temp_state <= TEMP_IDLE
        elsif rising_edge(CLOCK_50MHz) then

            case temp_state is
                when TEMP_IDLE =>

                when TEMP_PRODUCE =>

                when TEMP_DONE =>

                when others =>
                    temp_state <= TEMP_IDLE;
            end case;
        end if;
    end process;

end architecture rtl;
