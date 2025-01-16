library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;

entity RamReadController is
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
    LDQM : out std_logic;
    UDQM : out std_logic;

    SDRAM_READ_EN : in std_logic;
    SDRAM_DATA_OUT : out std_logic_vector(15 downto 0)
);
end entity RamReadController;

architecture rtl of RamReadController is

type MEMORY is
(
    MEMORY_IDLE,
    MEMORY_INIT,
    MEMORY_ACTIVATE,
    MEMORY_READ,
    MEMORY_PRECHARGE
);
signal ram_state: MEMORY := MEMORY_IDLE;

signal dq_internal : std_logic_vector(15 downto 0); -- Internal Data Bus
signal row_address : std_logic_vector(12 downto 0) := "0000000000101"; -- Example row address
signal col_address : std_logic_vector(12 downto 0) := "0000000001010"; -- Example column address
signal bank_select : std_logic_vector(1 downto 0)  := "00"; -- Example bank address
signal timing_counter : integer := 0;

begin

    process (CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            CLK_SDRAM <= CLOCK_50MHz;

            case ram_state is

                when MEMORY_IDLE =>
                    if SDRAM_READ_EN = '1' then
                        ram_state <= MEMORY_INIT;
                    end if;

                when MEMORY_INIT =>
                    CAS <= '1';
                    CKE <= '1'; -- Enable SDRAM Clock
                    RAS <= '0'; -- Activate Command
                    WE <= '1';
                    CS <= '0';  -- Select SDRAM
                    A0 <= row_address(0);
                    A1 <= row_address(1);
                    A2 <= row_address(2);
                    A3 <= row_address(3);
                    A4 <= row_address(4);
                    A5 <= row_address(5);
                    A6 <= row_address(6);
                    A7 <= row_address(7);
                    A8 <= row_address(8);
                    A9 <= row_address(9);
                    A10 <= row_address(10);
                    A11 <= row_address(11);
                    A12 <= row_address(12);
                    BA0 <= bank_select(0);
                    BA1 <= bank_select(1);
                    ram_state <= MEMORY_ACTIVATE;

                when MEMORY_ACTIVATE =>
                    if timing_counter < 3 then  -- tRCD (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        CAS <= '0'; -- Column Address Strobe
                        RAS <= '1'; -- De-assert RAS for the next command
                        WE <= '1';  -- Read Command
                        A0 <= col_address(0);
                        A1 <= col_address(1);
                        A2 <= col_address(2);
                        A3 <= col_address(3);
                        A4 <= col_address(4);
                        A5 <= col_address(5);
                        A6 <= col_address(6);
                        A7 <= col_address(7);
                        A8 <= col_address(8);
                        A9 <= col_address(9);
                        A10 <= col_address(10);
                        A11 <= col_address(11);
                        A12 <= col_address(12);
                        BA0 <= bank_select(0);
                        BA1 <= bank_select(1);
                        ram_state <= MEMORY_READ;
                    end if;

                when MEMORY_READ =>
                    LDQM <= '0'; -- Enable Lower Data Mask
                    UDQM <= '0'; -- Enable Upper Data Mask
                    if timing_counter < 3 then  -- CL (CAS Latency: 3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        dq_internal(0) <= D0;
                        dq_internal(1) <= D1;
                        dq_internal(2) <= D2;
                        dq_internal(3) <= D3;
                        dq_internal(4) <= D4;
                        dq_internal(5) <= D5;
                        dq_internal(6) <= D6;
                        dq_internal(7) <= D7;
                        dq_internal(8) <= D8;
                        dq_internal(9) <= D9;
                        dq_internal(10) <= D10;
                        dq_internal(11) <= D11;
                        dq_internal(12) <= D12;
                        dq_internal(13) <= D13;
                        dq_internal(14) <= D14;
                        dq_internal(15) <= D15;
                        SDRAM_DATA_OUT <= dq_internal; -- Output data to external port
                        ram_state <= MEMORY_PRECHARGE;
                    end if;

                when MEMORY_PRECHARGE =>
                    CAS <= '1';
                    RAS <= '0'; -- Precharge Command
                    WE <= '0';
                    A10 <= '1'; -- All banks precharge
                    BA0 <= bank_select(0);
                    BA1 <= bank_select(1);
                    if timing_counter < 3 then  -- tRP (3 clock cycles at 50 MHz)
                        timing_counter <= timing_counter + 1;
                    else
                        timing_counter <= 0;
                        ram_state <= MEMORY_IDLE;
                    end if;

                when others =>
                    ram_state <= MEMORY_IDLE;

            end case;
        end if;
    end process;

end architecture rtl;
