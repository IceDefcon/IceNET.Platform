library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity NetworkAnalyser is
port
(
	CLOCK 			: in std_logic; 	-- PIN_17
	
	LED_0 			: out std_logic; 	-- PIN_3
	LED_1 			: out std_logic; 	-- PIN_7
	LED_2 			: out std_logic;	-- PIN_9

	BUTTON_IN 			: in std_logic; 	-- PIN_144
	GPIO_IN 				: in std_logic; 	-- PIN_143
	GPIO_OUT 			: out std_logic 	-- PIN_142
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal button_debounced	: std_logic := '1';

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------
component debounce
port
(
	clock : in  std_logic;
	button_in : in  std_logic;
	button_out : out  std_logic
);
end component;

--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

LED_0 <= '0'; 	-- D2 Low Enable :: To check if FPGA is configured

debounce_module: debounce port map 
(
	clock => CLOCK,
	button_in => BUTTON_IN,
	button_out => button_debounced
);

gpio_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		LED_2 	<= GPIO_IN;
		LED_1 	<= button_debounced; 	-- D4 Low Enable
		GPIO_OUT <= button_debounced;
	end if;
end process;

end rtl;
