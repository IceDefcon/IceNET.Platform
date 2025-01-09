library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity MemoryControler is
Port
(
    CLOCK_50MHz : in  std_logic;
    MEMORY_CLOCK : out  std_logic;

    MEMORY_ADDRESS : out std_logic_vector(12 downto 0);
    MEMORY_DATA : inout std_logic_vector(15 downto 0);
    MEMORY_BANK : out std_logic_vector(1 downto 0);

    MEMORY_CAS : out std_logic;
    MEMORY_CKE : out std_logic;
    MEMORY_RAS : out std_logic;
    MEMORY_WE : out std_logic;
    MEMORY_CS : out std_logic;

    MEMORY_DQML : inout std_logic;
    MEMORY_DQMH : inout std_logic
);
end entity MemoryControler;

architecture rtl of MemoryControler is

type MEMORY is
(
    MEMORY_IDLE,
    MEMORY_INIT,
    MEMORY_PRECHARGE,
    MEMORY_AUTO_REFRESH,
    MEMORY_ACTIVATE,
    MEMORY_WRITE,
    MEMORY_READ
);
signal ram_state: MEMORY := MEMORY_IDLE;

begin

    process (CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then
            case ram_state is

                when MEMORY_IDLE =>
                    -- Idle state

                when MEMORY_INIT =>
                    -- Power-up and initialization sequence

                when MEMORY_PRECHARGE =>
                    -- Issue precharge all command

                when MEMORY_AUTO_REFRESH =>
                    -- Issue auto-refresh commands

                when MEMORY_ACTIVATE =>
                    -- Issue activate command with row address

                when MEMORY_WRITE =>
                    -- Drive column address and write data to SDRAM

                when MEMORY_READ =>
                    -- Issue read command and capture data from SDRAM

                when others =>
                    ram_state <= MEMORY_IDLE;
            end case;
        end if;
    end process;

end architecture rtl;
