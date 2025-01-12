library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartDriver is
port
(
    CLOCK_50MHz : in std_logic;

    WRITE_ENABLE : out std_logic;
    WRITE_DATA : out std_logic_vector(6 downto 0);
    WRITE_LAST : out std_logic;

    WRITE_BUSY : in std_logic
);
end UartDriver;

architecture rtl of UartDriver is

constant ASCII_0 : std_logic_vector(6 downto 0) := "0110000"; -- 0x30
constant ASCII_1 : std_logic_vector(6 downto 0) := "0110001"; -- 0x31
constant ASCII_2 : std_logic_vector(6 downto 0) := "0110010"; -- 0x32
constant ASCII_3 : std_logic_vector(6 downto 0) := "0110011"; -- 0x33
constant ASCII_4 : std_logic_vector(6 downto 0) := "0110100"; -- 0x34
constant ASCII_5 : std_logic_vector(6 downto 0) := "0110101"; -- 0x35
constant ASCII_6 : std_logic_vector(6 downto 0) := "0110110"; -- 0x36
constant ASCII_7 : std_logic_vector(6 downto 0) := "0110111"; -- 0x37
constant ASCII_8 : std_logic_vector(6 downto 0) := "0111000"; -- 0x38
constant ASCII_9 : std_logic_vector(6 downto 0) := "0111001"; -- 0x39
constant ASCII_A : std_logic_vector(6 downto 0) := "1000001"; -- 0x41
constant ASCII_B : std_logic_vector(6 downto 0) := "1000010"; -- 0x42
constant ASCII_C : std_logic_vector(6 downto 0) := "1000011"; -- 0x43
constant ASCII_D : std_logic_vector(6 downto 0) := "1000100"; -- 0x44
constant ASCII_E : std_logic_vector(6 downto 0) := "1000101"; -- 0x45
constant ASCII_F : std_logic_vector(6 downto 0) := "1000110"; -- 0x46
constant ASCII_LF : std_logic_vector(6 downto 0) := "0001010"; -- 0x0A
constant ASCII_CR : std_logic_vector(6 downto 0) := "0001100";  -- 0x0D
constant ASCII_DOT : std_logic_vector(6 downto 0) := "0101110"; -- 0x2E
constant ASCII_SPACE : std_logic_vector(6 downto 0) := "0100000"; -- 0x20

type uart_type is array (0 to 15) of std_logic_vector(6 downto 0);
signal uart_tx : uart_type;

type STATE is
(
    TEST_IDLE,
    TEST_INIT,
    TEST_CONFIG,
    TEST_WAIT,
    TEST_TRANSFER,
    TEST_CHECK,
    TEST_DONE
);
signal uart_state: STATE := TEST_IDLE;
signal delay_timer : std_logic_vector(25 downto 0) := (others => '0');
signal uart_length : integer := 0;
signal uart_byte : integer := 0;

begin

    uart_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case uart_state is
                when TEST_IDLE =>
                    if delay_timer = "10111110101111000001111111" then
                        delay_timer <= (others => '0');
                        uart_state <= TEST_INIT;
                    else
                        delay_timer <= delay_timer + '1';
                    end if;

                when TEST_INIT =>
                    uart_length <= 10;
                    uart_tx(0) <= ASCII_D;
                    uart_tx(1) <= ASCII_E;
                    uart_tx(2) <= ASCII_A;
                    uart_tx(3) <= ASCII_D;
                    uart_tx(4) <= ASCII_SPACE;
                    uart_tx(5) <= ASCII_C;
                    uart_tx(6) <= ASCII_0;
                    uart_tx(7) <= ASCII_D;
                    uart_tx(8) <= ASCII_E;
                    uart_tx(9) <= ASCII_LF;
                    --uart_tx(3) <= ASCII_CR;
                    uart_state <= TEST_CONFIG;

                when TEST_CONFIG =>
                    WRITE_ENABLE <= '0';
                    if uart_byte = uart_length then
                        uart_byte <= 0;
                        uart_state <= TEST_DONE;
                    else
                        uart_state <= TEST_WAIT;
                    end if;

                when TEST_WAIT =>
                    uart_state <= TEST_TRANSFER;

                when TEST_TRANSFER =>
                    if WRITE_BUSY = '0' then
                        WRITE_DATA <= uart_tx(uart_byte);
                        WRITE_LAST <= '0';
                        uart_byte <= uart_byte + 1;
                        uart_state <= TEST_CHECK;
                    end if;

                when TEST_CHECK =>
                    WRITE_ENABLE <= '1';
                    uart_state <= TEST_CONFIG;

                when TEST_DONE =>
                    uart_state <= TEST_IDLE;

                when others =>
                    uart_state <= TEST_IDLE;

            end case;
        end if;
    end process;

end architecture;
