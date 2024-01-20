library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity StateMachine is
    Port ( clk : in std_logic;
           reset : in std_logic;
           condition1 : in std_logic;
           condition2 : in std_logic;
           state_out : out std_logic_vector (2 downto 0));
end StateMachine;

architecture rtl of StateMachine is
    type State_Type is (State1, State2, State3);
    signal current_state, next_state : State_Type;

begin
    process(clk, reset)
    begin
        if reset = '1' then
            current_state <= State1;  -- Initial state
        elsif rising_edge(clk) then
            current_state <= next_state;
        end if;
    end process;

    process(current_state, condition1, condition2)
    begin
        next_state <= current_state;  -- Default next state

        case current_state is
            when State1 =>
                if condition1 = '1' then
                    next_state <= State2;  -- Transition to State2
                end if;

            when State2 =>
                if condition2 = '1' then
                    next_state <= State3;  -- Transition to State3
                end if;

            when State3 =>
                -- No further transitions from State3 in this example

            when others =>
                next_state <= State1;  -- Default next state
        end case;
    end process;

    -- Output the current state
    process(current_state)
    begin
        case current_state is
            when State1 =>
                state_out <= "000";
            when State2 =>
                state_out <= "001";
            when State3 =>
                state_out <= "010";
            when others =>
                state_out <= "000";  -- Default state
        end case;
    end process;

end rtl;
