library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SpiConverter is
Port
(
    CLOCK : in std_logic;
    RESET : in std_logic;
    
    CS : in std_logic;
    SCLK : in std_logic;

    SERIAL_MOSI : in std_logic;
    PARALLEL_MOSI : out std_logic_vector(7 downto 0);
    PARALLEL_MISO : in std_logic_vector(7 downto 0);
    SERIAL_MISO : out std_logic;

    CONVERSION_BIT_COUNT : out std_logic_vector(3 downto 0);
    CONVERSION_COMPLETE : out std_logic
);
end entity SpiConverter;

architecture rtl of SpiConverter is

signal sclk_hi : std_logic := '0';
signal sclk_lo : std_logic := '0';
signal spi_bit_count : std_logic_vector(3 downto 0) := (others => '0');
signal index : integer range 0 to 15 := 0;
signal spi_conversion_complete : std_logic := '0';
signal synced_parallel_mosi : std_logic_vector(7 downto 0);

begin

spi_process: process(CLOCK, RESET)
begin
    if RESET = '1' then
        sclk_hi <= '0';
        sclk_lo <= '0';
        spi_bit_count <= (others => '0');
        index <= 0;
        spi_conversion_complete <= '0';
        synced_parallel_mosi <= (others => '0');
        PARALLEL_MOSI <= (others => '0');
        SERIAL_MISO <= '0';
    elsif rising_edge(CLOCK) then

        if CS = '0' then

            index <= to_integer(unsigned(spi_bit_count));
            CONVERSION_BIT_COUNT <= spi_bit_count;

            if spi_conversion_complete = '1' then -- Interrupt bit for 20ns only
                spi_conversion_complete <= '0'; -- Then pull Low
            end if;

            if SCLK = '1' then
                sclk_hi <= '1';
            elsif SCLK = '0' and sclk_hi = '1' then
                sclk_lo <= '1';
            end if;

            if sclk_hi = '1' then
                SERIAL_MISO <= PARALLEL_MISO((7 - index));
            end if;

            if sclk_hi = '1' and sclk_lo = '1' then
                synced_parallel_mosi(7 - index) <= SERIAL_MOSI;

                sclk_hi <= '0';
                sclk_lo <= '0';
                spi_bit_count <= spi_bit_count + '1';
            end if;

            if spi_bit_count = "1000" then
                spi_conversion_complete <= '1'; -- Generate interrupt for I2C state machine
                spi_bit_count <= (others => '0');
                PARALLEL_MOSI <= synced_parallel_mosi;
            end if;
        end if;
    end if;    
end process;

CONVERSION_COMPLETE <= spi_conversion_complete;

end architecture rtl;
