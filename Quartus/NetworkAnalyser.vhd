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
	PIN_141 			: out std_logic 	-- High Impedance :: Connected to K64F
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal a1 					: std_logic := '0';
signal clock_register 	: std_logic_vector (1 downto 0) := (others => '0');

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------

component spi is
generic ( N : natural := 8	);
port 
(
	sclk   			: in std_logic;     								--SPI Clock
	cs     			: in std_logic;     								--Chip select
	mosi   			: in std_logic;	   							--Master Out Slave In  (MOSI)
	miso   			: out std_logic;	   							--Master In  Slave Out (MISO)
	reset  			: in std_logic;     								--Asynchronous Reset

	data_tx  		: in  std_logic_vector(N-1 downto 0);		--Parallel N-bit data to return back to the master
	data_rx 			: out std_logic_vector(N-1 downto 0);   	--Parallel N-bit data recevied from the master
	busyrx   		: out std_logic;									--Do not read data_rx while high
	busytx   		: out std_logic                        	--Do not write data_tx while high
);
end component spi;

--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

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
LED_1 <= '1'; 	-- D4 Low Enable
LED_2 <= '0'; 	-- D5 Low Enable

main_process:
process(CLOCK, SCLK)
begin
	if rising_edge(SCLK) then
		if rising_edge(CLOCK) then
			clock_register(1) <= clock_register(0);
			clock_register(0) <= SCLK;
		end if;
	end if;
end process;

spi_module : component spi
port map 
(
	sclk       	=> clock_register(1),
	cs        	=> SSEL,
	mosi    		=> MOSI,
	miso 			=> MISO,
	reset      	=> '1',

	data_tx     => (others => '0'),
	data_rx   	=> open,
	busyrx  		=> open,
	busytx 		=> open
);

end rtl;
