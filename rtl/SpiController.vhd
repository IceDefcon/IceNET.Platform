library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SpiController is
generic
(
    SM_OFFSET : integer := 3;
    BYTE_INIT : integer := 10;
    BYTE_BREAK : integer := 50;
    BYTE_EXIT : integer := 10
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;
    -- IN
    OFFLOAD_TRIGGER : in std_logic;
    OFFLOAD_ID : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);
    -- SPI
    CTRL_CS : out std_logic;
    CTRL_MISO : in std_logic;
    CTRL_MOSI : out std_logic;
    CTRL_SCK : out std_logic;
    -- OUT
    SINGLE_COMPLETE : out std_logic;
    BURST_COMPLETE : out std_logic;
    BURST_DATA : out std_logic_vector(7 downto 0);
    SINGLE_DATA : out std_logic_vector(7 downto 0);
    OFFLOAD_WAIT : out std_logic
);
end entity SpiController;

architecture rtl of SpiController is

type SPI_CONTROLLER_TYPE is
(
    SPI_IDLE,
    SPI_INIT,
    SPI_CONFIG,
    SPI_READ_PROCESS,
    SPI_WRITE_PROCESS,
    SPI_DONE
);
signal SPI_state: SPI_CONTROLLER_TYPE := SPI_IDLE;

signal byte_process_timer : integer range 0 to 1024 := 0;

constant TRANSFER_INIT : integer range 0 to 512 := 500;
constant BYTE_CLOCK : integer range 0 to 128 := 80;
constant TRANSFER_EXIT : integer range 0 to 512 := 500;
constant WRITE_DELAY : integer range 0 to 5000000 := 5000000;

signal write_couter : integer range 0 to 5000000 := 0;

signal spi_status : std_logic_vector(3 downto 0) := "0000";
signal sck_timer : std_logic_vector(3 downto 0) := "0000";
signal sck_timer_toggle : std_logic := '0';
signal index : integer range 0 to 15 := 0;
signal write_flag : std_logic := '0';
signal write_data : std_logic_vector(7 downto 0) := (others => '0');
signal bytes_amount : integer range 0 to 15 := 0;
signal bytes_count : integer range 0 to 15 := 0;
signal register_address : std_logic_vector(7 downto 0) := (others => '0');
signal transfer_byte : std_logic_vector(7 downto 0) := (others => '0');

signal first_byte : std_logic := '0';
signal next_byte : std_logic := '0';
signal last_byte : std_logic := '0';

signal feedback_byte : std_logic_vector(7 downto 0) := (others => '0');

signal sm_flag : std_logic := '0';

