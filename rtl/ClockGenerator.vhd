library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity ClockGenerator is
generic 
(
    SRC     : integer := 10     -- Source :: half clock cycle in ns
    DEST    : integer := 100    -- Destination :: half clock cycle in ns  
);
port
(    
    clock_in : in std_logic;
    clock_out : in std_logic
);
end ClockGenerator;

architecture rtl of ClockGenerator is

signal source_vector : std_logic_vector(to_unsigned(SRC, 8));
signal destination_vector : std_logic_vector(to_unsigned(DEST, 8));



end rtl;

50MHz
0011 0010

20MHz
0001 0100