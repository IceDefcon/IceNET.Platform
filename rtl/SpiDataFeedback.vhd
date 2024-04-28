library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SpiProcessing is
Port (
    CLOCK : in std_logic;
    
    CS : in std_logic;
    SCLK : in std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);

    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic
);
end entity SpiProcessing;

architecture rtl of SpiProcessing is

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

            if CS = '0' then
                if SCLK = '1' then
                    run <= '1';
                    count <= count + '1';
                    SERIAL_MISO <= PARALLEL_MISO(7 - index);
                elsif SCLK = '0' then
                    if count > 0 then
                        count <= count - '1';
                    end if;
                    SERIAL_MISO <= PARALLEL_MISO(7 - index);
                    if count_bit = "0111" then
                        run <= '0';
                        count_bit <= "0000";
                    end if;
                end if;
            end if;
        end if;    
    end process spi_process;

    -- Other signal assignments as needed
    -- ...
end architecture rtl;
