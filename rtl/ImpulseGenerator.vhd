library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity ImpulseGenerator is
    Port (
        CLOCK : in  std_logic;
        interrupt_period : in  std_logic_vector(25 downto 0);
        interrupt_length : in  std_logic_vector(3 downto 0);
        interrupt_signal : out std_logic
    );
end entity ImpulseGenerator;

architecture rtl of ImpulseGenerator is
    signal internal_interrupt_length : std_logic_vector(3 downto 0) := (others => '0');
    signal internal_interrupt_period : std_logic_vector(25 downto 0) := (others => '0');
    signal internal_interrupt_signal : std_logic := '0';
    signal internal_interrupt_cutoff : std_logic := '0';

begin
    interrupt_process: process(CLOCK)
    begin
        if rising_edge(CLOCK) then
            if internal_interrupt_cutoff = '1' then
                internal_interrupt_length <= std_logic_vector(unsigned(internal_interrupt_length) + 1);
                if internal_interrupt_length = interrupt_length then
                    internal_interrupt_signal <= '0';
                    internal_interrupt_cutoff <= '0';
                    internal_interrupt_length <= (others => '0');
                end if;
            end if;

            internal_interrupt_period <= std_logic_vector(unsigned(internal_interrupt_period) + 1);

            if internal_interrupt_period = interrupt_period then
                if internal_interrupt_signal = '0' then
                    internal_interrupt_signal <= '1';
                end if;
                internal_interrupt_period <= (others => '0');
                internal_interrupt_cutoff <= '1';
            end if;
        end if;
    end process interrupt_process;

    -- Output assignment
    interrupt_signal <= internal_interrupt_signal;

end architecture rtl;
