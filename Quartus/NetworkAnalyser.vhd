library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity NetworkAnalyser is
port
(
	clock 			: in std_logic;
	reset 			: in std_logic;

	sclk 				: in std_logic;
	mosi 				: in std_logic;
	miso 				: out std_logic;
	ssel 				: in std_logic;
	
	led_0 			: out std_logic;
	led_1 			: out std_logic;
	led_2 			: out std_logic
);
end NetworkAnalyser;

architecture rtl of NetworkAnalyser is

--------------------------------------------
-- SIGNAL DECLARATION
--------------------------------------------

signal A1 			: std_logic := '0';

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

led_0 <= '0'; 	-- D2 Low Enable
--led_1 <= '1'; 	-- D4 Low Enable
led_2 <= '0'; 	-- D5 Low Enable

main_process:
process(clock)
begin
	if rising_edge(clock) then
		A1 	<= not(A1);
		Led_1 <= A1;
	end if;
end process;

spi_module : component spi
port map 
(
	sclk       	=> sclk,
	cs        	=> ssel,
	mosi    		=> mosi,
	miso 			=> miso,
	reset      	=> '0',

	data_tx     => (others => '0'),
	data_rx   	=> open,
	busyrx  		=> open,
	busytx 		=> open
);

end rtl;

