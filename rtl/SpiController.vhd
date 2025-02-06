library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SpiController is
Port
(
    CLOCK_50MHz : in  std_logic;

    OFFLOAD_INT : in std_logic;

    MISO : in std_logic;
    MOSI : out std_logic;
    SCK : out std_logic;
    CSN : out std_logic;

    FPGA_INT : out std_logic
);
end entity SpiController;

architecture rtl of SpiController is

type INTERRUPT is
(
    IDLE,
    PRODUCE,
    DONE
);
signal SPI_state: INTERRUPT := IDLE;

begin

    FPGA_INT <= OFFLOAD_INT;


    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case SPI_state is
                when IDLE =>

                when PRODUCE =>

                when DONE =>

                when others =>
                    SPI_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
