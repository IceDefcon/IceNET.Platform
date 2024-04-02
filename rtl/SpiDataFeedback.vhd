library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SpiDataFeedback is
Port (
    CLOCK : in std_logic;
    SCLK : in std_logic;
    DATA : in std_logic_vector(7 downto 0);
    synced_miso : out std_logic
);
end entity SpiDataFeedback;

architecture rtl of SpiDataFeedback is

signal run : std_logic := '0';
signal count : std_logic_vector(4 downto 0) := (others => '0');
signal count_bit : std_logic_vector(3 downto 0) := (others => '0');
signal index : integer range 0 to 15 := 0;

signal synced_sclk : std_logic := '0';

begin
    spi_process: process(CLOCK)
    begin
        if rising_edge(CLOCK) then
            if run = '1' then
                if count = "00000" then
                    count_bit <= count_bit + '1';
                end if;
            end if;

            index <= to_integer(unsigned(count_bit));

            if SCLK = '1' then
                run <= '1';
                count <= count + '1';
                synced_miso <= DATA(7 - index); -- Assuming synchronous data
            elsif SCLK = '0' then
                if count > 0 then
                    count <= count - '1';
                end if;
                synced_miso <= DATA(7 - index); -- Assuming synchronous data
                if count_bit = "0111" then
                    run <= '0';
                    count_bit <= "0000";
                end if;
            end if;
        end if;    
    end process spi_process;

    -- Other signal assignments as needed
    -- ...
end architecture rtl;
