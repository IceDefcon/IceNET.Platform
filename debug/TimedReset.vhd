library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity TimedReset is
port
(
    MAIN_CLOCK : in std_logic;
    TIMED_RESET : out std_logic
);
end TimedReset;

architecture rtl of TimedReset is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

constant reset_trigger : std_logic_vector(25 downto 0) := "10111110101111000010000000"; -- 50Mhz :: 1s
--constant reset_trigger : std_logic_vector(25 downto 0) := "10011000100101101000000000"; -- 40Mhz :: 1s
signal reset_counter : std_logic_vector(25 downto 0) := (others => '0');
signal reset_reg : std_logic := '1';

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

reset_process:
process(MAIN_CLOCK)
begin
    if rising_edge(MAIN_CLOCK) then
        if reset_counter = reset_trigger then
            reset_reg <= '0';
        else
            reset_counter <= reset_counter + '1';
            reset_reg <= '1';
        end if;
    end if;
end process;

TIMED_RESET <= reset_reg;

end architecture;
