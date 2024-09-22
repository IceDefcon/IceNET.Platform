library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity InterruptGenerator is
generic
(
    PERIOD_MS : integer := 1000;
    PULSE_LENGTH : integer := 50
);
Port
(
    CLOCK_50MHz : in  std_logic;
    INTERRUPT_SIGNAL : out std_logic
);
end entity InterruptGenerator;

architecture rtl of InterruptGenerator is

    constant SM_OFFSET : integer := 4;
    signal period_count : integer := 0;
    signal length_count : integer := 0;
    signal pulse : std_logic := '0';
    type INTERRUPT is
    (
        IDLE,
        PERIOD,
        PRODUCE,
        DONE
    );
    signal interrupt_state: INTERRUPT := IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case interrupt_state is

                when IDLE =>
                    interrupt_state <= PERIOD;

                when PERIOD =>
                    if period_count = (PERIOD_MS * 50000 - SM_OFFSET - PULSE_LENGTH) then
                        period_count <= 0;
                        interrupt_state <= PRODUCE;
                    else
                        period_count <= period_count + 1;
                        interrupt_state <= PERIOD;
                    end if;

                when PRODUCE =>
                    if length_count = PULSE_LENGTH then
                        pulse <= '0';
                        length_count <= 0;
                        interrupt_state <= DONE;
                    else
                        pulse <= '1';
                        length_count <= length_count + 1;
                        interrupt_state <= PRODUCE;
                    end if;

                when DONE =>
                    interrupt_state <= IDLE;

                when others =>
                    interrupt_state <= IDLE;

            end case;

            INTERRUPT_SIGNAL <= pulse;

        end if;
    end process interrupt_process;
end architecture rtl;
