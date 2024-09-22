library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity FifoCtrl is
generic 
(
    WIDTH : integer := 8;
    DEPTH : integer := 16
);
port 
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;
    -- In
    DATA_IN : in  std_logic_vector(WIDTH-1 downto 0);
    WRITE_EN : in  std_logic;
    READ_EN : in  std_logic;
    -- Out
    DATA_OUT : out std_logic_vector(WIDTH-1 downto 0);
    FULL : out std_logic;
    EMPTY : out std_logic
);
end FifoCtrl;

architecture rtl of FifoCtrl is

type mem_type is array (0 to DEPTH-1) of std_logic_vector(WIDTH-1 downto 0);
signal memory : mem_type;
signal wr_ptr : integer range 0 to DEPTH-1 := 0;
signal rd_ptr : integer range 0 to DEPTH-1 := 0;
signal depth_count : integer range 0 to DEPTH := 0;

begin

    process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            wr_ptr <= 0;
            rd_ptr <= 0;
            depth_count  <= 0;
        elsif rising_edge(CLOCK_50MHz) then
            -- WRITE
            if WRITE_EN = '1' then
                if depth_count < DEPTH then
                    memory(wr_ptr) <= DATA_IN;
                    wr_ptr <= (wr_ptr + 1) mod DEPTH;
                    depth_count <= depth_count + 1;
                end if;
            end if;
            -- READ
            if READ_EN = '1' then
                if depth_count > 0 then
                    DATA_OUT <= memory(rd_ptr);
                    rd_ptr <= (rd_ptr + 1) mod DEPTH;
                    depth_count <= depth_count - 1;
                end if;
            end if;
        end if;
    end process;

    FULL  <= '1' when depth_count = DEPTH else '0';
    EMPTY <= '1' when depth_count = 0 else '0';

end rtl;
