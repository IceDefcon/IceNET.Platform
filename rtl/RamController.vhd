library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity RamController is
Port
(
    CLK_133MHz  : in  std_logic;
    CLK_800MHz  : in  std_logic;
    RESET       : in  std_logic;

    -- SDRAM Interface
    A           : out std_logic_vector(12 downto 0);
    BA          : out std_logic_vector(1 downto 0);
    CLK_SDRAM   : out std_logic;
    CKE         : out std_logic;
    CS          : out std_logic;
    RAS         : out std_logic;
    CAS         : out std_logic;
    WE          : out std_logic;
    DQ          : inout std_logic_vector(15 downto 0);
    LDQM        : out std_logic;
    UDQM        : out std_logic;

    -- User Interface
    ADDR        : in  std_logic_vector(23 downto 0);
    DATA_IN     : in  std_logic_vector(15 downto 0);
    DATA_OUT    : out std_logic_vector(15 downto 0);
    READ_EN     : in  std_logic;
    WRITE_EN    : in  std_logic;
    BUSY        : out std_logic
);
end RamController;

architecture rtl of RamController is

    type SDRAM_STATE is
    (
        SDRAM_INIT,
        SDRAM_PRECHARGE,
        SDRAM_AUTO_REFRESH,
        SDRAM_MODE,
        SDRAM_IDLE,
        SDRAM_ACTIVE,
        SDRAM_WRITE_CMD,
        SDRAM_READ_CMD,
        SDRAM_READ_DATA,
        SDRAM_REFRESH,
        SDRAM_DONE
    );
    signal memory_state : SDRAM_STATE := SDRAM_INIT;

    -- Internal signals
    signal command : std_logic_vector(4 downto 0);
    signal process_read : std_logic := '0';
    signal process_write : std_logic := '0';

    signal init_counter : integer range 0 to 13300 := 0;
    signal refresh_counter : integer range 0 to 8192 := 0;
    signal tRP_counter : integer range 0 to 16 := 0;
    signal tMRD_counter : integer range 0 to 16 := 0;
    signal tRRC_counter : integer range 0 to 16 := 0;
    signal tRCD_counter : integer range 0 to 16 := 0;
    signal tRAS_counter : integer range 0 to 16 := 0;
    signal tCL_counter : integer range 0 to 16 := 0;

    signal ar_counter : integer range 0 to 2 := 0;


    constant CMD_INIT       : std_logic_vector(4 downto 0) := "01111"; -- 0x0F CKE DOWN :: INIT
    constant CMD_NOP        : std_logic_vector(4 downto 0) := "10111"; -- 0x17 NOP
    constant CMD_ACTIVE     : std_logic_vector(4 downto 0) := "10011"; -- 0x13 BANK ACTIVE
    constant CMD_WRITE      : std_logic_vector(4 downto 0) := "10100"; -- 0x14 WRITE
    constant CMD_READ       : std_logic_vector(4 downto 0) := "10101"; -- 0x15 READ
    constant CMD_BSTOP      : std_logic_vector(4 downto 0) := "10110"; -- 0x16 BURST STOP
    constant CMD_PRECHARGE  : std_logic_vector(4 downto 0) := "10010"; -- 0x12 PRECHARGE
    constant CMD_AUTOREFRESH: std_logic_vector(4 downto 0) := "10001"; -- 0x11 AUTO REFRESH
    constant CMD_MODE       : std_logic_vector(4 downto 0) := "10000"; -- 0x10 MODE REGISTER SET

    constant MODE_REG : std_logic_vector(12 downto 0) := "00" & "00" & "00" & "010" & "0" & "000";

