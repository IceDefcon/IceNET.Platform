library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity InterruptGenerator is
generic
(
    PERIOD_MS : integer := 1000;
    PULSE_LENGTH : integer := 50;
    TRESHOLD_LENGTH : integer := 1000
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    INTERRUPT_SIGNAL : out std_logic;
    THRESHOLD : out std_logic
);
end entity InterruptGenerator;

architecture rtl of InterruptGenerator is

    constant SM_OFFSET : integer := 4;
    constant PERIOD_LENGTH : integer := (PERIOD_MS * 50000 - SM_OFFSET - PULSE_LENGTH);

    signal period_count : integer := 0;
    signal length_count : integer := 0;
    signal pulse : std_logic := '0';

    type INTERRUPT_TYPE is
    (
        INTERRUPT_IDLE,
        INTERRUPT_PERIOD,
        INTERRUPT_PRODUCE,
        INTERRUPT_DONE
    );
    signal interrupt_state : INTERRUPT_TYPE := INTERRUPT_IDLE;
    signal thresholded_time : std_logic := '0';

begin

    interrupt_process: process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            thresholded_time <= '0';
            period_count <= 0;
            length_count <= 0;
            pulse <= '0';
            interrupt_state <= INTERRUPT_IDLE;
            INTERRUPT_SIGNAL <= '0';
        elsif rising_edge(CLOCK_50MHz) then

            case interrupt_state is

                when INTERRUPT_IDLE =>
                    interrupt_state <= INTERRUPT_PERIOD;

                when INTERRUPT_PERIOD =>
                    if period_count = PERIOD_LENGTH then
                        period_count <= 0;
                        interrupt_state <= INTERRUPT_PRODUCE;
                    elsif period_count = (PERIOD_LENGTH - TRESHOLD_LENGTH + 1 + PULSE_LENGTH) then
                        thresholded_time <= '1';
                        period_count <= period_count + 1;
                        interrupt_state <= INTERRUPT_PERIOD;
                    else
                        period_count <= period_count + 1;
                        interrupt_state <= INTERRUPT_PERIOD;
                    end if;

                when INTERRUPT_PRODUCE =>
                    if length_count = PULSE_LENGTH then
                        pulse <= '0';
                        length_count <= 0;
                        thresholded_time <= '0';
                        interrupt_state <= INTERRUPT_DONE;
                    else
                        pulse <= '1';
                        length_count <= length_count + 1;
                        interrupt_state <= INTERRUPT_PRODUCE;
                    end if;

                when INTERRUPT_DONE =>
                    interrupt_state <= INTERRUPT_IDLE;

                when others =>
                    interrupt_state <= INTERRUPT_IDLE;

            end case;

            INTERRUPT_SIGNAL <= pulse;
            THRESHOLD <= thresholded_time;

        end if;
    end process interrupt_process;
end architecture rtl;
