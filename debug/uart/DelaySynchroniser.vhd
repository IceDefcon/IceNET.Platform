library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity DelaySynchroniser is
generic
(
    SYNCHRONIZATION_DEPTH : integer := 2
);
Port
(
    CLOCK : in  std_logic;
    RESET : in std_logic;

    ASYNC_INPUT : in std_logic;
    SYNC_OUTPUT : out std_logic
);
end entity DelaySynchroniser;

architecture rtl of DelaySynchroniser is

signal sync_vector : std_logic_vector(SYNCHRONIZATION_DEPTH - 1 downto 0) := (others => '0');

begin

    sync_process:
    process(CLOCK, RESET)
    begin
        if RESET = '1' then
            sync_vector <= (others => '0');
            SYNC_OUTPUT <= '0';
        elsif rising_edge(CLOCK) then
            for i in SYNCHRONIZATION_DEPTH - 1 downto 1 loop
                sync_vector(i) <= sync_vector(i - 1);
            end loop;
            sync_vector(0) <= ASYNC_INPUT;
            SYNC_OUTPUT <= sync_vector(SYNCHRONIZATION_DEPTH - 1);
        end if;
    end process sync_process;

end architecture rtl;
