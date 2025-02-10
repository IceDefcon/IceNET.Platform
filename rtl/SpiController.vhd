library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SpiController is
Port
(
    CLOCK_50MHz : in  std_logic;

    OFFLOAD_INT : in std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_CONTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;

    CTRL_MUX : out std_logic_vector(3 downto 0);

    FPGA_INT : out std_logic;
    FEEDBACK_DATA : out std_logic_vector(7 downto 0)
);
end entity SpiController;

architecture rtl of SpiController is

type SPI_CONTROLLER_TYPE is
(
    SPI_IDLE,
    SPI_CONFIG,
    SPI_PRODUCE,
    SPI_MUX,
    SPI_DONE
);

signal SPI_state: SPI_CONTROLLER_TYPE := SPI_IDLE;

constant BREAK_START : std_logic_vector(9 downto 0) := "1000000000";
constant BREAK_END : std_logic_vector(9 downto 0) := "0100000000";

begin

    spiControl_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case SPI_state is

                when SPI_IDLE =>
                    if OFFLOAD_INT = '1' then
                        SPI_state <= SPI_CONFIG;
                    end if;

                when SPI_CONFIG =>
                    SPI_state <= SPI_PRODUCE;

                when SPI_PRODUCE =>
                    SPI_state <= SPI_MUX;

                when SPI_MUX =>
                    SPI_state <= SPI_DONE;

                when SPI_DONE =>
                    SPI_state <= SPI_IDLE;

                when others =>
                    SPI_state <= SPI_IDLE;
            end case;
        end if;
    end process;
end architecture rtl;
