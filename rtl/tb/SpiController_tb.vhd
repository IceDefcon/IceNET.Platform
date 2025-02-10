library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.numeric_std.all;

entity SpiController_tb is
end SpiController_tb;

architecture tb of SpiController_tb is

component SpiController is
Port
(
    CLOCK_50MHz : in  std_logic;

    OFFLOAD_INT : in std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;

    CTRL_MUX : out std_logic_vector(3 downto 0);

    FPGA_INT : out std_logic;
	FEEDBACK_DATA : out std_logic_vector(7 downto 0)
);
end component;

signal CLOCK_50MHz : std_logic := '0';

signal switch_spi_ready : std_logic := '0';
signal offload_id : std_logic_vector(6 downto 0) := "0000100";
signal offload_ctrl : std_logic_vector(7 downto 0) := "10000010";
signal offload_register : std_logic_vector(7 downto 0) := "00010000";
signal offload_data : std_logic_vector(7 downto 0) := "00000000";

signal ctrl_CS : std_logic := '0';
signal ctrl_MISO : std_logic := '0';
signal ctrl_MOSI : std_logic := '0';
signal ctrl_SCLK : std_logic := '0';

signal spi_mux : std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_spi_feedback : std_logic := '0';
signal data_spi_feedback : std_logic_vector(7 downto 0) := "00010001";

constant INITIAL_BREAK : time := 1000 ns;
constant CLOCK_PERIOD : time := 20 ns;

begin

    SpiController_module: SpiController 
    port map
    (
        CLOCK_50MHz => CLOCK_50MHz,

        OFFLOAD_INT => switch_spi_ready,

        OFFLOAD_ID => offload_id,
        OFFLOAD_CONTROL => offload_ctrl,
        OFFLOAD_REGISTER => offload_register,
        OFFLOAD_DATA => offload_data,

        -- Master SPI interface
        CTRL_CS => ctrl_CS,
        CTRL_MISO => ctrl_MISO,
        CTRL_MOSI => ctrl_MOSI,
        CTRL_SCK => ctrl_SCLK,

        CTRL_MUX => spi_mux,

        FPGA_INT => interrupt_spi_feedback,
        FEEDBACK_DATA => data_spi_feedback
    );

    ------------------------------------
    -- CLOCK_50MHz
    ------------------------------------
    clock_process:
    process
    begin
        while true loop
            CLOCK_50MHz <= '0';
            wait for CLOCK_PERIOD / 2;
            CLOCK_50MHz <= '1';
            wait for CLOCK_PERIOD / 2;
        end loop;
        wait;
    end process;

    ------------------------------------
    -- OFFLOAD_INTERRUPT
    ------------------------------------
    offload_interrupt_process: process
    begin
        switch_spi_ready <= '0';
        wait for INITIAL_BREAK;
        switch_spi_ready <= '1';
        wait for CLOCK_PERIOD;
        switch_spi_ready <= '0';
        wait;  -- This will stop the process
    end process;

end tb;
