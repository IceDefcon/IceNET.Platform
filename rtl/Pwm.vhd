library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.numeric_std.all;

entity Pwm is
port
(    
    CLOCK_50MHz : in std_logic;

    OFFLOAD_INT : in std_logic;
    FPGA_INT : out std_logic;

    PWM_VECTOR : in std_logic_vector(7 downto 0);

    PWM_SIGNAL : out std_logic
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
signal pwm_width_prev : std_logic_vector(16 downto 0) := (others => '0');
signal pwm_base_timer : std_logic_vector(19 downto 0) := (others => '0');
signal pwm_timer : std_logic_vector(16 downto 0) := (others => '0');
signal pwm_config : std_logic := '0';
signal pwm_ready : std_logic := '0';
signal pwm_data : std_logic_vector(7 downto 0) := (others => '0');

constant multiplier : integer := 200;
constant offset : integer := 50000;

begin

pwm_process:
process(CLOCK_50MHz)
    variable pwm_integer : integer := 0;
begin
    if rising_edge(CLOCK_50MHz) then
        if pwm_base_timer = "11110100001000111111" then -- 20ms PWM Period
            pwm_base_timer <= (others => '0');
            pwm_base_pulse <= '1';
        else
            pwm_base_timer <= pwm_base_timer + '1';
            pwm_base_pulse <= '0';
        end if;

        -- Check if data is ready and store data
        if OFFLOAD_INT = '1' then
            pwm_ready <= '1';
            pwm_data <= PWM_VECTOR;
        end if;

        case pwm_state is
            when IDLE =>
                if pwm_base_pulse = '1' then
                    pwm_state <= INIT;
                else
                    pwm_state <= IDLE;
                end if;
                FPGA_INT <= '0';

            when INIT =>
                if pwm_ready = '1' then
                    -- Calculate PWM width
                    -- 50000*20ns + (200*[0:250])*20ns = 2ms
                    -- 1ms        + [0:1]ms            = [1:2]ms
                    if to_integer(unsigned(pwm_data)) <= 250 then
                        pwm_integer := offset + multiplier * to_integer(unsigned(pwm_data));
                    else
                        pwm_integer := offset + multiplier * 250; -- Max value
                    end if;
                    pwm_ready <= '0';
                    FPGA_INT <= '1';
                end if;
                pwm_state <= CONFIG;

            when CONFIG =>
                pwm_width <= std_logic_vector(to_unsigned(pwm_integer, 17));
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
                if pwm_width_prev /= pwm_width then
                    pwm_width_prev <= pwm_width;
                end if;
                pwm_state <= IDLE;

            when others =>
                pwm_state <= IDLE;

        end case;

        PWM_SIGNAL <= pwm_pulse;

    end if;
end process;

end rtl;