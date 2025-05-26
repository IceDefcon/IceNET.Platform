library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity DmaTriggerControl is
generic
(
    INTERRUPT_LENGTH_US : integer := 100
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    TRIGGER_LONG : in std_logic;
    TRIGGER_SHORT : in std_logic;

    RETURN_INTERRUPT : out std_logic
);
end entity DmaTriggerControl;

architecture rtl of DmaTriggerControl is

constant INTERRUPT_LENGTH : integer range 0 to 5000 := 50 * INTERRUPT_LENGTH_US;

signal feedbck_interrupt_logic : std_logic := '0';
signal secondary_dma_trigger_gpio_pulse_long : std_logic := '0';
signal feedback_interrupt_counter : integer := 0;

begin

    feedback_control_process:
    process(CLOCK_50MHz)
    begin

        if RESET = '1' then
            feedbck_interrupt_logic <= '0';
            secondary_dma_trigger_gpio_pulse_long <= '0';
            feedback_interrupt_counter <= 0;
        elsif rising_edge(CLOCK_50MHz) then
            ----------------------------------------------------
            -- Short to Long Transition
            ----------------------------------------------------
            if TRIGGER_SHORT = '1' then
                secondary_dma_trigger_gpio_pulse_long <= '1';
            end if;

            if TRIGGER_LONG = '1' or secondary_dma_trigger_gpio_pulse_long = '1' then
                if feedback_interrupt_counter = INTERRUPT_LENGTH then -- 5000 * 20 = 100us interrupt pulse back to CPU
                    RETURN_INTERRUPT <= '0';
                    feedbck_interrupt_logic <= '0';
                    secondary_dma_trigger_gpio_pulse_long <= '0';
                else
                    RETURN_INTERRUPT <= '1';
                    feedbck_interrupt_logic <= '1';
                    feedback_interrupt_counter <= feedback_interrupt_counter + 1;
                end if;
            else
                RETURN_INTERRUPT <= '0';
                feedbck_interrupt_logic <= '0';
                feedback_interrupt_counter <= 0;
            end if;

        end if;
    end process;

end architecture rtl;
