library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity NetworkAnalyser is
port
(
	clock 			: in std_logic;
	reset 			: in std_logic;

	led_0 			: out std_logic;
	led_1 			: out std_logic;
	led_2 			: out std_logic
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal nios_clk 				: std_logic;
signal nios_reset_n 			: std_logic;

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------



--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

led_0 <= '0'; 	-- D2
led_1 <= '1'; 	-- D4
led_2 <= '0'; 	-- D5

end rtl;

