library ieee;
use ieee.std_logic_1164.all;

entity spi_driver is
    port (
        clk: in std_logic;
        cs: in std_logic;
        mosi: in std_logic;
        miso: out std_logic;
        parallel_out: out std_logic_vector(7 downto 0)
    );
end spi_driver;

architecture rtl of spi_driver is
    signal shift_reg: std_logic_vector(7 downto 0);
    signal shift_reg_index: integer range 0 to 7 := 0;
    signal mosi_out: std_logic := '0';
    signal parallel_data: std_logic_vector(7 downto 0) := (others => '0');
begin

    -- Process to handle SPI communication
    spi_process: process(clk)
    begin
        if rising_edge(clk) then
            if cs = '0' then  -- Chip select active

                -- Shift in data from MOSI into shift register
                shift_reg(shift_reg_index) <= mosi;

                if shift_reg_index = 7 then
                    -- All bits shifted in, perform desired action
                    -- Here, we simply pass the received data back through MISO
                    miso <= mosi_out;

                    -- Assign parallel data output from shift register
                    parallel_data <= shift_reg;
                    
                    -- Reset shift register and index for next transmission
                    shift_reg <= (others => '0');
                    shift_reg_index <= 0;
                else
                    -- Increment shift register index for next bit
                    shift_reg_index <= shift_reg_index + 1;
                end if;

            else
                -- Chip select inactive, clear shift register and output
                shift_reg <= (others => '0');
                shift_reg_index <= 0;
                miso <= 'Z';  -- Tri-state MISO output
                parallel_data <= (others => '0');  -- Clear parallel output
            end if;
        end if;
    end process spi_process;
    
    -- Assign MISO output based on shift register value
    mosi_out <= shift_reg(7);
    
    -- Assign parallel output
    parallel_out <= parallel_data;

end rtl;