begin

    CLK_SDRAM <= CLK_133MHz;

    CKE <= command(4);
    CS  <= command(3);
    RAS <= command(2);
    CAS <= command(1);
    WE  <= command(0);

    LDQM <= '0';
    UDQM <= '0';

    process (CLK_133MHz, RESET)
    begin
        if RESET = '1' then

            init_counter <= 0;
            refresh_counter <= 0;
            process_read <= '0';
            process_write <= '0';
            DATA_OUT <= (others => '0');

            tRP_counter <= 3;  -- 20ns / (1/133MHz) = 2.660 cycles
            tMRD_counter <= 1; -- 2ns  / (1/133MHz) = 2.660 cycles
            tRRC_counter <= 9; -- 63ns / (1/133MHz) = 0.266 cycles
            tRCD_counter <= 3; -- 20ns / (1/133MHz) = 2.660 cycles
            tRAS_counter <= 3; -- 42ns / (1.133Mhz) = 5.586 cycles
            --tRC_counter <= 9;  -- 63ns / (1/133MHz) = 8.379 cycles ---> tRAS + tRP = 42ns + 20ns ~ 62ns
            tCL_counter <= 3;  -- For 133MHz Clock :: tCL × Clock Period = 3×7.5ns = 22.5ns

            ar_counter <= 2; -- Interrations of auto-refresh

            A <= (others => '0');
            BA <= (others => '0');

            command <= CMD_INIT;
            memory_state <= SDRAM_INIT;

        elsif rising_edge(CLK_133MHz) then
            case memory_state is

                when SDRAM_INIT =>
                    command <= CMD_NOP;
                    if init_counter = 13300 then
                        memory_state <= SDRAM_PRECHARGE;
                    else
                        init_counter <= init_counter + 1;
                    end if;

                when SDRAM_PRECHARGE =>
                    command <= CMD_PRECHARGE;
                    A <= (others => '1');
                    BA <= (others => '1');
                    memory_state <= SDRAM_AUTO_REFRESH;

                when SDRAM_AUTO_REFRESH =>
                    if tRP_counter = 0 then
                        command <= CMD_AUTOREFRESH;
                        memory_state <= SDRAM_MODE;
                        tRRC_counter <= 9;
                    else
                        command <= CMD_NOP;
                        tRP_counter <= tRP_counter - 1;
                    end if;

                when SDRAM_MODE =>
                    if tRRC_counter = 0 then
                        if ar_counter = 0 then
                            BA <= "00";
                            A <= MODE_REG;
                            command <= CMD_MODE;
                            memory_state <= SDRAM_IDLE;
                         else
                            command <= CMD_NOP;
                            memory_state <= SDRAM_AUTO_REFRESH;
                            ar_counter <= ar_counter - 1;
                        end if;
                    else
                        command <= CMD_NOP;
                        tRRC_counter <= tRRC_counter - 1;
                    end if;

                when SDRAM_IDLE =>
                    command <= CMD_NOP;
                    if tMRD_counter = 0 then

                        if WRITE_EN = '1' then
                            process_write <= '1';
                            memory_state <= SDRAM_ACTIVE;
                        elsif READ_EN = '1' then
                            process_read <= '1';
                            memory_state <= SDRAM_ACTIVE;
                        elsif refresh_counter = 8192 then
                            memory_state <= SDRAM_REFRESH;
                        end if;

                    else
                        tMRD_counter <= tMRD_counter - 1;
                    end if;

                when SDRAM_ACTIVE =>
                    command <= CMD_ACTIVE;
                    A <= ADDR(23 downto 11);
                    BA <= ADDR(10 downto 9);
                    if process_write = '1' then
                        memory_state <= SDRAM_WRITE_CMD;
                    elsif process_read = '1' then
                        memory_state <= SDRAM_READ_CMD;
                    end if;
                    tRCD_counter <= 2;

                when SDRAM_WRITE_CMD =>
                    if tRCD_counter = 0 then
                        command <= CMD_WRITE;
                        A(8 downto 0) <= ADDR(8 downto 0);
                        A(10) <= '0';
                        DQ <= DATA_IN;
                        memory_state <= SDRAM_DONE;
                    else
                        command <= CMD_NOP;
                        tRCD_counter <= tRCD_counter - 1;
                    end if;

                when SDRAM_READ_CMD =>
                    if tRCD_counter = 0 then
                        command <= CMD_READ;
                        A(8 downto 0) <= ADDR(8 downto 0);
                        A(10) <= '0';
                        tCL_counter <= 2;
                        DQ <= (others => 'Z');
                        memory_state <= SDRAM_READ_DATA;
                    else
                        command <= CMD_NOP;
                        tRCD_counter <= tRCD_counter - 1;
                    end if;

                when SDRAM_READ_DATA =>
                    command <= CMD_NOP;
                    if tCL_counter = 0 then
                        DATA_OUT <= DQ;
                        memory_state <= SDRAM_DONE;
                    else
                        tCL_counter <= tCL_counter - 1;
                    end if;

                when SDRAM_REFRESH =>
                    command <= CMD_AUTOREFRESH;
                    refresh_counter <= 0;
                    memory_state <= SDRAM_IDLE;

                when SDRAM_DONE =>
                    command <= CMD_NOP;
                    if tRAS_counter = 0 then
                        process_read <= '0';
                        process_write <= '0';
                        memory_state <= SDRAM_IDLE;
                    else
                        tRAS_counter <= tRAS_counter - 1;
                    end if;
            end case;

            refresh_counter <= refresh_counter + 1;
        end if;
    end process;

    BUSY <= '1' when memory_state /= SDRAM_IDLE else '0';
end rtl;
