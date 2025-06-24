    library ieee;
    use ieee.numeric_std.all;
    use ieee.std_logic_1164.all;

    entity TwoComplementConverter is
        Port (
            CLOCK_250MHz : in  std_logic;
            RESET        : in  std_logic;

            I_12_BITS    : in  std_logic_vector(11 downto 0);
            O_16_BITS    : out std_logic_vector(15 downto 0)
        );
    end TwoComplementConverter;

    architecture Behavioral of TwoComplementConverter is
    begin

        process(CLOCK_250MHz, RESET)
        begin
            if RESET = '1' then
                O_16_BITS <= (others => '0');
            elsif rising_edge(CLOCK_250MHz) then
                if I_12_BITS(11) = '0' then
                    O_16_BITS <= "0000" & I_12_BITS;
                else
                    O_16_BITS <= "1111" & I_12_BITS;
                end if;
            end if;
        end process;

    end Behavioral;
