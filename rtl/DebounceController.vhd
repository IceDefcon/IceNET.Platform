library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity DebounceController is
generic 
(
    DELAY : integer := 500000 -- For 50Mhz :: 500,000 = 10ms :: 50,000 = 1ms
);
port 
(    
    clock : in std_logic;

    button_in : in std_logic
    button_out : out std_logic
);
end DebounceController;

architecture rtl of DebounceController is

    signal count : integer := 0;
    signal stable_count : std_logic_vector(15 downto 0) := (others => '0');

    type PWM is 
    (
        IDLE,
        PERIOD,
        STABLE,
        DONE
    );

    signal pwm_state: PWM := IDLE;

    begin

    process(clock)
    begin
        if rising_edge(clock) then

            case pwm_state is

                when IDLE =>
                    if button_in = '1' then
                        pwm_state <= PERIOD;
                    else
                        pwm_state <= IDLE;
                    end if;

                when PERIOD =>
                    if stable_count < "1100001101010000" then
                        if button_in = '1' then
                            pwm_state <= PERIOD;
                        elsif button_in = '0' then
                            stable_count <= (others => '0');
                            pwm_state <= PERIOD;
                        end if;
                    else
                        stable_count <= stable_count + '1';
                        pwm_state <= STABLE;
                    end if;


                when STABLE =>
                    pwm_state <= DONE;

                when DONE =>
                    pwm_state <= IDLE;

                when others =>
                    pwm_state <= IDLE;

            end case;
        end if;
    end process;

end rtl;

1100001101010000