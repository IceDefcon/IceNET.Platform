library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.std_logic_unsigned.all;

ENTITY tb IS
END tb;

ARCHITECTURE behavior OF tb IS

signal CLOCK 	: std_logic := '0';  
signal CLK 		: std_logic := '0';
signal BTN 		: std_logic := '0';
signal BTN_CLR 	: std_logic := '0';

signal counter : std_logic_vector(3 downto 0) := (others => '0');

constant clk_period : time := 20 ns;

BEGIN

CLOCK <= not CLOCK after clk_period/2;

CLOCK_process:
process(CLOCK)
begin
	if rising_edge(CLOCK) then
		counter <= counter + 1;
		CLK <= CLOCK;
	else
		CLK <= '0';
	end if;
end process;

-- Instantiate the Unit Under Test (UUT)
NetworkAnalyser_module: entity work.DeBounce PORT MAP 
(
                  
	clk 		=> CLK, 	-- IN
	btn 		=> BTN,    	-- IN
	btn_clr 	=> BTN_CLR 	-- OUT
);

END;