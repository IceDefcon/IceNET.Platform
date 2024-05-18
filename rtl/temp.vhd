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
    SPIN,
    DONE
);
signal state_current, state_next: STATE := IDLE;
-- Test
signal isIDLE : std_logic := '0';
signal isSPIN : std_logic := '0';
signal isDONE : std_logic := '0';
signal state_current, state_next: STATE := IDLE;

offload_process:
process (CLOCK_50MHz, offload_reset)
begin
    if offload_reset = '1' then
        state_current <= IDLE;
    elsif rising_edge(CLOCK_50MHz) then
        case state_current is
            when IDLE =>
                isIDLE <= '1';
                isSPIN <= '0';
                isDONE <= '0';
                if offload_interrupt = '1' then
                    state_next <= SPIN;
                else
                    state_next <= IDLE;
                end if;

            when SPIN =>
                isIDLE <= '0';
                isSPIN <= '1';
                isDONE <= '0';
                state_next <= DONE;

            when DONE =>
                isIDLE <= '0';
                isSPIN <= '0';
                isDONE <= '1';
                state_next <= IDLE;

            when others =>
                state_next <= IDLE;

        end case;

        state_current <= state_next;
    end if;
end process;

end rtl;