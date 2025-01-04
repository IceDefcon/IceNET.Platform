library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity OffloadControler_tb is
end OffloadControler_tb;

architecture tb of OffloadControler_tb is

    component OffloadControler is
    port
    (
        CLOCK_50MHz : in std_logic;

        OFFLOAD_INTERRUPT : in std_logic;
        FIFO_DATA : in std_logic_vector(7 downto 0);

        FIFO_READ_ENABLE : out std_logic;
        OFFLOAD_READY : out std_logic
    );
    end component;

    signal CLOCK_50MHz : std_logic := '0';

    signal OFFLOAD_INTERRUPT : std_logic := '0';
    signal FIFO_DATA : std_logic_vector(7 downto 0) := (others => '0');
    signal FIFO_READ_ENABLE : std_logic;
    signal OFFLOAD_READY : std_logic;

    constant CLOCK_PERIOD : time := 20 ns;  -- 50 MHz CLOCK_50MHz

    signal end_sim : boolean := false;

begin

    -- Instantiate the OffloadControler component
    uut: OffloadControler
    port map
    (
        CLOCK_50MHz         => CLOCK_50MHz,
        -- IN
        OFFLOAD_INTERRUPT   => OFFLOAD_INTERRUPT,
        FIFO_DATA           => FIFO_DATA,
        -- OUT
        FIFO_READ_ENABLE    => FIFO_READ_ENABLE,
        OFFLOAD_READY       => OFFLOAD_READY
    );

    -- CLOCK_50MHz generation process
    clock_process : process
    begin
        while not end_sim loop
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
        -- Apply input stimuli
        wait for 1000 ns;  -- Initial wait
        OFFLOAD_INTERRUPT <= '1';  -- Trigger interrupt
        --FIFO_DATA <= "10101010";  -- Example data
        wait for 1020 ns;
        OFFLOAD_INTERRUPT <= '0';
        --FIFO_DATA <= "11110000";

        -- Additional stimulus (as needed)
        wait for 6000 ns;

        -- End simulation
        end_sim <= true;
        wait;
    end process stimulus_process;

end tb;
