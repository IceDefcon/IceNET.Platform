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
    SDRAM_READ_EN : in std_logic;
    SDRAM_DATA_IN : in std_logic_vector(15 downto 0);
    SDRAM_DATA_OUT : out std_logic_vector(15 downto 0)
);
end entity RamControler;

architecture rtl of RamControler is

type MEMORY is
(
    MEMORY_IDLE,
    MEMORY_INIT,
    MEMORY_ACTIVATE,
    MEMORY_WRITE,
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
                    ram_state <= MEMORY_INIT; -- Prepare for read

                when MEMORY_INIT =>
                    ram_state <= MEMORY_ACTIVATE;

                when MEMORY_ACTIVATE =>
                    ram_state <= MEMORY_WRITE;

                when MEMORY_WRITE =>
                    ram_state <= MEMORY_PRECHARGE;

                when MEMORY_READ =>
                    ram_state <= MEMORY_PRECHARGE;

                when MEMORY_PRECHARGE =>
                    ram_state <= MEMORY_IDLE;

                when others =>
                    ram_state <= MEMORY_IDLE;

            end case;
        end if;
    end process;

end architecture rtl;
