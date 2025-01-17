library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity SDRAM_Controller is
Port
(
    CLK         : in  std_logic;
    CLK_SLOW    : in  std_logic;
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
    READY       : out std_logic
);
end SDRAM_Controller;

architecture Behavioral of SDRAM_Controller is

    -- State definitions
    type state_type is
    (
        SDRAM_INIT,
        SDRAM_IDLE,
        SDRAM_ACTIVE,
        SDRAM_READ,
        SDRAM_WRITE,
        SDRAM_PRECHARGE,
        SDRAM_REFRESH
    );
    signal current_state, next_state : state_type;

    -- Internal signals
    signal init_counter : integer range 0 to 100 := 0;
    signal refresh_counter : integer range 0 to 8192 := 0;
    signal command : std_logic_vector(4 downto 0); -- Command signal

    -- Command definitions
    constant CMD_NOP        : std_logic_vector(4 downto 0) := "10111";
    constant CMD_ACTIVE     : std_logic_vector(4 downto 0) := "10011";
    constant CMD_READ       : std_logic_vector(4 downto 0) := "10101";
    constant CMD_WRITE      : std_logic_vector(4 downto 0) := "10100";
    constant CMD_PRECHARGE  : std_logic_vector(4 downto 0) := "10010";
    constant CMD_REFRESH    : std_logic_vector(4 downto 0) := "10001";
    constant CMD_LOAD_MODE  : std_logic_vector(4 downto 0) := "10000";

begin

    -- Assign SDRAM signals
    CLK_SDRAM <= CLK;
    CKE       <= '1';
    CS        <= command(4);
    RAS       <= command(3);
    CAS       <= command(2);
    WE        <= command(1);
    LDQM      <= '0'; -- Assume full-width access for simplicity
    UDQM      <= '0';

    -- State transition process
    process (CLK, RESET)
    begin
        if RESET = '1' then
            current_state <= SDRAM_INIT;
        elsif rising_edge(CLK) then
            current_state <= next_state;
        end if;
    end process;

    -- Next state logic
    process (current_state, READ_EN, WRITE_EN, init_counter, refresh_counter)
    begin
        next_state <= current_state; -- Default is to stay in the current state
        case current_state is
            when SDRAM_INIT =>
                if init_counter = 100 then
                    next_state <= SDRAM_IDLE;
                end if;

            when SDRAM_IDLE =>
                if WRITE_EN = '1' then
                    next_state <= SDRAM_ACTIVE;
                elsif READ_EN = '1' then
                    next_state <= SDRAM_ACTIVE;
                elsif refresh_counter = 8192 then
                    next_state <= SDRAM_REFRESH;
                end if;

            when SDRAM_ACTIVE =>
                if WRITE_EN = '1' then
                    next_state <= SDRAM_WRITE;
                elsif READ_EN = '1' then
                    next_state <= SDRAM_READ;
                end if;

            when SDRAM_WRITE =>
                next_state <= SDRAM_PRECHARGE;

            when SDRAM_READ =>
                next_state <= SDRAM_PRECHARGE;

            when SDRAM_PRECHARGE =>
                next_state <= SDRAM_IDLE;

            when SDRAM_REFRESH =>
                next_state <= SDRAM_IDLE;
        end case;
    end process;

    -- Output logic and counters
    process (CLK, RESET)
    begin
        if RESET = '1' then
            init_counter <= 0;
            refresh_counter <= 0;
            command <= CMD_NOP;
        elsif rising_edge(CLK) then
            case current_state is
                when SDRAM_INIT =>
                    command <= CMD_NOP;
                    if init_counter < 100 then
                        init_counter <= init_counter + 1;
                    end if;

                when SDRAM_IDLE =>
                    command <= CMD_NOP;

                when SDRAM_ACTIVE =>
                    command <= CMD_ACTIVE;
                    A <= ADDR(23 downto 11); -- Row address
                    BA <= ADDR(10 downto 9); -- Bank address

                when SDRAM_WRITE =>
                    command <= CMD_WRITE;
                    DQ <= DATA_IN;
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address

                when SDRAM_READ =>
                    command <= CMD_READ;
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address
                    DATA_OUT <= DQ;

                when SDRAM_PRECHARGE =>
                    command <= CMD_PRECHARGE;

                when SDRAM_REFRESH =>
                    command <= CMD_REFRESH;
                    refresh_counter <= 0;

            end case;

            -- Refresh counter
            if current_state /= SDRAM_REFRESH then
                refresh_counter <= refresh_counter + 1;
            end if;
        end if;
    end process;

    -- Ready signal
    READY <= '1' when current_state = SDRAM_IDLE else '0';

end Behavioral;
