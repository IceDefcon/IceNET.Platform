library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity RamControler is
Port
(
    CLOCK_50MHz : in  std_logic;
    CLK_SDRAM : out  std_logic;

    A0 : out std_logic;
    A1 : out std_logic;
    A2 : out std_logic;
    A3 : out std_logic;
    A4 : out std_logic;
    A5 : out std_logic;
    A6 : out std_logic;
    A7 : out std_logic;
    A8 : out std_logic;
    A9 : out std_logic;
    A10 : out std_logic;
    A11 : out std_logic;
    A12 : out std_logic;

    BA0 : out std_logic;
    BA1 : out std_logic;

    D0 : inout std_logic;
    D1 : inout std_logic;
    D2 : inout std_logic;
    D3 : inout std_logic;
    D4 : inout std_logic;
    D5 : inout std_logic;
    D6 : inout std_logic;
    D7 : inout std_logic;
    D8 : inout std_logic;
    D9 : inout std_logic;
    D10 : inout std_logic;
    D11 : inout std_logic;
    D12 : inout std_logic;
    D13 : inout std_logic;
    D14 : inout std_logic;
    D15 : inout std_logic;

    CAS : out std_logic;
    CKE : out std_logic;
    RAS : out std_logic;
    WE : out std_logic;
    CS : out std_logic;
    LDQM : inout std_logic;
    UDQM : inout std_logic;

    SDRAM_WRITE_EN : in std_logic;
    SDRAM_DATA_IN : in std_logic_vector(15 downto 0);
    SDRAM_READ_EN : in std_logic;
    SDRAM_DATA_OUT : out std_logic_vector(15 downto 0)
);
end entity RamControler;

architecture rtl of RamControler is

type WRITE_STATE is
(
    WRITE_IDLE,
    WRITE_INIT,
    WRITE_ACTIVATE,
    WRITE_PROCESS,
    WRITE_PRECHARGE
);
signal write_ram_state: WRITE_STATE := WRITE_IDLE;

type READ_STATE is
(
    READ_IDLE,
    READ_INIT,
    READ_ACTIVATE,
    READ_PROCESS,
    READ_PRECHARGE
);
signal read_ram_state: READ_STATE := READ_IDLE;

signal write_dq_internal : std_logic_vector(15 downto 0); -- Internal Data Bus
signal write_row_address : std_logic_vector(12 downto 0) := (others => '0');
signal write_col_address : std_logic_vector(12 downto 0) := (others => '0');
signal write_bank_select : std_logic_vector(1 downto 0)  := (others => '0');
signal read_dq_internal : std_logic_vector(15 downto 0); -- Internal Data Bus
signal read_row_address : std_logic_vector(12 downto 0) := (others => '0');
signal read_col_address : std_logic_vector(12 downto 0) := (others => '0');
signal read_bank_select : std_logic_vector(1 downto 0)  := (others => '0');
signal timing_counter : integer := 0;

