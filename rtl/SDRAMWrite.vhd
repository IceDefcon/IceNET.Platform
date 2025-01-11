library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity SDRAMWrite is
    Port (
        CLK         : in  std_logic;        -- System Clock
        RESET       : in  std_logic;        -- Reset signal
        SDRAM_CLK   : out std_logic;        -- SDRAM Clock
        CKE         : out std_logic;        -- Clock Enable
        CS          : out std_logic;        -- Chip Select
        RAS         : out std_logic;        -- Row Address Strobe
        CAS         : out std_logic;        -- Column Address Strobe
        WE          : out std_logic;        -- Write Enable
        A           : out std_logic_vector(12 downto 0); -- Address Bus
        BA          : out std_logic_vector(1 downto 0);  -- Bank Address
        DQ          : inout std_logic_vector(15 downto 0); -- Data Bus
        LDQM        : out std_logic;        -- Lower Data Mask
        UDQM        : out std_logic;        -- Upper Data Mask
        WRITE_EN    : in  std_logic;        -- External Signal to Trigger Write
        DATA_IN     : in  std_logic_vector(15 downto 0)  -- Data to Write
    );
end SDRAMWrite;

architecture Behavioral of SDRAMWrite is
    type state_type is (IDLE, ACTIVATE, WRITE, PRECHARGE);
    signal state       : state_type := IDLE;
    signal row_address : std_logic_vector(12 downto 0) := "0000000000101"; -- Example row address
    signal col_address : std_logic_vector(12 downto 0) := "0000000001010"; -- Example column address
    signal bank_select : std_logic_vector(1 downto 0)  := "00"; -- Example bank address
    signal dq_internal : std_logic_vector(15 downto 0); -- Internal Data Bus
begin

    -- SDRAM Clock
    SDRAM_CLK <= CLK;

    -- Main Process for Writing Data
    process(CLK, RESET)
    begin
        if RESET = '1' then
            state <= IDLE;
            CKE <= '0';
            CS <= '1';
            RAS <= '1';
            CAS <= '1';
            WE <= '1';
            LDQM <= '1';
            UDQM <= '1';
        elsif rising_edge(CLK) then
            case state is
                when IDLE =>
                    if WRITE_EN = '1' then
                        state <= ACTIVATE;
                        CKE <= '1'; -- Enable SDRAM Clock
                        CS <= '0';  -- Select SDRAM
                        RAS <= '0'; -- Activate Command
                        CAS <= '1';
                        WE <= '1';
                        A <= row_address; -- Set Row Address
                        BA <= bank_select; -- Select Bank
                    end if;

                when ACTIVATE =>
                    state <= WRITE;
                    RAS <= '1'; -- De-assert RAS for the next command
                    CAS <= '0'; -- Column Address Strobe
                    WE <= '0';  -- Write Enable
                    A <= col_address; -- Set Column Address
                    BA <= bank_select; -- Select Bank
                    dq_internal <= DATA_IN; -- Load Data to Internal Buffer

                when WRITE =>
                    state <= PRECHARGE;
                    DQ <= dq_internal; -- Drive Data onto the Bus
                    LDQM <= '0'; -- Enable Lower Data Mask
                    UDQM <= '0'; -- Enable Upper Data Mask

                when PRECHARGE =>
                    state <= IDLE;
                    RAS <= '0'; -- Precharge Command
                    CAS <= '1';
                    WE <= '0';
                    A(10) <= '1'; -- All banks precharge
                    BA <= bank_select;

                when others =>
                    state <= IDLE;
            end case;
        end if;
    end process;

end Behavioral;
