library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SpiProcessing is
Port (
    CLOCK : in std_logic;
    
    CS : in std_logic;
    SCLK : in std_logic;

    SPI_INT : out std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);

    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic
);
end entity SpiProcessing;

architecture rtl of SpiProcessing is

signal pipe_1 : std_logic := '0';
signal pipe_2 : std_logic := '0';
signal count_bit : std_logic_vector(3 downto 0) := (others => '0');
signal index : integer range 0 to 15 := 0;
signal spi_ready : std_logic := '0';
signal synced_parallel_mosi : std_logic_vector(7 downto 0);

begin

spi_process: process(CLOCK)
begin
    if rising_edge(CLOCK) then

        index <= to_integer(unsigned(count_bit));

        if spi_ready = '1' then
            spi_ready <= '0';
        end if;

        if SCLK = '1' then
            pipe_1 <= '1';
        elsif SCLK = '0' and pipe_1 = '1' then
            pipe_2 <= '1';
        end if;

        if pipe_1 = '1' and pipe_2 = '1' then
            synced_parallel_mosi(7 - index) <= SERIAL_MOSI;

            pipe_1 <= '0';
            pipe_2 <= '0';
            count_bit <= count_bit + '1';
        end if;

        if count_bit = "0111" then
            spi_ready <= '1';
            PARALLEL_MOSI <= synced_parallel_mosi;
        end if;
    end if;    
end process;

SPI_INT <= spi_ready;

end architecture rtl;
