library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

--
-- Previous Chip
-- EP2C5T144C8
--
-- Current Chip
-- EP4CE15F23C8
--

entity NetworkAnalyser is
port
(
	CLOCK 			: in std_logic; 	-- PIN_17
	
	LED_0 			: out std_logic; 	-- PIN_3 :: U7
	LED_1 			: out std_logic; 	-- PIN_7 :: U8
	LED_2 			: out std_logic;	-- PIN_9 :: R7
	LED_3 			: out std_logic; 	-- PIN_3 :: T8
	LED_4 			: out std_logic; 	-- PIN_7 :: R8
	LED_5 			: out std_logic;	-- PIN_9 :: P8
	LED_6 			: out std_logic; 	-- PIN_3 :: M8
	LED_7 			: out std_logic; 	-- PIN_7 :: N8
	
	KERNEL_CS 		: in 	std_logic; 	-- PIN_119 :: A5 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in 	std_logic; 	-- PIN_121 :: A7 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_125 :: A6 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in 	std_logic; 	-- PIN_129 :: A8 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	INT_IN 			: in 	std_logic; 	-- PIN_143 :: A3 :: YELLOW :: OPEN COLLECTOR
	INT_OUT 			: out std_logic; 	-- PIN_142 :: A4 :: BLUE :: P9_12 :: GPIO 66
	
	BUTTON_0 		: in 	std_logic; 	-- PIN_144 :: H20
	BUTTON_1 		: in 	std_logic; 	-- PIN_144 :: K19
	BUTTON_2 		: in 	std_logic; 	-- PIN_144 :: J18
	BUTTON_3 		: in 	std_logic 	-- PIN_144 :: K18

);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal button_debounced	: std_logic := '1';
signal inv_BUTTON_IN 	: std_logic := '1';

signal clock_1Mhz 		: std_logic := '0';

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------
component debounce
port
(
	clock 		: in  std_logic;
	button_in 	: in  std_logic;
	button_out 	: out  std_logic
);
end component;

component FrequencyDivider
port
(
	clk_in 	: in std_logic;      -- 50 MHz input clock
	reset 	: in std_logic;      -- Asynchronous reset signal
	clk_out 	: out std_logic      -- 1 MHz output clock
);
end component;
	 
--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

-- Combinational Logic
--inv_BUTTON_IN 	<= not BUTTON_0; -- Buton is low active so must be inverted before debounce
KERNEL_MISO 	<= KERNEL_MOSI;

debounce_module: debounce port map 
(
	clock 		=> CLOCK,
	button_in 	=> BUTTON_0,
	button_out 	=> button_debounced
);

debug_led_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		LED_7 	<= '0';
		LED_6 	<= '1';
		LED_5 	<= '1';
		LED_4 	<= '1';
		LED_3 	<= '1';
		LED_2 	<= BUTTON_0;
		LED_1 	<= BUTTON_0;
		LED_0 	<= BUTTON_0;
	end if;
end process;

gpio_interrupt_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		INT_OUT 	<= button_debounced;
	end if;
end process;

end rtl;
