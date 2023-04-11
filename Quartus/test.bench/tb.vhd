
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY tb IS
END tb;

ARCHITECTURE behavior OF tb IS
  
signal CLOCK : std_logic;
signal RESET : std_logic;

signal SSEL : std_logic;
signal MOSI : std_logic;
signal MISO : std_logic;
signal SCLK : std_logic;

signal LED_0 : std_logic;
signal LED_1 : std_logic;
signal LED_2 : std_logic;

signal PIN_112 : std_logic;
signal PIN_113 : std_logic;
signal PIN_114 : std_logic;
signal PIN_115 : std_logic;
signal PIN_118 : std_logic;
signal PIN_120 : std_logic;
signal PIN_122 : std_logic;
signal PIN_126 : std_logic;
signal PIN_132 : std_logic;
signal PIN_133 : std_logic;
signal PIN_134 : std_logic;
signal PIN_135 : std_logic;
signal PIN_137 : std_logic;
signal PIN_139 : std_logic;
signal PIN_141 : std_logic;


BEGIN
  
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