begin

    spiControl_process:
    process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            SPI_state <= SPI_IDLE;
            byte_process_timer <= 0;
            write_couter <= 0;
            spi_status <= (others => '0');
            sck_timer <= (others => '0');
            sck_timer_toggle <= '0';
            index <= 0;
            write_flag <= '0';
            write_data <= (others => '0');
            bytes_amount <= 0;
            bytes_count <= 0;
            register_address <= (others => '0');
            transfer_byte <= (others => '0');
            first_byte <= '0';
            next_byte <= '0';
            last_byte <= '0';
            feedback_byte <= (others => '0');
            OFFLOAD_WAIT <= '0';
            CTRL_CS <= '0';
            CTRL_MOSI <= '0';
            CTRL_SCK <= '0';
            SINGLE_COMPLETE <= '0';
            SINGLE_DATA <= (others => '0');
        elsif rising_edge(CLOCK_50MHz) then

            if OFFLOAD_TRIGGER = '1' then
                OFFLOAD_WAIT <= '1';
                SPI_state <= SPI_INIT;
            end if;

            case SPI_state is

                when SPI_IDLE =>
                    CTRL_CS <= '1';
                    CTRL_MOSI <= '1';
                    CTRL_SCK <= '0';

                when SPI_INIT =>
                    bytes_count <= 0;
                    spi_status <= "0000";
                    SPI_state <= SPI_CONFIG;
                    ----------------------------------------------------------------------
                    -- Parameters that are controlled from kernel
                    ----------------------------------------------------------------------
                    write_flag <= OFFLOAD_CONTROL(0);
                    write_data <= OFFLOAD_DATA;
                    bytes_amount <= 1 + to_integer(unsigned(OFFLOAD_CONTROL(6 downto 3)));
                    register_address <= OFFLOAD_REGISTER;

                when SPI_CONFIG =>
                    ------------------------------
                    -- Reset Process Variables
                    ------------------------------
                    --
                    -- ??? TODO ???
                    --
                    -- One clock cycle data loss
                    -- does not realy mater
                    -- due the existance
                    -- on rising edge
                    --
                    sck_timer <= "0100";
                    sck_timer_toggle <= '1';
                    spi_status <= "0000";
                    ------------------------------
                    -- Finished :: Jump to MUX
                    ------------------------------
                    if bytes_count = bytes_amount then
                        if write_flag = '1' then
                            if write_couter = WRITE_DELAY then
                                write_couter <= 0;
                                write_flag <= '0';
                                SPI_state <= SPI_DONE;
                            else
                                write_couter <= write_couter + 1;
                            end if;
                        else
                            SPI_state <= SPI_DONE;
                        end if;
                    else
                        --------------------------------
                        -- Set flags :: Jump to PROCESS
                        --------------------------------
                        if bytes_count = 0 then
                            first_byte <= '1';
                            next_byte <= '0';
                            last_byte <= '0';
                            transfer_byte <= register_address;
                        end if;

                        if bytes_count > 0 and bytes_count < (bytes_amount - 1) then
                            first_byte <= '0';
                            next_byte <= '1';
                            last_byte <= '0';
                            transfer_byte <= (others => '0'); -- Only this one is required
                        end if;

                        if bytes_count = bytes_amount - 1 then
                            first_byte <= '0';
                            next_byte <= '0';
                            last_byte <= '1';
                            if write_flag = '1' then
                                transfer_byte <= write_data;
                            else
                                transfer_byte <= (others => '0'); -- Burst length is already set
                            end if;
                        end if;

                        index <= 0;
                        bytes_count <= bytes_count + 1;
                        byte_process_timer <= 0;
                        if write_flag = '1' then
                            SPI_state <= SPI_WRITE_PROCESS;
                        else
                            SPI_state <= SPI_READ_PROCESS;
                        end if;
                    end if;

                when SPI_READ_PROCESS =>
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
                            if byte_process_timer < BYTE_BREAK - SM_OFFSET then
                                spi_status <= "0101"; -- Break Between Bytes
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT then
                                spi_status <= "0110"; -- Next Byte Init
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK then
                                spi_status <= "0111"; -- Generic Byte Clock Processing
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                                spi_status <= "1000"; -- Next Byte Exit
                            else
                                spi_status <= "1110"; -- Going Back to CONFIG
                                BURST_COMPLETE <= '1';
                                BURST_DATA <= feedback_byte;
                            end if;
                        end if;

                        if last_byte = '1' then
                            if byte_process_timer < BYTE_BREAK - SM_OFFSET then
                                spi_status <= "1001"; -- Break Between Bytes
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT then
                                spi_status <= "1010"; -- Last Byte Init
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK then
                                spi_status <= "1011"; -- Generic Byte Clock Process
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                                spi_status <= "1100"; -- Last Byte Exit
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT + TRANSFER_EXIT then
                                spi_status <= "1101"; -- Transfer Exit
                                SINGLE_COMPLETE <= '1';
                                BURST_COMPLETE <= '1';
                                SINGLE_DATA <= feedback_byte;
                                BURST_DATA <= feedback_byte;
                            else
                                spi_status <= "1111"; -- Going Back to CONFIG -> IDLE
                            end if;
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: SPI Status
------------------------------------------------------------------------------------------------------------------------------

                        ------------------------------------------------------
                        -- TRANSFER INIT + BREAK BETWEEN BYTES + TRANSFER EXIT
                        ------------------------------------------------------
                        if spi_status = "0101" -- Break Between Bytes
                        or spi_status = "1001" -- Break Between Bytes
                        then
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                        end if;

                        ----------------------------
                        -- BYTE INIT + BYTE EXIT
                        ----------------------------
                        if spi_status = "0001" -- Transfer Init
                        or spi_status = "0010" -- First Byte Init
                        or spi_status = "0100" -- First Byte Exit
                        or spi_status = "0110" -- Next Byte Init
                        or spi_status = "1000" -- Next Byte Exit
                        or spi_status = "1010" -- Last Byte Init
                        or spi_status = "1100" -- Last Byte Exit
                        or spi_status = "1101" -- Transfer Exit
                        then
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '0';
                            CTRL_SCK <= '0';
                        end if;

                        if spi_status = "1101" then
                            BURST_COMPLETE <= '0';
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
                                    -----------------------------------------
                                    -- SM Offset taken into account
                                    -----------------------------------------
                                    if sm_flag = '1' then
                                        sck_timer <= std_logic_vector(to_unsigned(SM_OFFSET, 4));
                                        sm_flag <= '0';
                                    else
                                        sck_timer <= (others => '0');
                                    end if;
                                    -----------------------------------------
                                    -- CLOCK [0]
                                    -----------------------------------------
                                    CTRL_SCK <= '0';
                                    -----------------------------------------
                                    -- DATA @ Rising Edge of the clock !
                                    -----------------------------------------
                                    CTRL_MOSI <= transfer_byte(7 - index);
                                    feedback_byte(8 - index) <= CTRL_MISO;
                                    index <= index + 1;
                                else
                                    sck_timer <= (others => '0');
                                    -----------------------------------------
                                    -- CLOCK [1]
                                    -----------------------------------------
                                    CTRL_SCK <= '1';
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
                            sm_flag <= '1';
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '0';
                            CTRL_SCK <= '0';
                            BURST_COMPLETE <= '0';
                        end if;

                        ------------------------
                        -- Back to CONFIG->IDLE
                        ------------------------
                        if spi_status = "1111" then
                            CTRL_CS <= '1';
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                            SPI_state <= SPI_CONFIG;
                            BURST_COMPLETE <= '0';
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: Increment Status Timer
------------------------------------------------------------------------------------------------------------------------------
                        byte_process_timer <= byte_process_timer + 1;
                    end if;

                when SPI_WRITE_PROCESS =>
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
                            if byte_process_timer < BYTE_BREAK - SM_OFFSET then
                                spi_status <= "0101"; -- Break Between Bytes
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT then
                                spi_status <= "0110"; -- Next Byte Init
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK then
                                spi_status <= "0111"; -- Generic Byte Clock Processing
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                                spi_status <= "1000"; -- Next Byte Exit
                            else
                                spi_status <= "1110"; -- Going Back to CONFIG
                            end if;
                        end if;

                        if last_byte = '1' then
                            if byte_process_timer < BYTE_BREAK - SM_OFFSET then
                                spi_status <= "1001"; -- Break Between Bytes
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT then
                                spi_status <= "1010"; -- Last Byte Init
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK then
                                spi_status <= "1011"; -- Generic Byte Clock Process
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT then
                                spi_status <= "1100"; -- Last Byte Exit
                            elsif byte_process_timer < BYTE_BREAK - SM_OFFSET + BYTE_INIT + BYTE_CLOCK + BYTE_EXIT + TRANSFER_EXIT then
                                spi_status <= "1101"; -- Transfer Exit
                                SINGLE_COMPLETE <= '1';
                                SINGLE_DATA <= OFFLOAD_REGISTER;
                            else
                                spi_status <= "1111"; -- Going Back to CONFIG -> IDLE
                            end if;
                        end if;

