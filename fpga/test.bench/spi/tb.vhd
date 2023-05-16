library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.std_logic_unsigned.all;

ENTITY tb IS
END tb;

ARCHITECTURE behavior OF tb IS
  
signal CLOCK : std_logic := '0';
signal RESET : std_logic := '1';

signal SSEL : std_logic := '0';
signal MOSI : std_logic := '0';
signal MISO : std_logic; 			-- Output
signal SCLK : std_logic := '0';

signal BUSYRX : std_logic;
signal BUSYTX : std_logic;

signal DATA_RX : std_logic_vector(7 downto 0);
signal DATA_TX : std_logic_vector(7 downto 0) := (others => '0');

signal counter : std_logic_vector(3 downto 0) := (others => '0');

constant clk_period : time := 20 ns;

BEGIN

CLOCK <= not CLOCK after clk_period/2;

spi_clock_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		counter <= counter + 1;
		SCLK <= CLOCK;
	else
		SCLK <= '0';
	end if;
end process;




--spi_clock:
--process(CLOCK)
--begin

-- Instantiate the Unit Under Test (UUT)
NetworkAnalyser_module: entity work.spi PORT MAP (
                  
	sclk 		=> SCLK, 	-- IN
	cs 			=> SCLK,    -- IN
	mosi 		=> MOSI,   	-- IN
	miso 		=> MISO,   	-- OUT
	reset 		=> RESET, 	-- IN

	data_tx  	=> DATA_TX,	-- IN (7 downto 0)
	data_rx 	=> DATA_RX,	-- ONT (7 downto 0)
	busyrx   	=> BUSYRX, 	-- OUT
	busytx   	=> BUSYTX 	-- OUT

	);

END;