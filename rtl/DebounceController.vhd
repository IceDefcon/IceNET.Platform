library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity DebounceController is
generic 
(
    PERIOD : integer := 50000; -- 50Mhz :: 50000*20ns = 1ms
    SM_OFFSET : integer := 3
);
port 
(    
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    BUTTON_IN : in std_logic;
    BUTTON_OUT : out std_logic
);
end DebounceController;

architecture rtl of DebounceController is

    signal stable_count : integer := 0;

    type DEBOUNCE_TYPE is
    (
        DEBOUNCE_IDLE,
        DEBOUNCE_WAITING,
        DEBOUNCE_STABLE,
        DEBOUNCE_DONE
    );

    signal debounce_state: DEBOUNCE_TYPE := DEBOUNCE_IDLE;

    begin

    process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            stable_count <= 0;
            debounce_state <= DEBOUNCE_IDLE;
            BUTTON_OUT <= '0';
        elsif rising_edge(CLOCK_50MHz) then

            case debounce_state is

                when DEBOUNCE_IDLE =>
                    if BUTTON_IN = '0' then
                        BUTTON_OUT <= '0';
                        debounce_state <= DEBOUNCE_WAITING;
                    else
                        BUTTON_OUT <= '0';
                        debounce_state <= DEBOUNCE_IDLE;
                    end if;

                when DEBOUNCE_WAITING =>
                    if stable_count = (PERIOD - SM_OFFSET) then
                        BUTTON_OUT <= '0';
                        stable_count <= 0;
                        debounce_state <= DEBOUNCE_STABLE;
                    else
                        if BUTTON_IN = '0' then
                            BUTTON_OUT <= '0';
                            stable_count <= stable_count + 1;
                            debounce_state <= DEBOUNCE_WAITING;
                        else
                            BUTTON_OUT <= '0';
                            stable_count <= 0;
                            debounce_state <= DEBOUNCE_IDLE;
                        end if;
                    end if;

                when DEBOUNCE_STABLE =>
                    if BUTTON_IN = '0' then
                        BUTTON_OUT <= '1';
                        debounce_state <= DEBOUNCE_STABLE;
                    else
                        BUTTON_OUT <= '1';
                        debounce_state <= DEBOUNCE_DONE;
                    end if;

                when DEBOUNCE_DONE =>
                    if stable_count = (PERIOD - SM_OFFSET) then
                        BUTTON_OUT <= '1';
                        stable_count <= 0;
                        debounce_state <= DEBOUNCE_IDLE;
                    else
                        if BUTTON_IN = '1' then
                            BUTTON_OUT <= '1';
                            stable_count <= stable_count + 1;
                            debounce_state <= DEBOUNCE_DONE;
                        else
                            BUTTON_OUT <= '1';
                            stable_count <= 0;
                            debounce_state <= DEBOUNCE_STABLE;
                        end if;
                    end if;

                when others =>
                    debounce_state <= DEBOUNCE_IDLE;

            end case;
        end if;
    end process;

end rtl;
