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
    clock : in std_logic;

    button_in : in std_logic;
    button_out : out std_logic
);
end DebounceController;

architecture rtl of DebounceController is

    signal stable_count : integer := 0;

    type DEBOUNCE is
    (
        IDLE,
        WAITING,
        STABLE,
        DONE
    );

    signal debounce_state: DEBOUNCE := IDLE;

    begin

    process(clock, button_in, stable_count)
    begin
        if rising_edge(clock) then

            case debounce_state is

                when IDLE =>
                    if button_in = '0' then
                        button_out <= '0';
                        debounce_state <= WAITING;
                    else
                        button_out <= '0';
                        debounce_state <= IDLE;
                    end if;

                when WAITING =>
                    if stable_count = (PERIOD - SM_OFFSET) then
                        button_out <= '0';
                        stable_count <= 0;
                        debounce_state <= STABLE;
                    else
                        if button_in = '0' then
                            button_out <= '0';
                            stable_count <= stable_count + 1;
                            debounce_state <= WAITING;
                        else
                            button_out <= '0';
                            stable_count <= 0;
                            debounce_state <= IDLE;
                        end if;
                    end if;

                when STABLE =>
                    if button_in = '0' then
                        button_out <= '1';
                        debounce_state <= STABLE;
                    else
                        button_out <= '1';
                        debounce_state <= DONE;
                    end if;

                when DONE =>
                    if stable_count = (PERIOD - SM_OFFSET) then
                        button_out <= '1';
                        stable_count <= 0;
                        debounce_state <= IDLE;
                    else
                        if button_in = '1' then
                            button_out <= '1';
                            stable_count <= stable_count + 1;
                            debounce_state <= DONE;
                        else
                            button_out <= '1';
                            stable_count <= 0;
                            debounce_state <= STABLE;
                        end if;
                    end if;

                when others =>
                    debounce_state <= IDLE;

            end case;
        end if;
    end process;

end rtl;
