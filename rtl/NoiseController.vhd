library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.std_logic_ARITH.ALL;
use IEEE.std_logic_UNSIGNED.ALL;

entity NoiseController is
Port 
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    INPUT_SIGNAL : in  std_logic;
    THRESHOLD : in  integer range 0 to 255;

    OUTPUT_SIGNAL  : out std_logic
);
end NoiseController;

architecture rtl of NoiseController is

    signal count : integer range 0 to 255 := 0;
    signal signal_high : std_logic := '0';

begin

    process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            count <= 0;
            OUTPUT_SIGNAL <= '0';
            signal_high <= '0';
        elsif rising_edge(CLOCK_50MHz) then

            if INPUT_SIGNAL = '1' then

                if count < THRESHOLD then
                    count <= count + 1;
                end if;

                if count >= THRESHOLD then
                    OUTPUT_SIGNAL <= '1';
                end if;
            else
                count <= 0;
                OUTPUT_SIGNAL <= '0';
            end if;
        end if;
    end process;

end rtl;
