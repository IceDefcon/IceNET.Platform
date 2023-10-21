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

entity NetworkAnalyser is
port
(
	CLOCK 			: in std_logic; 	-- PIN_T2 :: 50Mhz FPGA
	
	LED_0 			: out std_logic; 	-- PIN_U7
	LED_1 			: out std_logic; 	-- PIN_U8
	LED_2 			: out std_logic;	-- PIN_R7
	LED_3 			: out std_logic; 	-- PIN_T8
	LED_4 			: out std_logic; 	-- PIN_R8
	LED_5 			: out std_logic;	-- PIN_P8
	LED_6 			: out std_logic; 	-- PIN_M8
	LED_7 			: out std_logic; 	-- PIN_N8
	
	KERNEL_CS 		: in  std_logic; 	-- PIN_A5 :: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in  std_logic; 	-- PIN_A7 :: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_A6 :: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in  std_logic; 	-- PIN_A8 :: BBB P9_22 :: BLACK 	:: SPI0_SCLK
	
	INT_IN 			: in  std_logic; 	-- PIN_A3 :: BBB P8_7  :: YELLOW 	:: OPEN COLLECTOR
	INT_OUT 		: out std_logic; 	-- PIN_A4 :: BBB P9_12 :: BLUE 		:: GPIO 66
	
	BUTTON_0 		: in  std_logic; 	-- PIN_H20 :: Reset
	BUTTON_1 		: in  std_logic; 	-- PIN_K19 :: Doesnt Work :: WTF xD Broken Button or Incorrect Schematic
	BUTTON_2 		: in  std_logic; 	-- PIN_J18
	BUTTON_3 		: in  std_logic 	-- PIN_K18

);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

------------------------
-- SIGNAL DECLARATION --
------------------------
signal button_debounced			: std_logic := '1';
signal stop 					: std_logic := '0';
signal pulse 					: std_logic := '0';

signal clock_count 				: std_logic_vector(25 downto 0) := (others => '0');
signal interrupt_count 			: std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_signal 		: std_logic := '0';
signal interrupt_cutoff 		: std_logic := '0';

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
		LED_7 	<= '0';
		LED_6 	<= '0';
		LED_5 	<= '0';
		LED_4 	<= '0';
		LED_3 	<= BUTTON_3;
		LED_2 	<= BUTTON_2;
		LED_1 	<= BUTTON_1; -- BUTTON_1 is not working
		LED_0 	<= BUTTON_0;
	end if;
end process;

--------------------
-- SPI Process
--------------------
process(CLOCK, KERNEL_CS, KERNEL_SCLK, stop)
begin
	if rising_edge(CLOCK) then
		if KERNEL_CS = '0' then
			if KERNEL_SCLK = '0' then
				if stop = '0' then
					if (pulse = '0') then
						pulse <= '1';
					else
						pulse <= '0';
					end if;
					KERNEL_MISO <= pulse;
					stop <= '1';
				end if;
			elsif KERNEL_SCLK = '1' then
				stop <= '0';
			end if;
		end if;
	end if;
end process;

--------------------
-- clock division
--------------------
process(CLOCK, interrupt_cutoff, clock_count)
begin
	if rising_edge(CLOCK) then

		if interrupt_cutoff = '1' then
			interrupt_count <= interrupt_count + '1';
			if interrupt_count = "1111" then
				interrupt_signal <= '0';
				interrupt_cutoff <= '0';
				interrupt_count <= (others => '0');
			end if;
		end if;

		clock_count <= clock_count + '1';

		if clock_count = "11111111111111111111111111" then
			if interrupt_signal = '0' then
				interrupt_signal <= '1';
			end if;
			clock_count <= (others => '0');
			interrupt_cutoff <= '1';
		end if;
		
	end if;
end process;

----------------
-- Interrupts --
----------------
INT_OUT 	<= interrupt_signal;

end rtl;