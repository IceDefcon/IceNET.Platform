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

architecture rtl of SDRAM_Controller is

    -- State definitions
    type SDRAM_STATE is
    (
        SDRAM_INIT,
        SDRAM_MODE,
        SDRAM_MODE_DELAY,
        SDRAM_IDLE,
        SDRAM_ACTIVE,
        SDRAM_RAS_TO_CAS_DELAY,
        SDRAM_WRITE_CMD,
        SDRAM_READ_CMD,
        SDRAM_OP_DELAY,
        SDRAM_READ_DATA,
        SDRAM_DONE,
        SDRAM_REFRESH
    );
    signal memory_state : SDRAM_STATE;

    -- Internal signals
    signal init_counter : integer range 0 to 400 := 0;
    signal refresh_counter : integer range 0 to 8192 := 0;
    signal command : std_logic_vector(3 downto 0); -- Command signal

    signal process_read : std_logic := '0';
    signal process_write : std_logic := '0';

    signal delay_op : std_logic_vector(4 downto 0);
    signal delay_mode : std_logic_vector(4 downto 0);
    signal delay_ras_to_cas : std_logic_vector(4 downto 0);
    -- For 200Mhz two jumps between states takes 10ns
    constant two_states_offset : std_logic_vector(4 downto 0) := "00010";
    constant delay_offset : std_logic_vector(4 downto 0) := "11110";
    -- Command definitions
    constant CMD_NOP        : std_logic_vector(3 downto 0) := "0111";
    constant CMD_ACTIVE     : std_logic_vector(3 downto 0) := "0011";
    constant CMD_READ       : std_logic_vector(3 downto 0) := "0101";
    constant CMD_WRITE      : std_logic_vector(3 downto 0) := "0100";
    constant CMD_PRECHARGE  : std_logic_vector(3 downto 0) := "0010";
    constant CMD_REFRESH    : std_logic_vector(3 downto 0) := "0001";
    constant CMD_LOAD_MODE  : std_logic_vector(3 downto 0) := "0000";


    -- SDRAM mode register data sent on the address bus.
    --
    -- | A12-A10 |    A9    | A8  A7 | A6 A5 A4 |    A3    | A2 A1 A0 |
    -- | reserved| wr mode  |reserved| CAS Ltncy|burst mode| burst len|
    -- | 0  0  0 |    0     | 0   0  | 0  1  0  |     0    |  0  0  0 |
    constant MODE_REG : std_logic_vector(12 downto 0) := "000" & "0" & "00" & "010" & "0" & "000";

begin

    -- Assign SDRAM signals
    CLK_SDRAM <= CLK_200MHz;
    CKE <= '1';
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
                    if init_counter = 400 then
                        memory_state <= SDRAM_MODE;
                    else
                        init_counter <= init_counter + 1;
                    end if;

                when SDRAM_MODE =>
                    BA <= "00";
                    A <= MODE_REG;
                    command <= CMD_LOAD_MODE;
                    memory_state <= SDRAM_MODE_DELAY;

                when SDRAM_MODE_DELAY =>
                    if delay_mode = delay_offset - two_states_offset then
                        delay_mode <= (others => '0');
                        memory_state <= SDRAM_IDLE;
                    else
                        delay_mode <= delay_mode + '1';
                    end if;

                when SDRAM_IDLE =>
                    if WRITE_EN = '1' then
                        process_write <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif READ_EN = '1' then
                        process_read <= '1';
                        memory_state <= SDRAM_ACTIVE;
                    elsif refresh_counter = 8192 then
                        memory_state <= SDRAM_REFRESH;
                    else
                        command <= CMD_NOP;
                    end if;

                when SDRAM_ACTIVE =>
                    command <= CMD_ACTIVE;
                    A <= ADDR(23 downto 11); -- Row address
                    BA <= ADDR(10 downto 9); -- Bank address
                    memory_state <= SDRAM_RAS_TO_CAS_DELAY;

                when SDRAM_RAS_TO_CAS_DELAY =>
                    if delay_ras_to_cas = delay_offset - two_states_offset then
                        delay_ras_to_cas <= (others => '0');
                        if process_write = '1' then
                            memory_state <= SDRAM_WRITE_CMD;
                        elsif process_read = '1' then
                            memory_state <= SDRAM_READ_CMD;
                        end if;
                    else
                        delay_ras_to_cas <= delay_ras_to_cas + '1';
                    end if;

                when SDRAM_WRITE_CMD =>
                    command <= CMD_WRITE;
                    DQ <= DATA_IN;
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address
                    A(10) <= '1'; -- Auto-precharge
                    memory_state <= SDRAM_OP_DELAY;

                when SDRAM_READ_CMD =>
                    command <= CMD_READ;
                    DQ <= (others => 'Z');  -- Set data bus to high-impedance
                    A(8 downto 0) <= ADDR(8 downto 0); -- Column address
                    A(10) <= '1'; -- Auto-precharge
                    memory_state <= SDRAM_OP_DELAY;

                when SDRAM_OP_DELAY =>
                    if delay_op = delay_offset - two_states_offset then
                        delay_op <= (others => '0');
                            if process_write = '1' then
                                memory_state <= SDRAM_DONE;
                            elsif process_read = '1' then
                                DATA_OUT <= DQ;
                                memory_state <= SDRAM_READ_DATA;
                            end if;
                    else
                        delay_op <= delay_op + '1';
                    end if;

                when SDRAM_READ_DATA =>
                    memory_state <= SDRAM_DONE;

                when SDRAM_DONE =>
                    process_read <= '0';
                    process_write <= '0';
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

end rtl;