begin

    process (CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            CLK_SDRAM <= CLOCK_50MHz;

            --------------------------------------------------------------
            --
            -- WRITE
            --
            --------------------------------------------------------------
            case write_ram_state is

                when WRITE_IDLE =>
                    if SDRAM_WRITE_EN = '1' then
                        write_row_address <= write_row_address + '1';
                        write_col_address <= write_col_address + '1';
                        write_ram_state <= WRITE_INIT;
                    end if;

                when WRITE_INIT =>
                    CAS <= '1';
                    CKE <= '1';
                    RAS <= '0';
                    WE <= '1';
                    CS <= '0';
                    A0 <= write_row_address(0);
                    A1 <= write_row_address(1);
                    A2 <= write_row_address(2);
                    A3 <= write_row_address(3);
                    A4 <= write_row_address(4);
                    A5 <= write_row_address(5);
                    A6 <= write_row_address(6);
                    A7 <= write_row_address(7);
                    A8 <= write_row_address(8);
                    A9 <= write_row_address(9);
                    A10 <= write_row_address(10);
                    A11 <= write_row_address(11);
                    A12 <= write_row_address(12);
                    BA0 <= write_bank_select(0);
                    BA1 <= write_bank_select(1);
                    write_ram_state <= WRITE_ACTIVATE;

                when WRITE_ACTIVATE =>
                    if timing_counter < 3 then  -- tRCD (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        CAS <= '0'; -- Column Address Strobe
                        RAS <= '1'; -- De-assert RAS for the next command
                        WE <= '0';  -- Write Enable
                        A0 <= write_col_address(0);
                        A1 <= write_col_address(1);
                        A2 <= write_col_address(2);
                        A3 <= write_col_address(3);
                        A4 <= write_col_address(4);
                        A5 <= write_col_address(5);
                        A6 <= write_col_address(6);
                        A7 <= write_col_address(7);
                        A8 <= write_col_address(8);
                        A9 <= write_col_address(9);
                        A10 <= write_col_address(10);
                        A11 <= write_col_address(11);
                        A12 <= write_col_address(12);
                        BA0 <= write_bank_select(0);
                        BA1 <= write_bank_select(1);
                        write_dq_internal <= SDRAM_DATA_IN; -- Load Data to Internal Buffer
                        write_ram_state <= WRITE_PROCESS;
                    end if;

                when WRITE_PROCESS =>
                    D0 <= write_dq_internal(0);
                    D1 <= write_dq_internal(1);
                    D2 <= write_dq_internal(2);
                    D3 <= write_dq_internal(3);
                    D4 <= write_dq_internal(4);
                    D5 <= write_dq_internal(5);
                    D6 <= write_dq_internal(6);
                    D7 <= write_dq_internal(7);
                    D8 <= write_dq_internal(8);
                    D9 <= write_dq_internal(9);
                    D10 <= write_dq_internal(10);
                    D11 <= write_dq_internal(11);
                    D12 <= write_dq_internal(12);
                    D13 <= write_dq_internal(13);
                    D14 <= write_dq_internal(14);
                    D15 <= write_dq_internal(15);
                    LDQM <= '0'; -- Enable Lower Data Mask
                    UDQM <= '0'; -- Enable Upper Data Mask
                    if timing_counter < 3 then  -- tWR (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        write_ram_state <= WRITE_PRECHARGE;
                    end if;

                when WRITE_PRECHARGE =>
                    CAS <= '1';
                    RAS <= '0'; -- Precharge Command
                    WE <= '0';
                    A10 <= '1'; -- All banks precharge
                    BA0 <= write_bank_select(0);
                    BA1 <= write_bank_select(1);
                    if timing_counter < 3 then  -- tRP (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        write_ram_state <= WRITE_IDLE;
                    end if;

                when others =>
                    write_ram_state <= WRITE_IDLE;

            end case;

            --------------------------------------------------------------
            --
            -- READ
            --
            --------------------------------------------------------------
            case read_ram_state is

                when READ_IDLE =>
                    if SDRAM_READ_EN = '1' then
                        read_row_address <= read_row_address + '1';
                        read_col_address <= read_col_address + '1';
                        read_ram_state <= READ_INIT;
                    end if;

                when READ_INIT =>
                    CAS <= '1';
                    CKE <= '1'; -- Enable SDRAM Clock
                    RAS <= '0'; -- Activate Command
                    WE <= '1';
                    CS <= '0';  -- Select SDRAM
                    A0 <= read_row_address(0);
                    A1 <= read_row_address(1);
                    A2 <= read_row_address(2);
                    A3 <= read_row_address(3);
                    A4 <= read_row_address(4);
                    A5 <= read_row_address(5);
                    A6 <= read_row_address(6);
                    A7 <= read_row_address(7);
                    A8 <= read_row_address(8);
                    A9 <= read_row_address(9);
                    A10 <= read_row_address(10);
                    A11 <= read_row_address(11);
                    A12 <= read_row_address(12);
                    BA0 <= read_bank_select(0);
                    BA1 <= read_bank_select(1);
                    D0 <= 'Z';
                    D1 <= 'Z';
                    D2 <= 'Z';
                    D3 <= 'Z';
                    D4 <= 'Z';
                    D5 <= 'Z';
                    D6 <= 'Z';
                    D7 <= 'Z';
                    D8 <= 'Z';
                    D9 <= 'Z';
                    D10 <= 'Z';
                    D11 <= 'Z';
                    D12 <= 'Z';
                    D13 <= 'Z';
                    D14 <= 'Z';
                    D15 <= 'Z';
                    LDQM <= 'Z';
                    UDQM <= 'Z';
                    read_ram_state <= READ_ACTIVATE;

                when READ_ACTIVATE =>
                    if timing_counter < 3 then  -- tRCD (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        CAS <= '0'; -- Column Address Strobe
                        RAS <= '1'; -- De-assert RAS for the next command
                        WE <= '1';  -- Read Command
                        A0 <= read_col_address(0);
                        A1 <= read_col_address(1);
                        A2 <= read_col_address(2);
                        A3 <= read_col_address(3);
                        A4 <= read_col_address(4);
                        A5 <= read_col_address(5);
                        A6 <= read_col_address(6);
                        A7 <= read_col_address(7);
                        A8 <= read_col_address(8);
                        A9 <= read_col_address(9);
                        A10 <= read_col_address(10);
                        A11 <= read_col_address(11);
                        A12 <= read_col_address(12);
                        BA0 <= read_bank_select(0);
                        BA1 <= read_bank_select(1);
                        read_ram_state <= READ_PROCESS;
                    end if;

                when READ_PROCESS =>
                    LDQM <= '0'; -- Enable Lower Data Mask
                    UDQM <= '0'; -- Enable Upper Data Mask
                    if timing_counter < 3 then  -- CL (CAS Latency: 3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        read_dq_internal(0) <= D0;
                        read_dq_internal(1) <= D1;
                        read_dq_internal(2) <= D2;
                        read_dq_internal(3) <= D3;
                        read_dq_internal(4) <= D4;
                        read_dq_internal(5) <= D5;
                        read_dq_internal(6) <= D6;
                        read_dq_internal(7) <= D7;
                        read_dq_internal(8) <= D8;
                        read_dq_internal(9) <= D9;
                        read_dq_internal(10) <= D10;
                        read_dq_internal(11) <= D11;
                        read_dq_internal(12) <= D12;
                        read_dq_internal(13) <= D13;
                        read_dq_internal(14) <= D14;
                        read_dq_internal(15) <= D15;
                        SDRAM_DATA_OUT <= read_dq_internal; -- Output data to external port
                        read_ram_state <= READ_PRECHARGE;
                    end if;

                when READ_PRECHARGE =>
                    CAS <= '1';
                    RAS <= '0'; -- Precharge Command
                    WE <= '0';
                    A10 <= '1'; -- All banks precharge
                    BA0 <= read_bank_select(0);
                    BA1 <= read_bank_select(1);
                    if timing_counter < 3 then  -- tRP (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        read_ram_state <= READ_IDLE;
                    end if;

                when others =>
                    read_ram_state <= READ_IDLE;

            end case;
        end if;
    end process;

end architecture rtl;
