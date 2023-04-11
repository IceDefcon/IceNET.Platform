
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY tb IS
END tb;

ARCHITECTURE behavior OF tb IS
  
signal CLOCK : std_logic := '0';
signal RESET : std_logic := '0';

signal SSEL : std_logic := '0';
signal MOSI : std_logic := '0';
signal MISO : std_logic := '0';
signal SCLK : std_logic := '0';

signal LED_0 : std_logic := '0';
signal LED_1 : std_logic := '0';
signal LED_2 : std_logic := '0';

signal PIN_112 : std_logic := '0';
signal PIN_113 : std_logic := '0';
signal PIN_114 : std_logic := '0';
signal PIN_115 : std_logic := '0';
signal PIN_118 : std_logic := '0';
signal PIN_120 : std_logic := '0';
signal PIN_122 : std_logic := '0';
signal PIN_126 : std_logic := '0';
signal PIN_132 : std_logic := '0';
signal PIN_133 : std_logic := '0';
signal PIN_134 : std_logic := '0';
signal PIN_135 : std_logic := '0';
signal PIN_137 : std_logic := '0';
signal PIN_139 : std_logic := '0';
signal PIN_141 : std_logic := '0';

constant clk_period : time := 20 ns;

BEGIN

CLOCK <= not CLOCK after clk_period/2;

-- Instantiate the Unit Under Test (UUT)
NetworkAnalyser_module: entity work.NetworkAnalyser PORT MAP (
                  
	CLOCK => CLOCK,
	RESET => RESET,

	SSEL => SSEL,
	MOSI => MOSI,
	MISO => MISO,
	SCLK => SCLK,
		
	LED_0 => LED_0,
	LED_1 => LED_1,
	LED_2 => LED_2,
	
	PIN_112 => PIN_112,
	PIN_113 => PIN_113,
	PIN_114 => PIN_114,
	PIN_115 => PIN_115,
	PIN_118 => PIN_118,
	PIN_120 => PIN_120,
	PIN_122 => PIN_122,
	PIN_126 => PIN_126,
	PIN_132 => PIN_132,
	PIN_133 => PIN_133,
	PIN_134 => PIN_134,
	PIN_135 => PIN_135,
	PIN_137 => PIN_137,
	PIN_139 => PIN_139,
	PIN_141 => PIN_141
	);






END;