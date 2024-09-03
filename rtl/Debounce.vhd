library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity Debounce is
generic 
(
    DELAY : integer := 500000 -- For 50Mhz :: 500,000 = 10ms :: 50,000 = 1ms
);
port 
(    
    clock : in std_logic;

    button_in_1 : in std_logic;
    button_in_2 : in std_logic;
    button_in_3 : in std_logic;
    button_in_4 : in std_logic;

    button_out_1 : out std_logic;
    button_out_2 : out std_logic;
    button_out_3 : out std_logic;
    button_out_4 : out std_logic
);
end Debounce;

architecture rtl of Debounce is

    signal count_1 : integer := 0;
    signal button_temp_1 : std_logic := '0';

    signal count_2 : integer := 0;
    signal button_temp_2 : std_logic := '0';

    signal count_3 : integer := 0;
    signal button_temp_3 : std_logic := '0';

    signal count_4 : integer := 0;
    signal button_temp_4 : std_logic := '0';

    type STATE is 
    (
        IDLE,
        DELAY_INIT,
        DELAY_CONFIG,
        READ_ID,
        READ_REGISTER,
        READ_CONTROL,
        READ_DATA
    );
    signal offload_state: STATE := IDLE;

    begin

    process(clock)
    begin
        if rising_edge(clock) then
            if (button_in_1 /= button_temp_1) then -- if not equal
                button_temp_1 <= button_in_1;
                count_1 <= 0;
            elsif (count_1 = DELAY) then
                button_out_1 <= not button_temp_1;
            else
                count_1 <= count_1 + 1;
					button_out_1 <= '0'; -- To avoid high debouncing signals output must be keept as low on idle
            end if;

            if (button_in_2 /= button_temp_2) then -- if not equal
                button_temp_2 <= button_in_2;
                count_2 <= 0;
            elsif (count_2 = DELAY) then
                button_out_2 <= not button_temp_2;
            else
                count_2 <= count_2 + 1;
                    button_out_2 <= '0'; -- To avoid high debouncing signals output must be keept as low on idle
            end if;

            if (button_in_3 /= button_temp_3) then -- if not equal
                button_temp_3 <= button_in_3;
                count_3 <= 0;
            elsif (count_3 = DELAY) then
                button_out_3 <= not button_temp_3;
            else
                count_3 <= count_3 + 1;
                    button_out_3 <= '0'; -- To avoid high debouncing signals output must be keept as low on idle
            end if;

            if (button_in_4 /= button_temp_4) then -- if not equal
                button_temp_4 <= button_in_4;
                count_4 <= 0;
            elsif (count_4 = DELAY) then
                button_out_4 <= not button_temp_4;
            else
                count_4 <= count_4 + 1;
                    button_out_4 <= '0'; -- To avoid high debouncing signals output must be keept as low on idle
            end if;
            
        end if;
    end process;

end rtl;