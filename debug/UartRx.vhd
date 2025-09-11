library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartRx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_ENABLE : out std_logic;
    READ_SYMBOL : out std_logic_vector(6 downto 0);
    READ_BUSY : out std_logic;

    FPGA_UART_RX : in std_logic;
    UART_DEBUG : out std_logic_vector(5 downto 0)
);
end UartRx;

architecture rtl of UartRx is

constant bit_baud : integer range 0 to 128 := 100; -- 100*5ns ---> 2M Baud @ 200Mhz
constant bit_start : integer range 0 to 128 := 60; -- 60
constant bit_0 : integer range 0 to 2048 := bit_start + bit_baud; -- 160
constant bit_1 : integer range 0 to 2048 := bit_0 + bit_baud;     -- 260
constant bit_2 : integer range 0 to 2048 := bit_1 + bit_baud;     -- 360
constant bit_3 : integer range 0 to 2048 := bit_2 + bit_baud;     -- 460
constant bit_4 : integer range 0 to 2048 := bit_3 + bit_baud;     -- 560
constant bit_5 : integer range 0 to 2048 := bit_4 + bit_baud;     -- 660
constant bit_6 : integer range 0 to 2048 := bit_5 + bit_baud;     -- 760
constant bit_7 : integer range 0 to 2048 := bit_6 + bit_baud;     -- 860
constant bit_stop : integer range 0 to 2048 := bit_7 + bit_baud;  -- 960

signal symbol_byte : std_logic_vector(7 downto 0) := (others => '0');
signal symbol_process_timer : integer range 0 to 2048 := 0;
signal symbol_trigger : std_logic := '0';

type SYMBOL_SM is
(
    SYMBOL_IDLE,
    SYMBOL_PROCESS,
    SYMBOL_SYMBOL_READY,
    SYMBOL_DONE
);
signal symbol_state: SYMBOL_SM := SYMBOL_IDLE;

signal uart_rx_reg : std_logic := '1';
signal uart_rx_sync : std_logic := '1';


signal debug_uart_vector : std_logic_vector(5 downto 0) := (others => '0');

begin

state_machine_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        ---------------------------------------------------------------------------------------------------
        -- RESET Values
        ---------------------------------------------------------------------------------------------------
        symbol_state <= SYMBOL_IDLE;
        symbol_process_timer <= 0;
        READ_BUSY <= '0';
    elsif rising_edge(CLOCK) then
        ---------------------------------------------------------------------------------------------------
        -- Avoid Latches
        ---------------------------------------------------------------------------------------------------
        symbol_trigger <= '0';
        debug_uart_vector(1) <= '0';

        ---------------------------------------------------------------------------------------------------
        -- UART Sync
        ---------------------------------------------------------------------------------------------------
        uart_rx_reg <= FPGA_UART_RX;
        uart_rx_sync <= uart_rx_reg;
        debug_uart_vector(0) <= uart_rx_reg;

        ---------------------------------------------------------------------------------------------------
        -- State Machine
        ---------------------------------------------------------------------------------------------------
        case symbol_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_IDLE =>
                debug_uart_vector(5 downto 2) <= "1111";
                if uart_rx_sync = '0' then
                    symbol_state <= SYMBOL_PROCESS;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_PROCESS =>
                if symbol_process_timer = 2048 then
                else
                    if symbol_process_timer = bit_start then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT START
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0001";
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_0 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 0
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0010";
                        symbol_byte(0) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_1 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 1
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0011";
                        symbol_byte(1) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 2
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0100";
                        symbol_byte(2) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_3 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 3
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0101";
                        symbol_byte(3) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_4 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 4
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0110";
                        symbol_byte(4) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_5 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 5
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "0111";
                        symbol_byte(5) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_6 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 6
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "1000";
                        symbol_byte(6) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_7 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 7
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "1001";
                        symbol_byte(7) <= uart_rx_sync;
                        debug_uart_vector(1) <= '1';

                    elsif symbol_process_timer = bit_stop then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT STOP
                        ---------------------------------------------------------------------------------------------------
                        debug_uart_vector(5 downto 2) <= "1010";
                        symbol_state <= SYMBOL_SYMBOL_READY;
                        debug_uart_vector(1) <= '1';

                    end if;

                    symbol_process_timer <= symbol_process_timer + 1;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_SYMBOL_READY =>
                debug_uart_vector(5 downto 2) <= "1011";
                symbol_trigger <= '1';
                symbol_state <= SYMBOL_DONE;
                READ_SYMBOL <= symbol_byte(6 downto 0);

            ---------------------------------------------------------------------------------------------------
            -- DONE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_DONE =>
                debug_uart_vector(5 downto 2) <= "1100";
                symbol_byte <= (others => '0');
                symbol_process_timer <= 0;
                symbol_state <= SYMBOL_IDLE;

            when others =>
                symbol_state <= SYMBOL_IDLE;

        end case;
    end if;
end process;

READ_ENABLE <= symbol_trigger;

UART_DEBUG <= debug_uart_vector;

end architecture;
