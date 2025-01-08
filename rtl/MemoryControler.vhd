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
    IDLE,
    PRODUCE,
    DONE
);
signal ram_state: MEMORY := IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case ram_state is
                when IDLE =>

                when PRODUCE =>

                when DONE =>

                when others =>
                    ram_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
