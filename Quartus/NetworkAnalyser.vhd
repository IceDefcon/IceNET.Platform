library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity NetworkAnalyser is
port
(
	CLOCK 			: in std_logic; 	-- PIN_17
	RESET 			: in std_logic; 	-- PIN_136 :: Connected to K64F

	SSEL 				: in std_logic;  	-- PIN_119 :: Connected to K64F
	MOSI 				: in std_logic;  	-- PIN_121 :: Connected to K64F :: Input to FPGA
	MISO 				: out std_logic;  -- PIN_125 :: Connected to K64F :: Back to Freedom board
	SCLK 				: in std_logic;  	-- PIN_129 :: Connected to K64F :: Input form K64F Freedom
	
	LED_0 			: out std_logic; 	-- PIN_3
	LED_1 			: out std_logic; 	-- PIN_7
	LED_2 			: out std_logic;	-- PIN_9

	PIN_112 			: out std_logic;	-- High Impedance :: Connected to K64F
	PIN_113 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_114 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_115 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_118 			: out std_logic; 	-- High Impedance :: Connected to K64F :: Counter PIN_119 :: SSEL
	PIN_120 			: out std_logic; 	-- High Impedance :: Connected to K64F :: Counter PIN_121 :: MOSI
	PIN_122 			: out std_logic; 	-- High Impedance :: Connected to K64F :: Counter PIN_125 :: MISO
	PIN_126 			: out std_logic; 	-- High Impedance :: Connected to K64F :: Counter PIN_129 :: SCLK
	PIN_132 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_133 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_134 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_135 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_137 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_139 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_141 			: out std_logic; 	-- High Impedance :: Connected to K64F
	PIN_143 			: out std_logic; 	-- High Impedance
	
	BUTTON_IN 			: in std_logic; 	-- Button PIN_144
	BUTTON_OUT 			: out std_logic 	-- Button PIN_142
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

PIN_112 <= 'Z';
PIN_113 <= 'Z';
PIN_114 <= 'Z';
PIN_115 <= 'Z';
PIN_118 <= 'Z';
PIN_120 <= 'Z';
PIN_122 <= 'Z';
PIN_126 <= 'Z';
PIN_132 <= 'Z';
PIN_133 <= 'Z';
PIN_134 <= 'Z';
PIN_135 <= 'Z';
PIN_137 <= 'Z';
PIN_139 <= 'Z';
PIN_141 <= 'Z';
PIN_143 <= 'Z';

LED_0 <= '0'; 						-- D2 Low Enable
LED_1 <= button_debounced; 	-- D4 Low Enable
LED_2 <= '0'; 						-- D5 Low Enable

DeBounce_module: debounce port map 
(
	clock => CLOCK,
	button_in => BUTTON_IN,
	button_out => button_debounced
);

button_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		BUTTON_OUT <= button_debounced;
	end if;
end process;

end rtl;
