library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity SPI_Synchronizer is
Port 
(
    CLK_50MHz : in  STD_LOGIC;
    IN_SCLK   : in  STD_LOGIC;
    IN_CS     : in  STD_LOGIC;
    IN_MOSI   : in  STD_LOGIC;
    OUT_SCLK  : out STD_LOGIC;
    OUT_CS    : out STD_LOGIC;
    OUT_MOSI  : out STD_LOGIC
);
end SPI_Synchronizer;

architecture Behavioral of SPI_Synchronizer is

signal synced_sclk  : STD_LOGIC := '0';
signal synced_cs    : STD_LOGIC := '0';
signal synced_mosi  : STD_LOGIC := '0';

begin

process(CLK_50MHz)
begin
    if rising_edge(CLK_50MHz) then
        synced_sclk <= IN_SCLK;
        synced_cs   <= IN_CS;
        synced_mosi <= IN_MOSI;
    end if;
end process;

process(CLK_50MHz)
begin
    if rising_edge(CLK_50MHz) then
        OUT_SCLK <= synced_sclk;
        OUT_CS   <= synced_cs;
        OUT_MOSI <= synced_mosi;
    end if;
end process;

end Behavioral;