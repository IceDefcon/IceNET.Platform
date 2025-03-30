library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.numeric_std.all;

entity PwmController is
generic
(
    BASE_PERIOD_MS : integer := 20
);
port
(    
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    OFFLOAD_INT : in std_logic;
    FPGA_INT : out std_logic;

    PWM_VECTOR : in std_logic_vector(7 downto 0);

    PWM_SIGNAL : out std_logic
);
end PwmController;

architecture rtl of PwmController is

-- Default base period :: 20 * 50000clk * 20ns = 20ms
constant DEFAULT_PERIOD : integer := BASE_PERIOD_MS*50000 - 1;
constant MULTIPLIER : integer := 200; -- 200*20ns = 4us Resolution :: 1ms/4us = 250 steps
constant MIN_OFFSET : integer := 50000; -- Minimum 1ms

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
signal pwm_config : std_logic := '0';
signal pwm_ready : std_logic := '0';
signal pwm_data_vector : std_logic_vector(7 downto 0) := (others => '0');

begin

pwm_process:
process(CLOCK_50MHz, RESET)
    variable pwm_integer : integer := 50000; -- This is equal to MIN_OFFSET 1ms
begin
    if RESET = '1' then
        pwm_integer := 50000;
        pwm_state <= IDLE;
        pwm_base_pulse <= '0';
        pwm_pulse <= '0';
        pwm_width <= (others => '0');
        pwm_base_timer <= (others => '0');
        pwm_timer <= (others => '0');
        pwm_config <= '0';
        pwm_ready <= '0';
        pwm_data_vector <= (others => '0');
        FPGA_INT <= '0';
        PWM_SIGNAL <= '0';
    elsif rising_edge(CLOCK_50MHz) then
        if pwm_base_timer = std_logic_vector(to_unsigned(DEFAULT_PERIOD, 20)) then -- 20ms PWM Period
            pwm_base_timer <= (others => '0');
            pwm_base_pulse <= '1';
        else
            pwm_base_timer <= pwm_base_timer + '1';
            pwm_base_pulse <= '0';
        end if;

        -- Check if new data vector is ready to store
        if OFFLOAD_INT = '1' then
            pwm_ready <= '1';
            pwm_data_vector <= PWM_VECTOR;
        end if;

        -- PWM 20ms sync
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
                    if to_integer(unsigned(pwm_data_vector)) <= 250 then
                        pwm_integer := MIN_OFFSET + MULTIPLIER * to_integer(unsigned(pwm_data_vector));
                    else
                        pwm_integer := MIN_OFFSET + MULTIPLIER * 250; -- Max value
                    end if;
                    pwm_ready <= '0';
                    FPGA_INT <= '1'; -- Return signal to Kernel to unblock SM
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
                pwm_state <= IDLE;

            when others =>
                pwm_state <= IDLE;

        end case;

        PWM_SIGNAL <= pwm_pulse;

    end if;
end process;

end rtl;