------------------------------------------------------------------------------------------------------------------------------
-- PIPE[1] :: SPI Status
------------------------------------------------------------------------------------------------------------------------------

                        ------------------------------------------------------
                        -- TRANSFER INIT + BREAK BETWEEN BYTES + TRANSFER EXIT
                        ------------------------------------------------------
                        if spi_status = "0101" -- Break Between Bytes
                        or spi_status = "1001" -- Break Between Bytes
                        then
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '1';
                            CTRL_SCK <= '0';
                        end if;

                        ----------------------------
                        -- BYTE INIT + BYTE EXIT
                        ----------------------------
                        if spi_status = "0001" -- Transfer Init
                        or spi_status = "0010" -- First Byte Init
                        or spi_status = "0100" -- First Byte Exit
                        or spi_status = "0110" -- Next Byte Init
                        or spi_status = "1000" -- Next Byte Exit
                        or spi_status = "1010" -- Last Byte Init
                        or spi_status = "1100" -- Last Byte Exit
                        or spi_status = "1101" -- Transfer Exit
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
                                    -----------------------------------------
                                    -- SM Offset taken into account
                                    -----------------------------------------
                                    if sm_flag = '1' then
                                        sck_timer <= std_logic_vector(to_unsigned(SM_OFFSET, 4));
                                        sm_flag <= '0';
                                    else
                                        sck_timer <= (others => '0');
                                    end if;
                                    -----------------------------------------
                                    -- CLOCK [0]
                                    -----------------------------------------
                                    CTRL_SCK <= '0';
                                    -----------------------------------------
                                    -- DATA @ Rising Edge of the clock !
                                    -----------------------------------------
                                    CTRL_MOSI <= transfer_byte(7 - index);
                                    index <= index + 1;
                                else
                                    sck_timer <= (others => '0');
                                    -----------------------------------------
                                    -- CLOCK [1]
                                    -----------------------------------------
                                    CTRL_SCK <= '1';
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
                            sm_flag <= '1';
                            CTRL_CS <= '0';
                            CTRL_MOSI <= '0';
                            CTRL_SCK <= '0';
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

                when SPI_DONE =>
                    CTRL_CS <= '1';
                    SINGLE_COMPLETE <= '0';
                    OFFLOAD_WAIT <= '0';
                    SPI_state <= SPI_IDLE;

                when others =>
                    SPI_state <= SPI_IDLE;
            end case;
        end if;
    end process;
end architecture rtl;
