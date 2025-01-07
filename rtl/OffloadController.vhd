library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity OffloadController is
port
(    
    CLOCK_50MHz : in std_logic;

    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(6 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : in std_logic
);
end OffloadController;

architecture rtl of OffloadController is

type STATE is 
(
    IDLE,
    -- HEADER
    HEADER_INIT,
    HEADER_DELAY,
    HEADER_1,
    HEADER_2,
    HEADER_3,
    HEADER_4,
    HEADER_CONFIG,
    -- DEVICE
    DEVICE_INIT,
    DEVICE_DELAY,
    DEVICE_1,
    DEVICE_2,
    DEVICE_3,
    DEVICE_4,
    DEVICE_CONFIG,
    -- TRANSFER
    TRANSFER_INIT,
    TRANSFER_DELAY,
    TRANSFER_1,
    TRANSFER_2,
    TRANSFER_READY,
    TRANSFER_WAIT_1,
    TRANSFER_WAIT_2,
    TRANSFER_EXECUTE,
    -- FINAL CHECKSUM
    CHECKSUM
);
signal offload_state: STATE := IDLE;

signal header_size : integer := 0;
signal header_devices : integer := 0;
signal header_scramble : std_logic_vector(7 downto 0) := (others => '0');
signal header_checksum : std_logic_vector(7 downto 0) := (others => '0');

signal config_size : integer := 0;
signal config_devices : integer := 0;
signal config_scramble : std_logic_vector(7 downto 0) := (others => '0');
signal config_checksum : std_logic_vector(7 downto 0) := (others => '0');

signal device_size : integer := 0;
signal device_ctrl : std_logic_vector(7 downto 0) := (others => '0');
signal device_id : std_logic_vector(6 downto 0) := (others => '0');
signal device_pairs : integer := 0;
signal device_register : std_logic_vector(7 downto 0) := (others => '0');
signal device_data : std_logic_vector(7 downto 0) := (others => '0');
signal device_checksum : std_logic_vector(7 downto 0) := (others => '0');

signal transfer_pairs : integer := 0;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin

offload_process:
process (CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case offload_state is

            when IDLE =>
                OFFLOAD_READY <= '0';
                FIFO_READ_ENABLE <= '0';
                if OFFLOAD_INTERRUPT = '1' then
                    offload_state <= HEADER_INIT;
                else
                    offload_state <= IDLE;
                end if;

            when HEADER_INIT =>
                FIFO_READ_ENABLE <= '1';
                offload_state <= HEADER_DELAY;

            when HEADER_DELAY =>
                FIFO_READ_ENABLE <= '1';
                offload_state <= HEADER_1;

            ----------------------------------------------------------
            --
            -- HEADER
            --
            ----------------------------------------------------------
            --
            -- BYTE 0 :: Header Size
            -- BYTE 1 :: Device Amount
            -- BYTE 2 :: Scrambling byte
            -- BYTE 3 :: Checksum
            --
            ----------------------------------------------------------
            when HEADER_1 =>
                FIFO_READ_ENABLE <= '1';
                header_size <= to_integer(unsigned(FIFO_DATA));
                offload_state <= HEADER_2;

            when HEADER_2 =>
                FIFO_READ_ENABLE <= '1';
                header_devices <= to_integer(unsigned(FIFO_DATA));
                offload_state <= HEADER_3;

            when HEADER_3 =>
                FIFO_READ_ENABLE <= '0';
                header_scramble <= FIFO_DATA;
                offload_state <= HEADER_4;

            when HEADER_4 =>
                FIFO_READ_ENABLE <= '0';
                header_checksum <= FIFO_DATA;
                offload_state <= HEADER_CONFIG;

            when HEADER_CONFIG =>
                FIFO_READ_ENABLE <= '0';
                config_size <= header_size;
                config_devices <= header_devices;
                config_scramble <= header_scramble;
                config_checksum <= header_checksum;
                offload_state <= DEVICE_INIT;

            when DEVICE_INIT =>
                if config_devices > 0 then
                    FIFO_READ_ENABLE <= '1';
                    config_devices <= config_devices - 1;
                    offload_state <= DEVICE_DELAY;
                else
                    FIFO_READ_ENABLE <= '0';
                    offload_state <= IDLE;
                end if;

            when DEVICE_DELAY =>
                FIFO_READ_ENABLE <= '1';
                offload_state <= DEVICE_1;

            ----------------------------------------------------------
            --
            -- DEVICE CONFIG
            --
            ----------------------------------------------------------
            --
            -- DEVICE 0 :: Device Config Size
            -- DEVICE 1 :: Device Ctrl :: 0x11 ---> I2C, Write
            -- DEVICE 2 :: Device ID :: For I2C
            -- DEVICE 3 :: Device config pairs
            --
            ----------------------------------------------------------
            when DEVICE_1 =>
                device_size <= to_integer(unsigned(FIFO_DATA));
                FIFO_READ_ENABLE <= '1';
                offload_state <= DEVICE_2;

            when DEVICE_2 =>
                device_ctrl <= FIFO_DATA;
                FIFO_READ_ENABLE <= '1';
                offload_state <= DEVICE_3;

            when DEVICE_3 =>
                device_id <= FIFO_DATA(0) & FIFO_DATA(1) 
                & FIFO_DATA(2) & FIFO_DATA(3) 
                & FIFO_DATA(4) & FIFO_DATA(5) 
                & FIFO_DATA(6); -- Device ID :: Reverse concatenation
                FIFO_READ_ENABLE <= '0';
                offload_state <= DEVICE_4;

            when DEVICE_4 =>
                FIFO_READ_ENABLE <= '0';
                device_pairs <= to_integer(unsigned(FIFO_DATA));
                offload_state <= DEVICE_CONFIG;

            when DEVICE_CONFIG =>
                FIFO_READ_ENABLE <= '0';
                OFFLOAD_ID <= device_id;
                OFFLOAD_CTRL <= device_ctrl;
                transfer_pairs <= device_pairs;
                offload_state <= TRANSFER_INIT;

            ----------------------------------------------------------
            --
            -- REGISTER TRANSFER
            --
            ----------------------------------------------------------
            --
            -- TRANSFER_1       :: I2C or SPI Register
            -- TRANSFER_2       :: Data in case of write
            -- TRANSFER_READY   :: Transfer Offload ready
            -- TRANSFER_WAIT_1  :: Need to wait for 2 clk in order to take WAIT flag into account
            -- TRANSFER_WAIT_2  :: Need to wait for 2 clk in order to take WAIT flag into account
            -- TRANSFER_EXECUTE :: The actual Data Transfer
            --
            ----------------------------------------------------------
            when TRANSFER_INIT =>
                OFFLOAD_READY <= '0';
                if transfer_pairs > 0 then
                    FIFO_READ_ENABLE <= '1';
                    offload_state <= TRANSFER_DELAY;
                else
                    FIFO_READ_ENABLE <= '1';
                    offload_state <= CHECKSUM;
                end if;

            when TRANSFER_DELAY =>
                FIFO_READ_ENABLE <= '1';
                transfer_pairs <= transfer_pairs - 1;
                offload_state <= TRANSFER_1;

            when TRANSFER_1 =>
                FIFO_READ_ENABLE <= '0';
                OFFLOAD_REGISTER <= FIFO_DATA;
                offload_state <= TRANSFER_2;

            when TRANSFER_2 =>
                FIFO_READ_ENABLE <= '0';
                OFFLOAD_DATA <= FIFO_DATA;
                offload_state <= TRANSFER_READY;

            when TRANSFER_READY =>
                OFFLOAD_READY <= '1';
                offload_state <= TRANSFER_WAIT_1;

            when TRANSFER_WAIT_1 =>
                OFFLOAD_READY <= '0';
                offload_state <= TRANSFER_WAIT_2;

            when TRANSFER_WAIT_2 =>
                OFFLOAD_READY <= '0';
                offload_state <= TRANSFER_EXECUTE;

            when TRANSFER_EXECUTE =>
                FIFO_READ_ENABLE <= '0';
                if OFFLOAD_WAIT = '0' then
                    offload_state <= TRANSFER_INIT;
                else
                    offload_state <= TRANSFER_EXECUTE;
                end if;

            ----------------------------------------------------------
            --
            -- FINAL CHECKSUM
            --
            ----------------------------------------------------------
            when CHECKSUM =>
                OFFLOAD_READY <= '0';
                FIFO_READ_ENABLE <= '0';
                offload_state <= DEVICE_INIT;

            when others =>
                FIFO_READ_ENABLE <= '0';
                offload_state <= IDLE;

        end case;
    end if;
end process;

end rtl;
