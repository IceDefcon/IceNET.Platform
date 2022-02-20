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

signal nios_clk 				: std_logic;
signal nios_reset_n 			: std_logic;

signal spi_mem_address    		: std_logic_vector(9 downto 0);
signal spi_mem_chipselect 		: std_logic;
signal spi_mem_clken      		: std_logic;
signal spi_mem_write      		: std_logic;
signal spi_mem_readdata   		: std_logic_vector(31 downto 0);
signal spi_mem_writedata  		: std_logic_vector(31 downto 0);
signal spi_mem_byteenable 		: std_logic_vector(3 downto 0);

--------------------------------------------
-- COMPONENTS DECLARATION
--------------------------------------------

component spi is
generic ( N : natural := 8	);
port 
(
	sclk   			: in std_logic;     						--SPI Clock
	cs     			: in std_logic;     						--Chip select
	mosi   			: in std_logic;	   							--Master Out Slave In  (MOSI)
	miso   			: out std_logic;	   						--Master In  Slave Out (MISO)
	reset  			: in std_logic;     						--Asynchronous Reset

	data_tx  		: in  std_logic_vector(N-1 downto 0);		--Parallel N-bit data to return back to the master
	data_rx 		: out std_logic_vector(N-1 downto 0);   	--Parallel N-bit data recevied from the master
	busyrx   		: out std_logic;							--Do not read data_rx while high
	busytx   		: out std_logic                        		--Do not write data_tx while high
);
end component spi;

component NiosFirmware is
port
(
	cpu_clk            : in  std_logic                     := 'X';
	sys_reset_n        : in  std_logic                     := 'X';
	spi_mem_address    : in  std_logic_vector(9 downto 0)  := (others => 'X');
	spi_mem_chipselect : in  std_logic                     := 'X';
	spi_mem_clken      : in  std_logic                     := 'X';
	spi_mem_write      : in  std_logic                     := 'X';
	spi_mem_readdata   : out std_logic_vector(31 downto 0);
	spi_mem_writedata  : in  std_logic_vector(31 downto 0) := (others => 'X');
	spi_mem_byteenable : in  std_logic_vector(3 downto 0)  := (others => 'X')
);
end component NiosFirmware;

--------------------------------------------
-- MAIN ROUTINE
--------------------------------------------
begin

led_0 <= '0'; 	-- D2
led_1 <= '1'; 	-- D4
led_2 <= '0'; 	-- D5

spi_module : component spi
port map 
(
	sclk           => sclk,
	cs        	=> ssel,
	mosi    		=> mosi,
	miso 			=> miso,
	reset      	=> '0',

	data_tx     	=> (others => '0'),
	data_rx   	=> open,
	busyrx  		=> open,
	busytx 		=> open
);

NiosFirmware_module : component NiosFirmware
port map 
(
	cpu_clk            => nios_clk,
	sys_reset_n        => nios_reset_n,
	spi_mem_address    => spi_mem_address,
	spi_mem_chipselect => spi_mem_chipselect,
	spi_mem_clken      => spi_mem_clken,
	spi_mem_write      => spi_mem_write,
	spi_mem_readdata   => spi_mem_readdata,
	spi_mem_writedata  => spi_mem_writedata,
	spi_mem_byteenable => spi_mem_byteenable
);

end rtl;

