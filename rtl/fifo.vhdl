library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity fifo is
port 
(
    clk     : in  std_logic;
    reset   : in  std_logic;
    data_in : in  std_logic_vector(7 downto 0);
    wr_en   : in  std_logic;
    rd_en   : in  std_logic;
    data_out: out std_logic_vector(7 downto 0);
    full    : out std_logic;
    empty   : out std_logic
);
end fifo;

architecture rtl of fifo is

type mem_type is array (0 to 15) of std_logic_vector(7 downto 0);
signal memory   : mem_type;
signal wr_ptr   : integer range 0 to 15 := 0;
signal rd_ptr   : integer range 0 to 15 := 0;
signal count    : integer range 0 to 16 := 0;

begin

    process(clk, reset)
    begin
        if reset = '1' then
            wr_ptr <= 0;
            rd_ptr <= 0;
            count  <= 0;
        elsif rising_edge(clk) then
            if wr_en = '1' then
                memory(wr_ptr) <= data_in;
                wr_ptr <= (wr_ptr + 1) mod 16;
                count <= count + 1;
            end if;
            if rd_en = '1' then
                data_out <= memory(rd_ptr);
                rd_ptr <= (rd_ptr + 1) mod 16;
                count <= count - 1;
            end if;
        end if;
    end process;

    full  <= '1' when count = 16 else '0';
    empty <= '1' when count = 0 else '0';

end rtl;
