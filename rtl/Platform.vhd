library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

------------------------------
-- Author: Ice.Marek 		--
-- IceNET Technology 2023 	--
-- 							--
-- FPGA Chip 				--
-- Cyclone IV 				--
-- EP4CE15F23C8 			--

------------------------------
entity Platform is
port
(
	CLOCK 			: in std_logic; 	-- PIN_T2 :: 50Mhz Clock

	LED_0 			: out std_logic; 	-- PIN_U7
	LED_1 			: out std_logic; 	-- PIN_U8
	LED_2 			: out std_logic;	-- PIN_R7
	LED_3 			: out std_logic; 	-- PIN_T8
	LED_4 			: out std_logic; 	-- PIN_R8
	LED_5 			: out std_logic;	-- PIN_P8
	LED_6 			: out std_logic; 	-- PIN_M8
	LED_7 			: out std_logic; 	-- PIN_N8

	KERNEL_CS 		: in  std_logic; 	-- PIN_A5 	:: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in  std_logic; 	-- PIN_A7 	:: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_A6 	:: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in  std_logic; 	-- PIN_A8 	:: BBB P9_22 :: BLACK 	:: SPI0_SCLK

	I2C_IN_SDA 		: in 	std_logic; 	-- PIN_A9 	:: BBB P9_20 :: BLUE
	I2C_IN_SCK 		: in 	std_logic; 	-- PIN_A10 	:: BBB P9_19 :: GREEN-ORANGE

	INT_CPU 			: out std_logic; 	-- PIN_A3 	:: BBB P9_12 :: BLACK
	INT_FPGA 		: in 	std_logic; 	-- PIN_A4 	:: BBB P9_14 :: WHITE

	BUTTON_0 		: in  std_logic; 	-- PIN_H20 	:: Reset
	BUTTON_1 		: in  std_logic; 	-- PIN_K19 	:: Doesnt Work :: Broken Button or Incorrect Schematic
	BUTTON_2 		: in  std_logic; 	-- PIN_J18
	BUTTON_3 		: in  std_logic 	-- PIN_K18
);
end Platform;

architecture rtl of Platform is

------------------------
-- SIGNAL DECLARATION --
------------------------

-- BUTTON
signal button_debounced			: std_logic := '0';

-- SPI
constant DATA					: std_logic_vector(7 downto 0) := "10001000"; -- 0x88
signal run 						: std_logic := '0';
signal index 					: integer := 0;
signal count 					: std_logic_vector(4 downto 0) := (others => '0');
signal count_bit 				: std_logic_vector(3 downto 0) := (others => '0');

-- Interrupt Pulse Generator
signal interrupt_period 		: std_logic_vector(25 downto 0) := (others => '0');
signal interrupt_length 		: std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_signal 		: std_logic := '0';
signal interrupt_cutoff 		: std_logic := '0';

signal ALIGNED_KERNEL_SCLK 	: std_logic := '0';

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
		LED_7 	<= '1';
		LED_6 	<= '1';
		LED_5 	<= '1';
		LED_4 	<= '1';
		LED_3 	<= BUTTON_3;
		LED_2 	<= BUTTON_2;
		LED_1 	<= BUTTON_1; -- BUTTON_1 is not working
		LED_0 	<= BUTTON_0;
	end if;
end process;

clock_align_process:
process(CLOCK, KERNEL_SCLK)
begin
	if rising_edge(CLOCK) then
		if KERNEL_SCLK = '1' then
			ALIGNED_KERNEL_SCLK <= '1';
		elsif KERNEL_SCLK = '0' then
			ALIGNED_KERNEL_SCLK <= '0';
		end if;
	end if;
end process;


--------------------
-- SPI Process
--------------------
spi_process:
process(CLOCK, ALIGNED_KERNEL_SCLK, run, count, count_bit)
begin
	if rising_edge(CLOCK) then

		if run = '1' then
			if count = "00000" then
				count_bit <= count_bit + '1';
			end if;
		end if;

		index <= to_integer(unsigned(count_bit));

		if ALIGNED_KERNEL_SCLK = '1' then
			run <= '1';
			count <= count + '1';
			KERNEL_MISO <= DATA(7 - index); -- Other way return on the wire
		elsif ALIGNED_KERNEL_SCLK = '0' then
			if count > 0 then
				count <= count - '1';
			end if;
			KERNEL_MISO <= DATA(7 - index); -- Other way return on the wire
			if count_bit = "0111" then
				run <= '0';
				count_bit <= "0000";
			end if;
		end if;
	end if;		
end process;

-----------------------------------
-- Interrupt pulse generator
-- 0x2FAF07F/50 MHz
-- (49999999 + 1)/50000000 Hz = 1 sec
-----------------------------------
process(CLOCK, interrupt_cutoff, interrupt_period)
begin
	if rising_edge(CLOCK) then

		-- Interrupt is 16 * 20ns ---> 320ns Long
		if interrupt_cutoff = '1' then
			interrupt_length <= interrupt_length + '1';
			if interrupt_length = "1111" then
				interrupt_signal <= '0';
				interrupt_cutoff <= '0';
				interrupt_length <= (others => '0');
			end if;
		end if;

		interrupt_period <= interrupt_period + '1';

		-- Interrupt is generated every (49999999 + 1) * 20ns ---> 1s
		if interrupt_period = "10111110101111000001111111" then
			if interrupt_signal = '0' then
				interrupt_signal <= '1';
			end if;
			interrupt_period <=(others => '0');
			interrupt_cutoff <= '1';
		end if;
		
	end if;
end process;

----------------------
-- Interrupts
----------------------
INT_CPU <= '0';--interrupt_signal;

end rtl;