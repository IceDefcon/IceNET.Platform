library ieee;
use ieee.std_logic_1164.all;

package Types is

    -- Type Definitions
    type UART_LOG_MESSAGE is array (0 to 7) of std_logic_vector(6 downto 0);
    type UART_LOG_DATA is array (0 to 3) of std_logic_vector(6 downto 0);
    type UART_LOG_ID is array (0 to 1) of std_logic_vector(6 downto 0);

    -- Function Declaration
    function CONCATENATE_LOG(
        log_id : UART_LOG_ID;
        log_data : UART_LOG_DATA
    ) return UART_LOG_MESSAGE;

    -- ASCII Constants
    constant ASCII_0    : std_logic_vector(6 downto 0) := "0110000"; -- 0x30
    constant ASCII_1    : std_logic_vector(6 downto 0) := "0110001"; -- 0x31
    constant ASCII_2    : std_logic_vector(6 downto 0) := "0110010"; -- 0x32
    constant ASCII_3    : std_logic_vector(6 downto 0) := "0110011"; -- 0x33
    constant ASCII_4    : std_logic_vector(6 downto 0) := "0110100"; -- 0x34
    constant ASCII_5    : std_logic_vector(6 downto 0) := "0110101"; -- 0x35
    constant ASCII_6    : std_logic_vector(6 downto 0) := "0110110"; -- 0x36
    constant ASCII_7    : std_logic_vector(6 downto 0) := "0110111"; -- 0x37
    constant ASCII_8    : std_logic_vector(6 downto 0) := "0111000"; -- 0x38
    constant ASCII_9    : std_logic_vector(6 downto 0) := "0111001"; -- 0x39
    constant ASCII_A    : std_logic_vector(6 downto 0) := "1000001"; -- 0x41
    constant ASCII_B    : std_logic_vector(6 downto 0) := "1000010"; -- 0x42
    constant ASCII_C    : std_logic_vector(6 downto 0) := "1000011"; -- 0x43
    constant ASCII_D    : std_logic_vector(6 downto 0) := "1000100"; -- 0x44
    constant ASCII_E    : std_logic_vector(6 downto 0) := "1000101"; -- 0x45
    constant ASCII_F    : std_logic_vector(6 downto 0) := "1000110"; -- 0x46
    constant ASCII_LF   : std_logic_vector(6 downto 0) := "0001010"; -- 0x0A
    constant ASCII_CR   : std_logic_vector(6 downto 0) := "0001100"; -- 0x0D
    constant ASCII_DOT  : std_logic_vector(6 downto 0) := "0101110"; -- 0x2E
    constant ASCII_SPACE: std_logic_vector(6 downto 0) := "0100000"; -- 0x20

end package Types;

package body Types is

    -- Function Definition
    function CONCATENATE_LOG
    (
        log_id : UART_LOG_ID;
        log_data : UART_LOG_DATA
    )
    return UART_LOG_MESSAGE is variable result : UART_LOG_MESSAGE;
    begin

        -- Insert log_id into the first 2 elements
        result(0) := log_id(0);
        result(1) := log_id(1);
        result(2) := ASCII_SPACE;

        -- Insert log_data into the next 6 elements
        for i in 0 to 3 loop
            result(i + 3) := log_data(i);
        end loop;

        result(7) := ASCII_LF;

        return result;
    end function CONCATENATE_LOG;

end package body Types;
