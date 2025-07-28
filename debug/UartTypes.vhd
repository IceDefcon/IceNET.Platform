library ieee;
use ieee.std_logic_1164.all;

package UartTypes is

    subtype UART_ASCII is std_logic_vector(6 downto 0);
    type UART_LOG_ID is array (0 to 1) of std_logic_vector(3 downto 0);
    type UART_LOG_KEY is array (0 to 1) of std_logic_vector(3 downto 0);
    type UART_LOG_DATA is array (0 to 3) of std_logic_vector(3 downto 0);

    function CONVERT_TO_ASCII
    (
        hex_code : std_logic_vector(3 downto 0)
    )
    return UART_ASCII;

    constant ASCII_R    : std_logic_vector(6 downto 0) := "1010010"; -- 0x52
    constant ASCII_COLON: std_logic_vector(6 downto 0) := "0111010"; -- 0x3A

    constant ASCII_LF    : std_logic_vector(6 downto 0) := "0001010";
    constant ASCII_CR    : std_logic_vector(6 downto 0) := "0001100";
    constant ASCII_DOT   : std_logic_vector(6 downto 0) := "0101110";
    constant ASCII_SPACE : std_logic_vector(6 downto 0) := "0100000";

end package UartTypes;

package body UartTypes is

    function CONVERT_TO_ASCII
    (
        hex_code : std_logic_vector(3 downto 0)
    )
    return UART_ASCII is variable result : UART_ASCII;
    begin
        case hex_code is
            when "0000" => result := "0110000"; -- ASCII '0'
            when "0001" => result := "0110001"; -- ASCII '1'
            when "0010" => result := "0110010"; -- ASCII '2'
            when "0011" => result := "0110011"; -- ASCII '3'
            when "0100" => result := "0110100"; -- ASCII '4'
            when "0101" => result := "0110101"; -- ASCII '5'
            when "0110" => result := "0110110"; -- ASCII '6'
            when "0111" => result := "0110111"; -- ASCII '7'
            when "1000" => result := "0111000"; -- ASCII '8'
            when "1001" => result := "0111001"; -- ASCII '9'
            when "1010" => result := "1000001"; -- ASCII 'A'
            when "1011" => result := "1000010"; -- ASCII 'B'
            when "1100" => result := "1000011"; -- ASCII 'C'
            when "1101" => result := "1000100"; -- ASCII 'D'
            when "1110" => result := "1000101"; -- ASCII 'E'
            when "1111" => result := "1000110"; -- ASCII 'F'
            when others => result := "0000000"; -- Default (undefined input)
        end case;

        return result;
    end function CONVERT_TO_ASCII;

end package body UartTypes;
