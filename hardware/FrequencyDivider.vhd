library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity FrequencyDivider is
    port (
        clk_in : in std_logic;       -- 50 MHz input clock
        reset : in std_logic;        -- Asynchronous reset signal
        clk_out : out std_logic      -- 1 MHz output clock
    );
end entity FrequencyDivider;

architecture Behavioral of FrequencyDivider is
    signal counter : unsigned(4 downto 0) := (others => '0');
    signal clk_out_internal : std_logic := '0';

begin

    -- Counter process to divide the clock
    process(clk_in, reset)
    begin
        if reset = '1' then
            counter <= (others => '0');  -- Reset the counter
        elsif rising_edge(clk_in) then
            if counter = "11000" then   -- Counter value for divide-by-50 (1 MHz)
                counter <= (others => '0');
                clk_out_internal <= not clk_out_internal;  -- Toggle the internal clock output
            else
                counter <= counter + 1;  -- Increment the counter
            end if;
        end if;
    end process;

    -- Assign the output clock signal
    process(clk_in, reset)
    begin
        if reset = '1' then
            clk_out <= '0';  -- Initialize the output clock to '0' on reset
        elsif rising_edge(clk_in) then
            clk_out <= clk_out_internal;  -- Assign the internal clock output to the output port
        end if;
    end process;

end architecture Behavioral;
