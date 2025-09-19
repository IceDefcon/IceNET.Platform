library ieee;
use ieee.std_logic_1164.all;

entity TimedReset is
port
(
    CLOCK : in  std_logic;
    RESET : out std_logic
);
end TimedReset;

architecture rtl of TimedReset is

constant RESET_TRIGGER : integer := 100000000; -- 250 MHz :: 3s

signal reset_counter : integer range 0 to RESET_TRIGGER := 0;
signal reset_reg     : std_logic := '1'; -- active high reset at start

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------
begin

reset_process : process(CLOCK)
begin
    if rising_edge(CLOCK) then
        if reset_reg = '1' then
            if reset_counter = RESET_TRIGGER then
                reset_reg <= '0'; -- release reset after 3s
            else
                reset_counter <= reset_counter + 1;
            end if;
        end if;
    end if;
end process;

RESET <= reset_reg;

end rtl;
