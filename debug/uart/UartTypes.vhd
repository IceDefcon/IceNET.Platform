library ieee;
use ieee.std_logic_1164.all;

package UartTypes is

    subtype UART_ASCII is std_logic_vector(7 downto 0);
    type UART_LOG_ID   is array (0 to 1) of std_logic_vector(3 downto 0);
    type UART_LOG_KEY  is array (0 to 1) of std_logic_vector(3 downto 0);
    type UART_LOG_DATA is array (0 to 3) of std_logic_vector(3 downto 0);

    constant PARAMETER_NUMBER : integer := 2;
    type PARAMETER_ARRAY is array (0 to PARAMETER_NUMBER - 1) of std_logic_vector(31 downto 0);

    function HEX_TO_ASCII
    (
        hex_code : std_logic_vector(3 downto 0)
    )
    return UART_ASCII;

    function ASCII_TO_HEX
    (
        ascii_code : UART_ASCII
    )
    return std_logic_vector;

    constant ASCII_R     : std_logic_vector(7 downto 0) := "01010010"; -- 0x52
    constant ASCII_COLON : std_logic_vector(7 downto 0) := "00111010"; -- 0x3A

    constant ASCII_LF    : std_logic_vector(7 downto 0) := "00001010";
    constant ASCII_CR    : std_logic_vector(7 downto 0) := "00001100";
    constant ASCII_DOT   : std_logic_vector(7 downto 0) := "00101110";
    constant ASCII_SPACE : std_logic_vector(7 downto 0) := "00100000";

end package UartTypes;

package body UartTypes is

    function HEX_TO_ASCII
    (
        hex_code : std_logic_vector(3 downto 0)
    )
    return UART_ASCII is
        variable result : UART_ASCII;
    begin
        case hex_code is
            when "0000" => result := "00110000"; -- ASCII '0'
            when "0001" => result := "00110001"; -- ASCII '1'
            when "0010" => result := "00110010"; -- ASCII '2'
            when "0011" => result := "00110011"; -- ASCII '3'
            when "0100" => result := "00110100"; -- ASCII '4'
            when "0101" => result := "00110101"; -- ASCII '5'
            when "0110" => result := "00110110"; -- ASCII '6'
            when "0111" => result := "00110111"; -- ASCII '7'
            when "1000" => result := "00111000"; -- ASCII '8'
            when "1001" => result := "00111001"; -- ASCII '9'
            when "1010" => result := "01000001"; -- ASCII 'A'
            when "1011" => result := "01000010"; -- ASCII 'B'
            when "1100" => result := "01000011"; -- ASCII 'C'
            when "1101" => result := "01000100"; -- ASCII 'D'
            when "1110" => result := "01000101"; -- ASCII 'E'
            when "1111" => result := "01000110"; -- ASCII 'F'
            when others => result := "00000000"; -- Default (undefined input)
        end case;

        return result;
    end function HEX_TO_ASCII;

    function ASCII_TO_HEX
    (
        ascii_code : UART_ASCII
    )
    return std_logic_vector is
        variable result : std_logic_vector(3 downto 0);
    begin
        case ascii_code is
            when "00110000" => result := "0000"; -- '0'
            when "00110001" => result := "0001"; -- '1'
            when "00110010" => result := "0010"; -- '2'
            when "00110011" => result := "0011"; -- '3'
            when "00110100" => result := "0100"; -- '4'
            when "00110101" => result := "0101"; -- '5'
            when "00110110" => result := "0110"; -- '6'
            when "00110111" => result := "0111"; -- '7'
            when "00111000" => result := "1000"; -- '8'
            when "00111001" => result := "1001"; -- '9'
            when "01000001" => result := "1010"; -- 'A'
            when "01000010" => result := "1011"; -- 'B'
            when "01000011" => result := "1100"; -- 'C'
            when "01000100" => result := "1101"; -- 'D'
            when "01000101" => result := "1110"; -- 'E'
            when "01000110" => result := "1111"; -- 'F'
            when others    => result := "0000"; -- Default
        end case;

        return result;
    end function ASCII_TO_HEX;

end package body UartTypes;
