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
    SPI_PROCESS,
    SPI_MUX,
    SPI_DONE
);
signal SPI_state: SPI_CONTROLLER_TYPE := SPI_IDLE;

signal byte_process_timer : integer range 0 to 1024 := 0;

constant TRANSFER_INIT : integer range 0 to 512 := 500;
constant BYTE_INIT : integer range 0 to 16   := 10;
constant BYTE_CLOCK : integer range 0 to 128 := 80;
constant BYTE_EXIT : integer range 0 to 16   := 10;
constant BYTE_BREAK : integer range 0 to 64  := 50;
constant TRANSFER_EXIT : integer range 0 to 256 := 250;

signal spi_status : std_logic_vector(3 downto 0) := "0000";
signal sck_timer : std_logic_vector(3 downto 0) := "0000";
signal sck_timer_toggle : std_logic := '0';
signal index : integer range 0 to 15 := 0;
signal bytes_amount : integer range 0 to 15 := 0;
signal bytes_count : integer range 0 to 15 := 0;
signal bytes_data : std_logic_vector(7 downto 0) := (others => '0');
signal bytes_register : std_logic_vector(7 downto 0) := (others => '0');

signal first_byte : std_logic := '0';
signal next_byte : std_logic := '0';
signal last_byte : std_logic := '0';

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
                        SPI_state <= SPI_INIT;
                    end if;

                when SPI_INIT =>
                    bytes_count <= 0;
                    spi_status <= "0000";
                    SPI_state <= SPI_CONFIG;
                    ----------------------------------------------------------------------
                    -- Parameters that are controlled from kernel
                    ----------------------------------------------------------------------
                    bytes_amount <= 1 + to_integer(unsigned(OFFLOAD_CONTROL(6 downto 3)));
                    bytes_data <= OFFLOAD_DATA;
                    bytes_register <= OFFLOAD_REGISTER;

                when SPI_CONFIG =>
                    ------------------------------
                    -- Finished :: Jump to MUX
                    ------------------------------
                    if bytes_count = bytes_amount then
                        SPI_state <= SPI_MUX;
                    else
                        --------------------------------
                        -- Set flags :: Jump to PROCESS
                        --------------------------------
                        if bytes_count = 0 then
                            first_byte <= '1';
                            next_byte <= '0';
                            last_byte <= '0';
                        end if;

                        if bytes_count > 0 and bytes_count < (bytes_amount - 1) then
                            first_byte <= '0';
                            next_byte <= '1';
                            last_byte <= '0';
                            bytes_register <= (others => '0'); -- Only this one is required
                        end if;

                        if bytes_count = bytes_amount - 1 then
                            first_byte <= '0';
                            next_byte <= '0';
                            last_byte <= '1';
                            bytes_register <= (others => '0'); -- Burst length is already set
                        end if;

                        index <= 0;
                        spi_status <= "0000";
                        bytes_count <= bytes_count + 1;
                        byte_process_timer <= 0;
                        SPI_state <= SPI_PROCESS;
                    end if;

                when SPI_PROCESS =>
                    if byte_process_timer = 1024 then
                    else
------------------------------------------------------------------------------------------------------------------------------
-- PIPE[0] :: Transfer Counter
------------------------------------------------------------------------------------------------------------------------------
                    if first_byte = '1' then
                        if byte_process_timer < TRANSFER_INIT then
                            spi_status <= "0001"; -- Transfer Init
                        elsif byte_process_timer < TRANSFER_INIT + BYTE_INIT then
                            spi_status <= "0010"; -- First Byte Init
                        elsif byte_process_timer < TRANSFER_INIT + BYTE_INIT + BYTE_CLOCK then
                            spi_status <= "0011"; -- Generic Byte Clock Process
                        elsif byte_process_timer < TRANSFER_INIT + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                            spi_status <= "0100"; -- First Byte Exit
                        else
                            spi_status <= "1110"; -- Going Back to CONFIG
                        end if;
                    end if;

                    if next_byte = '1' then
                        if byte_process_timer < BYTE_BREAK then
                            spi_status <= "0101"; -- Break Between Bytes
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT then
                            spi_status <= "0110"; -- Next Byte Init
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT + BYTE_CLOCK then
                            spi_status <= "0111"; -- Generic Byte Clock Process
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                            spi_status <= "1000"; -- Next Byte Exit
                        else
                            spi_status <= "1110"; -- Going Back to CONFIG
                        end if;
                    end if;

                    if last_byte = '1' then
                        if byte_process_timer < BYTE_BREAK then
                            spi_status <= "1001"; -- Break Between Bytes
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT then
                            spi_status <= "1010"; -- Last Byte Init
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT + BYTE_CLOCK then
                            spi_status <= "1011"; -- Generic Byte Clock Process
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                            spi_status <= "1100"; -- Last Byte Exit
                        elsif byte_process_timer < BYTE_BREAK + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT + TRANSFER_EXIT then
                            spi_status <= "1101"; -- Transfer Exit
                            FPGA_INT <= '1';
                        else
                            spi_status <= "1110"; -- Going Back to CONFIG -> IDLE
                        end if;
                    end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: SPI Status
------------------------------------------------------------------------------------------------------------------------------

                        ------------------------------------------------------
                        -- TRANSFER INIT + BREAK BETWEEN BYTES + TRANSFER EXIT
                        ------------------------------------------------------
                        if spi_status = "0001" -- Transfer Init
                        or spi_status = "0101" -- Break Between Bytes
                        or spi_status = "1001" -- Break Between Bytes
                        or spi_status = "1101" -- Transfer Exit
                        then
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                        end if;

                        ----------------------------
                        -- BYTE INIT + BYTE EXIT
                        ----------------------------
                        if spi_status = "0010" -- Transfer Init
                        or spi_status = "0100" -- First Byte Exit
                        or spi_status = "0110" -- Next Byte Init
                        or spi_status = "1000" -- Next Byte Exit
                        or spi_status = "1010" -- Last Byte Init
                        or spi_status = "1100" -- Last Byte Exit
                        then
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '0';
                            CTRL_SCK <= '0';
                        end if;

                        ---------------------------------------------------
                        -- CLOCK and DATA PROCESS
                        ---------------------------------------------------
                        if spi_status = "0011"
                        or spi_status = "0111"
                        or spi_status = "1011"
                        then
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
                                    CTRL_MOSI <= bytes_register(7 - index);
                                    index <= index + 1;
                                end if;
                            else
                                sck_timer <= sck_timer + '1';
                            end if;
                        end if;

                        ------------------------
                        -- Back to CONFIG
                        ------------------------
                        if spi_status = "1110" then
                            SPI_state <= SPI_CONFIG;
                        end if;

                        ------------------------
                        -- Back to CONFIG->IDLE
                        ------------------------
                        if spi_status = "1111" then
                            CTRL_CS <= '1';
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                            SPI_state <= SPI_CONFIG;
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: Increment Status Timer
------------------------------------------------------------------------------------------------------------------------------
                        byte_process_timer <= byte_process_timer + 1;
                    end if;

                when SPI_MUX =>
                    CTRL_CS <= '1';
                    FPGA_INT <= '0';
                    CTRL_MUX <= "0001";
                    FEEDBACK_DATA <= "10000001";
                    SPI_state <= SPI_DONE;

                when SPI_DONE =>
                    SPI_state <= SPI_IDLE;

                when others =>
                    SPI_state <= SPI_IDLE;
            end case;
        end if;
    end process;
end architecture rtl;
