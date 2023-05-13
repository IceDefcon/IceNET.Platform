library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity licznik is
generic(N = 8)
port
(    
    clock       : in std_logic;
    reset       : in std_logic;

    signal_in   : in std_logic;
    count       : out std_logic_vector(N-1 downto 0)
);
end licznik;

architecture rtl of licznik is

    signal temp_signal  : std_logic := 0;
    signal temp_licznik : std_logic_vector(7 downto 0) := (others => '0');

    licznik_process:
    process(clock,reset)
    begin
        if(reset = '1') then
            temp_licznik <= (others => '0');
        else
            if(rising_edge(clock)) then
                if(signal_in = '1') then
                    temp_licznik <= temp_licznik + '1';
                else
                    temp_licznik <= temp_licznik - '1';
                end if;
            end if;
        end if;
    end process;

    count <= temp_licznik;

end rtl;