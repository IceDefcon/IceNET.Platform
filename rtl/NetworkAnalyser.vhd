library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

------------------------------
-- Author: Ice.Marek 		--
-- IceNET Technology 2023 	--
-- 							--
-- Current Chip 			--
-- Cyclone IV 				--
-- EP4CE15F23C8 			--
------------------------------

entity NetworkAnalyser is
port
(
	CLOCK 			: in std_logic; 	-- PIN_T2
	
	LED_0 			: out std_logic; 	-- PIN_U7
	LED_1 			: out std_logic; 	-- PIN_U8
	LED_2 			: out std_logic;	-- PIN_R7
	LED_3 			: out std_logic; 	-- PIN_T8
	LED_4 			: out std_logic; 	-- PIN_R8
	LED_5 			: out std_logic;	-- PIN_P8
	LED_6 			: out std_logic; 	-- PIN_M8
	LED_7 			: out std_logic; 	-- PIN_N8
	
	I2C_SDA_I 		: inout std_logic; 	-- PIN_A9  :: BBB P9_20 :: BLUE
	I2C_SCL_I 		: inout std_logic; 	-- PIN_A10 :: BBB P9_19 :: GREEN
	I2C_SDA_O 		: inout std_logic; 	-- PIN_A13 :: GYROSCOPE :: WHITE
	I2C_SCL_O 		: inout std_logic; 	-- PIN_A14 :: GYROSCOPE :: BLACK
	
	KERNEL_CS 		: in  std_logic; 	-- PIN_A5 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in  std_logic; 	-- PIN_A7 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_A6 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in  std_logic; 	-- PIN_A8 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	INT_IN 			: in  std_logic; 	-- PIN_A3 :: BBB P8_7  :: YELLOW 	:: OPEN COLLECTOR
	INT_OUT 		: out std_logic; 	-- PIN_A4 :: BBB P9_12 :: BLUE 		:: GPIO 66
	
	BUTTON_0 		: in  std_logic; 	-- PIN_H20
	BUTTON_1 		: in  std_logic; 	-- PIN_K19 :: Doesnt Work
	BUTTON_2 		: in  std_logic; 	-- PIN_J18
	BUTTON_3 		: in  std_logic 	-- PIN_K18

);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

------------------------
-- SIGNAL DECLARATION --
------------------------
signal button_debounced	: std_logic := '1';
signal clock_1Mhz 		: std_logic := '0';
signal direction 		: std_logic := '0';
signal counter 			: std_logic_vector(3 downto 0) 	:=  (others => '0');

-- Synchronised I2C signal
signal I2C_SDA_IS 		: std_logic := '0';
signal I2C_SCL_IS 		: std_logic := '0';

----------------------------
-- COMPONENTS DECLARATION --
----------------------------
component Debounce
port
(
	clock 		: in  std_logic;
	button_in 	: in  std_logic;
	button_out 	: out std_logic
);
end component;

------------------
-- MAIN ROUTINE --
------------------
begin

-------------------------
-- Clock Process Logic --
-------------------------
--------------
-- Debounce --
--------------
debounce_module: Debounce port map 
(
	clock 		=> CLOCK,
	button_in 	=> BUTTON_0,
	button_out 	=> button_debounced
);

----------------
-- Led Status --
----------------
status_led_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		LED_7 	<= counter(3);
		LED_6 	<= counter(2);
		LED_5 	<= counter(1);
		LED_4 	<= counter(0);
		LED_3 	<= BUTTON_3;
		LED_2 	<= BUTTON_2;
		LED_1 	<= not direction; -- BUTTON_1 is not working
		LED_0 	<= BUTTON_0;
	end if;
end process;

----------------
-- Interrupts --
----------------
gpio_interrupt_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		INT_OUT 	<= button_debounced;
	end if;
end process;

---------
-- I2C --
---------
synchronised_process:
process(CLOCK, I2C_SDA_IS, I2C_SCL_IS, I2C_SDA_I, I2C_SCL_I)
begin
	if rising_edge(CLOCK) then
		I2C_SDA_IS <= I2C_SDA_I;
		I2C_SCL_IS <= I2C_SCL_I;
	end if;
end process;

count_process:
process(I2C_SCL_IS, BUTTON_2, counter, direction)
begin
	if rising_edge(I2C_SCL_IS) then
		counter <= counter + 1;
	end if;
	
	if BUTTON_2 = '0' then
		counter <= (others => '0');
	end if;
	
	if counter = "1001" then
		direction <= '1';
	else
		direction <= '0';
	end if;
end process;

i2c_process:
process(direction)
begin
	if direction = '1' then
		I2C_SCL_O 	<= 'Z';
		I2C_SCL_O 	<= 'Z';
	else
		I2C_SCL_O 	<= I2C_SCL_IS;
		I2C_SDA_O 	<= I2C_SDA_IS;
	end if;
end process;

-------------------------
-- Combinational Logic --
-------------------------
KERNEL_MISO <= KERNEL_MOSI;
	
end rtl;
