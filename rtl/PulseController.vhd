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
    INPUT_PULSE : in std_logic;
    OUTPUT_PULSE : out std_logic
);
end entity PulseController;

architecture rtl of PulseController is

type PULSE is
(
    PULSE_IDLE,
    PULSE_PRODUCE,
    PULSE_WAIT,
    PULSE_DONE
);
signal pulse_state: PULSE := PULSE_IDLE;
signal pulse_count : integer range 0 to 64 := 0;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case pulse_state is
                when PULSE_IDLE =>
                    -- Duble Reset Values
                    pulse_count <= 0;
                    OUTPUT_PULSE <= '0';

                    if INPUT_PULSE = '1' then
                        pulse_state <= PULSE_PRODUCE;
                    end if;

                when PULSE_PRODUCE =>
                    OUTPUT_PULSE <= '1';
                    if PULSE_LENGTH = pulse_count + 1 then
                        pulse_count <= 0;
                        pulse_state <= PULSE_WAIT;
                    else
                        pulse_count <= pulse_count + 1;
                    end if;

                when PULSE_WAIT =>
                    OUTPUT_PULSE <= '0';
                    if INPUT_PULSE = '0' then
                        pulse_state <= PULSE_DONE;
                    end if;

                when PULSE_DONE =>
                    pulse_state <= PULSE_IDLE;

                when others =>
                    pulse_state <= PULSE_IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;

