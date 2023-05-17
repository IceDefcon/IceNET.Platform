library IEEE;
    use IEEE.std_logic_1164.all;
    use IEEE.numeric_std.all;

entity DeBounce is
    port(    clk : in std_logic;
             btn : in std_logic;
         btn_clr : out std_logic);
end DeBounce;

architecture rtl of DeBounce is

    constant delay : integer := 1000000; -- For 50Mhz clock this is 20ms
    signal count : integer := 0;
    signal btn_tmp : std_logic := '0';

    begin

    process(clk)
    begin
        if rising_edge(clk) then
            if (btn /= btn_tmp) then
                btn_tmp <= btn;
                count <= 0;
            elsif (count = delay) then
                btn_clr <= btn_tmp;
            else
                count <= count + 1;
            end if;
        end if;
    end process;

end rtl;