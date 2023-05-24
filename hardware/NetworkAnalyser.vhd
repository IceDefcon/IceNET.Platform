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

	S_CS 				: in 	std_logic; 	-- PIN_119 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	S_MOSI 			: in 	std_logic; 	-- PIN_121 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	S_MISO 			: out std_logic; 	-- PIN_125 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	S_SCLK 			: in 	std_logic; 	-- PIN_129 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	M_CS 				: out std_logic; 	-- PIN_118 :: BBB P9_28 :: YELOW 	:: SPI1_CS0
	M_MOSI 			: out std_logic; 	-- PIN_120 :: BBB P9_30 :: GREEN 	:: SPI1_D1 :: GPIO_112
	M_MISO 			: in 	std_logic; 	-- PIN_122 :: BBB P9_29 :: RED 		:: SPI1_D0
	M_SCLK 			: out std_logic; 	-- PIN_126 :: BBB P9_31 :: ORANGE 	:: SPI1_SCLK
	
	BUTTON_IN 		: in std_logic; 	-- PIN_144
	GPIO_IN 			: in std_logic; 	-- PIN_143
	GPIO_OUT 		: out std_logic 	-- PIN_142
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal button_debounced	: std_logic := '1';
signal inv_BUTTON_IN 	: std_logic := '1';

signal clock_1Mhz 		: std_logic := '0';
signal clock_25Mhz 		: std_logic := '0';
signal clock_12_5Mhz 	: std_logic := '0';

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

component FrequencyDivider
port
(
	clk_in : in std_logic;       -- 50 MHz input clock
	reset : in std_logic;        -- Asynchronous reset signal
	clk_out : out std_logic      -- 1 MHz output clock
);
end component;
 
--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

-- Combinational Logic
inv_BUTTON_IN 	<= not BUTTON_IN; -- Buton is low active so must be inverted before debounce

-- Passing slave inputs to master outputs
--M_CS 	 <= S_CS;
--M_MOSI <= S_MOSI;
--S_MISO <= M_MISO;
--M_SCLK <= S_SCLK;
S_MISO <= S_MOSI;

debounce_module: debounce port map 
(
	clock => CLOCK,
	button_in => inv_BUTTON_IN,
	button_out => button_debounced
);

FrequencyDivider_module: FrequencyDivider port map -- Divider :: 50/2 - 1 = Vector 24 downto 0 which is 50% duty cycle
(
	clk_in => CLOCK,
	reset => '0',
	clk_out => clock_1Mhz
);

gpio_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		LED_2 	<= '1'; --not BUTTON_IN; 	-- D5 Low Enable
		LED_1 	<= '1'; --not GPIO_IN; 		-- D4 Low Enable
		LED_0 	<= not button_debounced; 	-- D2 Low Enable
		GPIO_OUT <= button_debounced;
	end if;
end process;

clock_25Mhz_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		clock_25Mhz <= not clock_25Mhz;
	end if;
end process;

clock_12_5Mhz_process:
process(clock_25Mhz)
begin
	if rising_edge(clock_25Mhz) then
		clock_12_5Mhz <= not clock_12_5Mhz;
	end if;
end process;

end rtl;
