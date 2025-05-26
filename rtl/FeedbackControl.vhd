library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity FeedbackControl is
generic
(
    INTERRUPT_LENGTH_US : integer := 100
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    TRIGGER : in std_logic;
    RETURN_DATA : out std_logic_vector(7 downto 0);
    RETURN_INTERRUPT : out std_logic
);
end entity FeedbackControl;

architecture rtl of FeedbackControl is

type FEEDBACK_SM is
(
    FEEDBACK_IDLE,
    FEEDBACK_PRODUCE,
    FEEDBACK_DONE
);
signal feedback_state: FEEDBACK_SM := FEEDBACK_IDLE;

begin

    feedback_control_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case feedback_state is
                when FEEDBACK_IDLE =>

                when FEEDBACK_PRODUCE =>

                when FEEDBACK_DONE =>

                when others =>
                    feedback_state <= FEEDBACK_IDLE;
            end case;
        end if;
    end process;

end architecture rtl;
