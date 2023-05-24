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

	CS 				: in 	std_logic; 	-- PIN_119 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	MOSI 				: in 	std_logic; 	-- PIN_121 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	MISO 				: out std_logic; 	-- PIN_125 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	SCLK 				: in 	std_logic; 	-- PIN_129 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	INT_CS 			: in 	std_logic; 	-- PIN_118 :: BBB P9_28 :: YELOW 	:: SPI1_CS0
	INT_MOSI 		: in 	std_logic; 	-- PIN_120 :: BBB P9_30 :: GREEN 	:: SPI1_D1 :: GPIO_112
	INT_MISO 		: out std_logic; 	-- PIN_122 :: BBB P9_29 :: RED 		:: SPI1_D0
	INT_SCLK 		: in 	std_logic; 	-- PIN_126 :: BBB P9_31 :: ORANGE 	:: SPI1_SCLK
	
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

--signal s_miso				: std_logic;
--signal s_parallel_out	: std_logic_vector(7 downto 0);


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

--component spi_driver is
--	port (
--	clk: in std_logic;
--	cs: in std_logic;
--	mosi: in std_logic;
--	miso: out std_logic;
--	parallel_out: out std_logic_vector(7 downto 0)
--);
--end component;
	 
--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

-- Combinational Logic
inv_BUTTON_IN 	<= not BUTTON_IN; -- Buton is low active so must be inverted before debounce
MISO 				<= MOSI;
INT_MISO 		<= INT_MOSI;

debounce_module: debounce port map 
(
	clock => CLOCK,
	button_in => inv_BUTTON_IN,
	button_out => button_debounced
);

--spi_driver_module: spi_driver port map 
--(
--	clk => SCLK,
--	cs => CS,
--	mosi => MOSI,
--	miso => s_miso,
--	parallel_out => s_parallel_out
--);

-- Assign the internal signals to the output ports
--MISO <= s_miso;


gpio_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		LED_2 	<= not BUTTON_IN; 			-- D5 Low Enable
		LED_1 	<= not GPIO_IN; 				-- D4 Low Enable
		LED_0 	<= not button_debounced; 	-- D2 Low Enable
		GPIO_OUT <= button_debounced;
	end if;
end process;

end rtl;
