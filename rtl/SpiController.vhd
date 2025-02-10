library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

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
    SPI_INIT,
    SPI_CONFIG,
    SPI_PRODUCE,
    SPI_MUX,
    SPI_DONE
);

signal SPI_state: SPI_CONTROLLER_TYPE := SPI_IDLE;

signal transfer_counter : integer range 0 to 1024 := 0;

constant CLOCK_CYCLE : integer range 0 to 16 := 10; -- 10 cycles
constant TRANSFER_START : integer range 0 to 512 := 512; -- 512 cycles
constant TRANSFER_END : integer range 0 to 128 := 80; -- 80 cycles
constant BUS_FREE : integer range 0 to 256 := 256; -- 256 cycles

signal spi_status : std_logic_vector(3 downto 0) := "0000";
signal sck_timer : std_logic_vector(3 downto 0) := "0000";
signal sck_timer_toggle : std_logic := '0';
signal index : integer range 0 to 15 := 0;
signal bytes : integer range 0 to 15 := 0;
signal bytes_count : integer range 0 to 15 := 0;

begin

    spiControl_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case SPI_state is

                when SPI_IDLE =>
                    spi_status <= "0000";
                    CTRL_CS <= '1';
                    CTRL_MOSI <= '1';
                    CTRL_SCK <= '0';
                    if OFFLOAD_INT = '1' then
                        SPI_state <= SPI_INIT;
                    end if;

                when SPI_INIT =>
                    bytes <= 2; -- This can be parametrized from kernel
                    SPI_state <= SPI_CONFIG;

                when SPI_CONFIG =>
                    if bytes_count = bytes then
                        bytes_count <= 0;
                        SPI_state <= SPI_MUX;
                    else
                        bytes_count <= bytes_count + 1;
                        index <= 0;
                        SPI_state <= SPI_PRODUCE;
                    end if;

                when SPI_PRODUCE =>
                    if transfer_counter = TRANSFER_START + TRANSFER_END + BUS_FREE then
                        transfer_counter <= 0;
                        spi_status <= "0000";
                        sck_timer <= "0000";
                        SPI_state <= SPI_CONFIG;
                    else
------------------------------------------------------------------------------------------------------------------------------
-- PIPE[0] :: Transfer Counter
------------------------------------------------------------------------------------------------------------------------------
                        if transfer_counter = TRANSFER_START then
                            spi_status <= "0001";
                        end if;

                        if transfer_counter = TRANSFER_START + TRANSFER_END then
                            spi_status <= "0010";
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: SPI Status
------------------------------------------------------------------------------------------------------------------------------
                        if spi_status = "0001" then

                            if sck_timer = "0100" then -- Half bit time
                                sck_timer_toggle <= not sck_timer_toggle;

                                if sck_timer_toggle = '1' then
                                    sck_timer <= (others => '0');
                                    -----------------------------------------
                                    -- CLOCK [0]
                                    -----------------------------------------
                                    CTRL_SCK <= '0';
                                else
                                    sck_timer <= (others => '0');
                                    -----------------------------------------
                                    -- CLOCK [1]
                                    -----------------------------------------
                                    CTRL_SCK <= '1';
                                    -----------------------------------------
                                    -- DATA @ Rising Edge of the clock !
                                    -----------------------------------------
                                    CTRL_MOSI <= OFFLOAD_REGISTER(7 - index);
                                    index <= index + 1;
                                end if;
                            else
                                sck_timer <= sck_timer + '1';
                            end if;
                        end if;

                        if spi_status = "0010" then
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: Increment Status Timer
------------------------------------------------------------------------------------------------------------------------------
                        transfer_counter <= transfer_counter + 1;
                    end if;

                when SPI_MUX =>
                    CTRL_CS <= '1';
                    SPI_state <= SPI_DONE;

                when SPI_DONE =>
                    SPI_state <= SPI_IDLE;

                when others =>
                    SPI_state <= SPI_IDLE;
            end case;
        end if;
    end process;
end architecture rtl;
