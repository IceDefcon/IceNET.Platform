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
	
	BUTTON 			: in std_logic 	-- Button
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal clock50MHz 		: std_logic := '0';
signal sclk_delayed 		: std_logic := '0';
signal key_button			: std_logic := '0';

constant depth: positive := 107;
signal shift_register 	: std_logic_vector(depth - 2 downto 0) := (others => '0');

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------
COMPONENT DeBounce
PORT
(
	Clock : IN  std_logic;
	Reset : IN  std_logic;
	button_in : IN  std_logic;
	pulse_out : OUT  std_logic
);
END COMPONENT;

--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

DeBounce_module: DeBounce PORT MAP 
(
	Clock => CLOCK,
	Reset => '0',
	button_in => BUTTON,
	pulse_out => key_button
);
		  
-- GREEN LED
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

LED_0 <= '0'; 	-- D2 Low Enable
--LED_1 <= '1'; 	-- D4 Low Enable
LED_2 <= '0'; 	-- D5 Low Enable

main_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		clock50MHz 	<= not(clock50MHz); -- Virtual Signal Clock
		LED_1 		<= clock50MHz;
	end if;
end process;

--K64F have clock process too fast with respect to data !!!
--shift_process:
--process(CLOCK)
--begin
--	if rising_edge(CLOCK) then
--		shift_register <= shift_register(shift_register'high - 1 downto shift_register'low) & SCLK;
--		sclk_delayed <= shift_register(shift_register'high);
--	end if;
--end process;

 
end rtl;
