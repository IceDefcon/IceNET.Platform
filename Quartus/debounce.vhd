library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity debounce is
port
(    
    clock : in std_logic;
    button_in : in std_logic;
    button_out : out std_logic
);
end debounce;

architecture rtl of debounce is

    constant delay : integer := 50000; -- For 50Mhz :: 500,000 = 10ms :: 50000 = 1ms
    signal count : integer := 0;
    signal button_temp : std_logic := '0';

    begin

    process(clock)
    begin
        if rising_edge(clock) then
            if (button_in /= button_temp) then -- if not equal
                button_temp <= button_in;
                count <= 0;
            elsif (count = delay) then
                button_out <= button_temp;
            else
                count <= count + 1;
					 button_out <= '0'; -- To avoid high debouncing signals output must be keept as low on idle
            end if;
        end if;
    end process;

end rtl;