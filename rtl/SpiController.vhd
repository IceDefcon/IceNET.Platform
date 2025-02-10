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

signal transfer_counter : integer range 0 to 1024 := 0;

constant CLOCK_CYCLE : integer range 0 to 16 := 10; -- 10 cycles
constant BREAK_START : integer range 0 to 512 := 512; -- 512 cycles
constant BREAK_TRANSFER : integer range 0 to 128 := 80; -- 80 cycles
constant BREAK_END : integer range 0 to 256 := 256; -- 256 cycles

begin

    spiControl_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case SPI_state is

                when SPI_IDLE =>
                    CTRL_CS <= '1';
                    CTRL_MOSI <= '1';
                    CTRL_SCK <= '0';
                    if OFFLOAD_INT = '1' then
                        SPI_state <= SPI_CONFIG;
                    end if;

                when SPI_CONFIG =>
                    SPI_state <= SPI_PRODUCE;

                when SPI_PRODUCE =>
                    if transfer_counter = BREAK_START + BREAK_TRANSFER + BREAK_END then
                        CTRL_CS <= '1';
                        SPI_state <= SPI_MUX;
                    else
                        CTRL_CS <= '0';
                        transfer_counter <= transfer_counter + 1;
                    end if;

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
