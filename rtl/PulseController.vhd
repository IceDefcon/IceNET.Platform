library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity PulseController is
generic
(
    PULSE_LENGTH : integer := 1
);
Port
(
    CLOCK_50MHz : in  std_logic;
    ENABLE_CONTROLLER : in std_logic;

    INPUT_PULSE : in std_logic;
    OUTPUT_PULSE : out std_logic
);
end entity PulseController;

architecture rtl of PulseController is

type PULSE is
(
    PULSE_IDLE,
    PULSE_INIT,
    PULSE_PRODUCE,
    PULSE_DONE
);

signal pulse_state: PULSE := PULSE_IDLE;
signal pulse_count : integer range 0 to 64 := 0;
signal wait_process : std_logic := '0';

-- Synchronization registers to prevent metastability
signal sync_input_pulse : std_logic_vector(1 downto 0) := "00";

begin

    ------------------------------------------------------------
    --
    -- Two stage synchroniser
    --
    -- Move sync_input_pulse(0) into sync_input_pulse(1)
    -- And INPUT_PULSE to sync_input_pulse(0)
    --
    ------------------------------------------------------------
    sync_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then
            sync_input_pulse <= sync_input_pulse(0) & INPUT_PULSE;
        end if;
    end process sync_process;

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then
            case pulse_state is
                when PULSE_IDLE =>
                    pulse_count <= 0;
                    OUTPUT_PULSE <= '0';
                    wait_process <= '0';

                    if ENABLE_CONTROLLER = '1' then
                        pulse_state <= PULSE_INIT;
                    end if;

                when PULSE_INIT =>
                    pulse_count <= 0;
                    OUTPUT_PULSE <= '0';
                    -------------------------------------------------------------------------------
                    --
                    -- Check against stable sync_input_pulse(1) :: Delay flip-flop doing the job
                    --
                    -------------------------------------------------------------------------------
                    if sync_input_pulse(1) = '1' and wait_process = '0' then
                        wait_process <= '1';
                        pulse_state <= PULSE_PRODUCE;
                    elsif sync_input_pulse(1) = '0' and wait_process = '1' then
                        wait_process <= '0';
                    end if;

                when PULSE_PRODUCE =>
                    OUTPUT_PULSE <= '1';
                    if pulse_count >= PULSE_LENGTH - 1 then
                        pulse_count <= 0;
                        pulse_state <= PULSE_DONE;
                    else
                        pulse_count <= pulse_count + 1;
                    end if;

                when PULSE_DONE =>
                    OUTPUT_PULSE <= '0';
                    pulse_state <= PULSE_INIT;

                when others =>
                    pulse_state <= PULSE_IDLE;
                    pulse_count <= 0;
                    OUTPUT_PULSE <= '0';
                    wait_process <= '0';
            end case;
        end if;
    end process interrupt_process;

end architecture rtl;
