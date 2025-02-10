library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.numeric_std.all;

entity temp_tb is
end temp_tb;

architecture tb of temp_tb is

    -- Component declaration for the PWM
    component test_component is
        port(
            CLOCK_50MHz   : in  std_logic;
            xxx : out std_logic;
            yyy : out std_logic
        );
    end component;

    -- Signals to connect to the DUT (Device Under Test)
    signal CLOCK_50MHz : std_logic := '0';
    signal test_xxx : std_logic := '0';
    signal test_yyy : std_logic := '0';

    -- CLOCK_50MHz period definition
    constant CLOCK_PERIOD : time := 20 ns;  -- 50 MHz CLOCK_50MHz

begin

    -- Instantiate the PWM component
    uut: test_component
        port map (
            CLOCK_50MHz => CLOCK_50MHz,
            xxx => test_xxx,
            yyy => test_yyy
        );

    -- CLOCK_50MHz generation process
    clock_process : process
    begin
        while true loop
            CLOCK_50MHz <= '0';
            wait for CLOCK_PERIOD / 2;
            CLOCK_50MHz <= '1';
            wait for CLOCK_PERIOD / 2;
        end loop;
        wait;
    end process clock_process;

    -- Simulation process
    stimulus_process : process
    begin
        -- Wait for 100 ms to observe some PWM cycles
        wait for 2000000 ns;
        
        -- Stop the simulation
        assert false report "End of simulation" severity note;
        wait;
    end process stimulus_process;

end tb;
