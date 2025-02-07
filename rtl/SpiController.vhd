library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SpiController is
Port
(
    CLOCK_50MHz : in  std_logic;

    OFFLOAD_INT : in std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_COTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;

    CTRL_MUX : out std_logic_vector(3 downto 0);

    FPGA_INT : out std_logic
);
end entity SpiController;

architecture rtl of SpiController is

type INTERRUPT is
(
    IDLE,
    PRODUCE,
    MUX,
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

                when MUX =>

                when DONE =>

                when others =>
                    SPI_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
