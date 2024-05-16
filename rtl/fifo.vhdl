library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity fifo is
generic 
(
    WIDTH   : integer := 8;
    DEPTH   : integer := 16
);
port 
(
    clk     : in  std_logic;
    reset   : in  std_logic;
    data_in : in  std_logic_vector(WIDTH-1 downto 0);
    wr_en   : in  std_logic;
    rd_en   : in  std_logic;
    data_out: out std_logic_vector(WIDTH-1 downto 0);
    full    : out std_logic;
    empty   : out std_logic
);
end fifo;

architecture rtl of fifo is

type mem_type is array (0 to DEPTH-1) of std_logic_vector(WIDTH-1 downto 0);
signal memory : mem_type;
signal wr_ptr : integer range 0 to DEPTH-1 := 0;
signal rd_ptr : integer range 0 to DEPTH-1 := 0;
signal count : integer range 0 to DEPTH := 0;
-- Debugs to remove
signal curr : std_logic_vector(WIDTH-1 downto 0) := (others => '0');
signal prev : std_logic_vector(WIDTH-1 downto 0) := (others => '0');
-- ctrl byte is stored ready
signal ctrl : std_logic := '0';

begin

process(clk, reset)
begin
    if reset = '1' then
        wr_ptr <= 0;
        rd_ptr <= 0;
        count  <= 0;
    elsif rising_edge(clk) then
        if wr_en = '1' and count < DEPTH then
            memory(wr_ptr) <= data_in;
            wr_ptr <= (wr_ptr + 1) mod DEPTH;
            count <= count + 1;
        end if;
        
        if rd_en = '1' and count > 0 then
            data_out <= memory(rd_ptr);
            rd_ptr <= (rd_ptr + 1) mod DEPTH;
            count <= count - 1;
        end if;
    end if;
end process;

full  <= '1' when count = DEPTH else '0';
empty <= '1' when count = 0 else '0';

end rtl;
