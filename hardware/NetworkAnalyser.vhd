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
	
	LED_0 			: out std_logic; 	-- PIN_3
	LED_1 			: out std_logic; 	-- PIN_7
	LED_2 			: out std_logic;	-- PIN_9

	KERNEL_CS 		: in 	std_logic; 	-- PIN_119 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in 	std_logic; 	-- PIN_121 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_125 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in 	std_logic; 	-- PIN_129 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	--REQ_CS 		: in 	std_logic; 	-- PIN_118 :: BBB P9_28 :: YELOW 	:: SPI1_CS0
	--REQ_MOSI 		: in 	std_logic; 	-- PIN_120 :: BBB P9_30 :: GREEN 	:: SPI1_D1 :: GPIO_112
	--REQ_MISO 		: out std_logic; 	-- PIN_122 :: BBB P9_29 :: RED 		:: SPI1_D0
	--REQ_SCLK 		: in 	std_logic; 	-- PIN_126 :: BBB P9_31 :: ORANGE 	:: SPI1_SCLK

	INT_IN 			: in 	std_logic; 	-- PIN_143 :: YELLOW :: OPEN COLLECTOR
	INT_OUT 			: out std_logic; 	-- PIN_142 :: BLUE :: P9_12 :: GPIO 66
	
	BUTTON 			: in 	std_logic 	-- PIN_144

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
inv_BUTTON_IN 	<= not BUTTON; -- Buton is low active so must be inverted before debounce
KERNEL_MISO 	<= KERNEL_MOSI;

debounce_module: debounce port map 
(
	clock 		=> CLOCK,
	button_in 	=> inv_BUTTON_IN,
	button_out 	=> button_debounced
);

FrequencyDivider_module: FrequencyDivider port map -- Divider :: 50/2 - 1 = Vector 24 downto 0 which is 50% duty cycle
(
	clk_in 	=> CLOCK,
	reset 	=> '0',
	clk_out 	=> clock_1Mhz
);

debug_led_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		--LED_2 	<= BUTTON; 				-- D5 Low Enable
		--LED_1 	<= INT_IN; 				-- D4 Low Enable
		--LED_0 	<= not button_debounced; 	-- D2 Low Enable
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
