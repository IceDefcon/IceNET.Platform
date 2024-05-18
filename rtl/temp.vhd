library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity temp is
port
(    
    clock : in std_logic;
    reset : in std_logic;

    input : in std_logic;
    output : out std_logic
);
end temp;

architecture rtl of temp is

type STATE is 
(
    IDLE,
    SPIN
);
signal test_state : STATE := IDLE;
signal test_interrupt : std_logic := '0';
signal test_count : std_logic_vector(1 downto 0) := '0';

process (CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case test_state is

            when IDLE =>
                --
                -- BODY
                --
                if test_interrupt = '1' then
                    test_state <= SPIN;
                else
                    test_state <= IDLE;
                end if;

            when SPIN =>
                if test_count = "00" then
                    --
                    -- BODY
                    --
                    test_state <= SPIN;
                elsif test_count = "01" then
                    --
                    -- BODY
                    --
                    test_state <= SPIN;
                else
                    --
                    -- BODY
                    --
                    test_state <= IDLE;
                end if;

            when others =>
                test_state <= IDLE;

        end case;
    end if;
end process;

end rtl;