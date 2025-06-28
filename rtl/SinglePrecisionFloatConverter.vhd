library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-------------------------
--
-- Author: Ice.Marek
-- IceNET Robotics 2025
--
-------------------------

entity SinglePrecisionFloatConverter is
Port
(
    CLOCK_250MHz : in  std_logic;
    RESET        : in  std_logic;

    FIXED_IN  : in  std_logic_vector(15 downto 0);  -- 16-bit 2's complement
    FLOAT_OUT : out std_logic_vector(31 downto 0)   -- 32-bit IEEE 754 float
);
end SinglePrecisionFloatConverter;

architecture rtl of SinglePrecisionFloatConverter is

    function FloatConversion
    (
        twoComplment : signed(15 downto 0)
    )
    return std_logic_vector is

        variable result          : std_logic_vector(31 downto 0);
        variable AbsoluteValue   : unsigned(15 downto 0);
        variable Exponent        : integer := 0;
        variable Mantissa        : unsigned(22 downto 0) := (others => '0');
        variable leading_one     : integer := 0;
        variable Normalized_Wide : unsigned(38 downto 0);  -- 39-bit wide for safe shifting
        variable SignBit         : std_logic;

    begin
        -- Handle zero input
        if twoComplment = 0 then
            return (31 downto 0 => '0');
        end if;

        -- Determine sign and absolute value
        if twoComplment < 0 then
            SignBit := '1';
            AbsoluteValue := unsigned(-twoComplment);
        else
            SignBit := '0';
            AbsoluteValue := unsigned(twoComplment);
        end if;

        -- Find position of the leading 1
        for i in 15 downto 0 loop
            if AbsoluteValue(i) = '1' then
                leading_one := i;
                exit;
            end if;
        end loop;

        -- Calculate exponent with bias (127 for single-precision)
        Exponent := leading_one + 127;

        -- Normalize the number by shifting to left into a wider vector
        Normalized_Wide := resize(AbsoluteValue, 39) sll (38 - leading_one);

        -- Extract 23-bit mantissa (exclude implicit leading 1)
        Mantissa := Normalized_Wide(37 downto 15);  -- 23 bits starting after the leading 1

        -- Assemble IEEE 754 single-precision float
        result(31)           := SignBit;
        result(30 downto 23) := std_logic_vector(to_unsigned(Exponent, 8));
        result(22 downto 0)  := std_logic_vector(Mantissa);

        return result;
    end function;

begin

    process(FIXED_IN)
        variable signed_input : signed(15 downto 0);
    begin
        signed_input := signed(FIXED_IN);
        FLOAT_OUT <= FloatConversion(signed_input);
    end process;

end rtl;
