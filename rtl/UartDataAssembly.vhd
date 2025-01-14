library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.Types.all;

entity UartDataAssembly is
port
(
    CLOCK_50MHz : in std_logic;

    UART_LOG_MESSAGE_ID : in UART_LOG_ID;
    UART_LOG_MESSAGE_DATA : in UART_LOG_DATA;

    WRITE_ENABLE : out std_logic;
    WRITE_SYMBOL : out std_logic_vector(6 downto 0);

    WRITE_BUSY : in std_logic
);
end UartDataAssembly;

architecture rtl of UartDataAssembly is

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
    WRITE_IDLE,
    WRITE_INIT,
    WRITE_CONFIG,
    WRITE_WAIT,
    WRITE_TRANSFER,
    WRITE_CHECK,
    WRITE_DONE
);
signal uart_state: STATE := WRITE_IDLE;
signal delay_timer : std_logic_vector(25 downto 0) := (others => '0');
signal uart_length : integer := 0;
signal uart_byte : integer := 0;

begin

    uart_process:
    process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case uart_state is
                when WRITE_IDLE =>
                    if delay_timer = "10111110101111000001111111" then
                        delay_timer <= (others => '0');
                        uart_state <= WRITE_INIT;
                    else
                        delay_timer <= delay_timer + '1';
                    end if;

                when WRITE_INIT =>
                    uart_length <= 10;
                    uart_tx(0) <= "1000100";
                    uart_tx(1) <= "1000101";
                    uart_tx(2) <= "1000001";
                    uart_tx(3) <= "1000100";
                    uart_tx(4) <= "0100000";
                    uart_tx(5) <= "1000011";
                    uart_tx(6) <= "0110000";
                    uart_tx(7) <= "1000100";
                    uart_tx(8) <= "1000101";
                    uart_tx(9) <= "0001010";
                    uart_state <= WRITE_CONFIG;

                when WRITE_CONFIG =>
                    WRITE_ENABLE <= '0';
                    if uart_byte = uart_length then
                        uart_byte <= 0;
                        uart_state <= WRITE_DONE;
                    else
                        uart_state <= WRITE_WAIT;
                    end if;

                when WRITE_WAIT =>
                    uart_state <= WRITE_TRANSFER;

                when WRITE_TRANSFER =>
                    if WRITE_BUSY = '0' then
                        WRITE_SYMBOL <= uart_tx(uart_byte);
                        uart_byte <= uart_byte + 1;
                        uart_state <= WRITE_CHECK;
                    end if;

                when WRITE_CHECK =>
                    WRITE_ENABLE <= '1';
                    uart_state <= WRITE_CONFIG;

                when WRITE_DONE =>
                    uart_state <= WRITE_IDLE;

                when others =>
                    uart_state <= WRITE_IDLE;

            end case;
        end if;
    end process;

end architecture;
