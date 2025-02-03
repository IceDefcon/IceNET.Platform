library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

--
-- SDRAM Controller
-- HY57V2562GTR
-- Chip
--

entity RamController is
Port
(
    CLK_200MHz  : in  std_logic;
    CLK_100MHz  : in  std_logic;
    CLK_50Mhz   : in  std_logic;
    RESET       : in  std_logic;

    -- SDRAM Interface
    A           : out std_logic_vector(12 downto 0);  -- Address Bus
    BA          : out std_logic_vector(1 downto 0);   -- Bank Address
    CLK_SDRAM   : out std_logic;
    CKE         : out std_logic;
    CS          : out std_logic;
    RAS         : out std_logic;
    CAS         : out std_logic;
    WE          : out std_logic;
    DQ          : inout std_logic_vector(15 downto 0); -- Data Bus
    LDQM        : out std_logic;
    UDQM        : out std_logic;

    -- User Interface
    ADDR        : in  std_logic_vector(23 downto 0);
    DATA_IN     : in  std_logic_vector(15 downto 0);
    DATA_OUT    : out std_logic_vector(15 downto 0);
    READ_EN     : in  std_logic;
    WRITE_EN    : in  std_logic;
    BUSY       : out std_logic
);
end RamController;

architecture rtl of RamController is

    type SDRAM_STATE is (
        SDRAM_INIT,
        SDRAM_MODE,
        SDRAM_IDLE,
        SDRAM_ACTIVE,
        SDRAM_WRITE_CMD,
        SDRAM_READ_CMD,
        SDRAM_READ_DATA,
        SDRAM_REFRESH,
        SDRAM_DONE
    );
    signal memory_state : SDRAM_STATE;

    -- Internal signals
    signal init_counter : integer range 0 to 10000 := 0; -- Increased delay
    signal refresh_counter : integer range 0 to 8192 := 0;
    signal command : std_logic_vector(4 downto 0);
    signal process_read : std_logic := '0';
    signal process_write : std_logic := '0';

    signal tRCD_counter : integer range 0 to 8 := 0; -- Row Address to Column Address Delay
    signal tRAS_counter : integer range 0 to 8 := 0; -- Row Address to Column Address Delay
    signal tCL_counter : integer range 0 to 8 := 0; -- Clock cycles to return data after a READ command is issued

    -- Command definitions
    constant CMD_INIT       : std_logic_vector(4 downto 0) := "01111";
    constant CMD_MODE       : std_logic_vector(4 downto 0) := "10000";
    constant CMD_NOP        : std_logic_vector(4 downto 0) := "10111";
    constant CMD_ACTIVE     : std_logic_vector(4 downto 0) := "10011";
    constant CMD_WRITE      : std_logic_vector(4 downto 0) := "10100";
    constant CMD_READ       : std_logic_vector(4 downto 0) := "10101";
    constant CMD_BURST_STOP : std_logic_vector(4 downto 0) := "10110";
    constant CMD_PRECHARGE  : std_logic_vector(4 downto 0) := "10010";
    constant CMD_REFRESH    : std_logic_vector(4 downto 0) := "10001";

    -- SDRAM Mode Register Configuration
    constant MODE_REG : std_logic_vector(12 downto 0) := "000" & "0" & "00" & "010" & "0" & "000"; -- CAS Latency 3

begin
    CLK_SDRAM <= CLK_100MHz;
    CKE <= command(4);
    CS <= command(3);
    RAS <= command(2);
    CAS <= command(1);
    WE <= command(0);
    LDQM <= '0'; -- Enbled :: LowActive ENABLE or DISABLE 0:7  bits
    UDQM <= '0'; -- Enbled :: LowActive ENABLE or DISABLE 8:15 bits

    process (CLK_100MHz, RESET)
    begin
        if RESET = '1' then
            memory_state <= SDRAM_INIT;
            init_counter <= 0;
            refresh_counter <= 0;
            process_read <= '0';
            process_write <= '0';
            DATA_OUT <= (others => '0');
        elsif rising_edge(CLK_100MHz) then

            case memory_state is
                when SDRAM_INIT =>
                    command <= CMD_INIT;
                    if init_counter = 10000 then
                        memory_state <= SDRAM_MODE;
                    else
                        init_counter <= init_counter + 1;
                    end if;

                when SDRAM_MODE =>
                    BA <= "00";
                    A <= MODE_REG;
                    command <= CMD_MODE;
                    memory_state <= SDRAM_IDLE;

                when SDRAM_IDLE =>
                    command <= CMD_ACTIVE;
                    if WRITE_EN = '1' then
                        process_write <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif READ_EN = '1' then
                        process_read <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif refresh_counter = 8192 then
                        memory_state <= SDRAM_REFRESH;
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
                    tRCD_counter <= 8;

                when SDRAM_WRITE_CMD =>
                    if tRCD_counter = 0 then
                        command <= CMD_WRITE;
                        DQ <= DATA_IN;
                        A(8 downto 0) <= ADDR(8 downto 0);
                        A(10) <= '1';
                        --tRAS_counter <= 3;
                        memory_state <= SDRAM_DONE;
                    else
                        command <= CMD_NOP;
                        tRCD_counter <= tRCD_counter - 1;
                    end if;

                when SDRAM_READ_CMD =>
                    if tRCD_counter = 0 then
                        command <= CMD_READ;
                        DQ <= (others => 'Z');
                        A(8 downto 0) <= ADDR(8 downto 0);
                        A(10) <= '1';
                        --tRAS_counter <= 3;
                        memory_state <= SDRAM_READ_DATA;
                    else
                        command <= CMD_NOP;
                        tRCD_counter <= tRCD_counter - 1;
                    end if;

                when SDRAM_READ_DATA =>
                    command <= CMD_NOP;
                    if tCL_counter = 0 then
                        DATA_OUT <= (others => '0');
                        --DATA_OUT <= DQ;
                        memory_state <= SDRAM_DONE;
                    else
                        tCL_counter <= tCL_counter - 1;
                    end if;

                when SDRAM_REFRESH =>
                    command <= CMD_REFRESH;
                    refresh_counter <= 0;
                    memory_state <= SDRAM_IDLE;

                when SDRAM_DONE =>
                    command <= CMD_NOP;
                    process_read <= '0';
                    process_write <= '0';
                    memory_state <= SDRAM_IDLE;

            end case;

            if memory_state /= SDRAM_REFRESH then
                refresh_counter <= refresh_counter + 1;
            end if;
        end if;
    end process;

    BUSY <= '1' when memory_state /= SDRAM_IDLE else '0';

end rtl;
