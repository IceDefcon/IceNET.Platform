library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.numeric_std.all;

entity Pwm is
port
(    
    CLOCK_50MHz : in std_logic;

    PWM_SIGNAL : out std_logic;
    PWM_INTERRUPT : out std_logic
);
end Pwm;

architecture rtl of Pwm is

-- Pwm
type PWM is 
(
    IDLE,
    INIT,
    CONFIG,
    PRODUCE,
    DONE
);
signal pwm_state: PWM := IDLE;
signal pwm_base_pulse : std_logic := '0';
signal pwm_pulse : std_logic := '0';
signal pwm_width : std_logic_vector(16 downto 0) := (others => '0');
signal pwm_base_timer : std_logic_vector(19 downto 0) := (others => '0');
signal pwm_timer : std_logic_vector(16 downto 0) := (others => '0');

begin

pwm_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if pwm_base_timer = "11110100001000111111" then -- 20ms PWM Period
            pwm_base_timer <= (others => '0');
            pwm_base_pulse <= '1';
        else
            pwm_base_timer <= pwm_base_timer + '1';
            pwm_base_pulse <= '0';
        end if;

        case pwm_state is
            when IDLE =>
                if pwm_base_pulse = '1' then
                    pwm_state <= INIT;
                else
                    pwm_state <= IDLE;
                end if;

            when INIT =>
                pwm_state <= CONFIG;

            when CONFIG =>
                --pwm_width <= "01100001101001111"; -- [50000] 1ms :: Zero throtle
                pwm_width <= "11000011010100000"; -- [100000] 2ms :: Full throtle
                pwm_state <= PRODUCE;

            when PRODUCE =>
                if pwm_timer = pwm_width then
                    pwm_pulse <= '0';
                    pwm_timer <= (others => '0');
                    pwm_state <= DONE;
                else
                    pwm_pulse <= '1';
                    pwm_timer <= pwm_timer + '1';
                    pwm_state <= PRODUCE;
                end if;

            when DONE =>
                pwm_state <= IDLE;

            when others =>
                pwm_state <= IDLE;

        end case;

        PWM_SIGNAL <= pwm_pulse;
        PWM_INTERRUPT <= pwm_base_pulse;

    end if;
end process;

end rtl;