library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity SDRAM_Controller is
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
end SDRAM_Controller;

architecture Behavioral of SDRAM_Controller is

    -- State definitions
    type SDRAM_STATE is
    (
        SDRAM_INIT,
        SDRAM_IDLE,
        SDRAM_ACTIVE,
        SDRAM_RAS_TO_CAS_DELAY,
        SDRAM_WRITE,
        SDRAM_READ,
        SDRAM_GRAB,
        SDRAM_PRECHARGE,
        SDRAM_REFRESH
    );
    signal memory_state : SDRAM_STATE;

    -- Internal signals
    signal init_counter : integer range 0 to 100 := 0;
    signal refresh_counter : integer range 0 to 8192 := 0;
    signal command : std_logic_vector(3 downto 0); -- Command signal

    signal process_read : std_logic := '0';
    signal process_write : std_logic := '0';

    signal delay_ras_to_cas : std_logic_vector(3 downto 0);
    constant two_jump_offset : std_logic_vector(3 downto 0) := "0010";

        -- Command definitions
    constant CMD_NOP        : std_logic_vector(3 downto 0) := "0111";
    constant CMD_ACTIVE     : std_logic_vector(3 downto 0) := "0011";
    constant CMD_READ       : std_logic_vector(3 downto 0) := "0101";
    constant CMD_WRITE      : std_logic_vector(3 downto 0) := "0100";
    constant CMD_PRECHARGE  : std_logic_vector(3 downto 0) := "0010";
    constant CMD_REFRESH    : std_logic_vector(3 downto 0) := "0001";
    constant CMD_LOAD_MODE  : std_logic_vector(3 downto 0) := "0000";

begin

    -- Assign SDRAM signals
    CLK_SDRAM <= CLK_200MHz;
    CKE       <= '1';
    CS        <= command(3);
    RAS       <= command(2);
    CAS       <= command(1);
    WE        <= command(0);
    LDQM      <= '0'; -- Assume full-width access for simplicity
    UDQM      <= '0';

    process (CLK_200MHz, RESET, memory_state, READ_EN, WRITE_EN, init_counter, refresh_counter)
    begin

        if RESET = '1' then
            init_counter <= 0;
            refresh_counter <= 0;
            command <= CMD_NOP;
        elsif rising_edge(CLK_200MHz) then

            case memory_state is

                when SDRAM_INIT =>
                    command <= CMD_NOP;
                    if init_counter = 100 then
                        memory_state <= SDRAM_IDLE;
                    else
                        init_counter <= init_counter + 1;
                    end if;

                when SDRAM_IDLE =>
                    command <= CMD_NOP;
                    if WRITE_EN = '1' then
                        process_read <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif READ_EN = '1' then
                        process_write <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif refresh_counter = 8192 then
                        memory_state <= SDRAM_REFRESH;
                    end if;

                when SDRAM_ACTIVE =>
                    command <= CMD_ACTIVE;
                    A <= ADDR(23 downto 11); -- Row address
                    BA <= ADDR(10 downto 9); -- Bank address

                when SDRAM_RAS_TO_CAS_DELAY =>
                    if delay_ras_to_cas = "0100" - two_jump_offset then
                        delay_ras_to_cas <= (others => '0');
                        if WRITE_EN = '1' then
                            DQ <= DATA_IN;
                            memory_state <= SDRAM_WRITE;
                        elsif READ_EN = '1' then
                            DQ <= (others => 'Z');
                            memory_state <= SDRAM_READ;
                        end if;
                    else
                        delay_ras_to_cas <= delay_ras_to_cas + '1';
                    end if;

                when SDRAM_WRITE =>
                    command <= CMD_WRITE;
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address
                    memory_state <= SDRAM_PRECHARGE;

                when SDRAM_READ =>
                    command <= CMD_READ;
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address
                    DATA_OUT <= DQ;
                    memory_state <= SDRAM_PRECHARGE;

                when SDRAM_GRAB =>
                    memory_state <= SDRAM_PRECHARGE;

                when SDRAM_PRECHARGE =>
                    process_read <= '0';
                    process_write <= '0';
                    command <= CMD_PRECHARGE;
                    memory_state <= SDRAM_IDLE;

                when SDRAM_REFRESH =>
                    command <= CMD_REFRESH;
                    refresh_counter <= 0;
                    memory_state <= SDRAM_IDLE;

            end case;

            -- Refresh counter
            if memory_state /= SDRAM_REFRESH then
                refresh_counter <= refresh_counter + 1;
            end if;

        end if;
    end process;

    -- BUSY signal
    BUSY <= '1' when memory_state /= SDRAM_IDLE else '0';

end Behavioral;
