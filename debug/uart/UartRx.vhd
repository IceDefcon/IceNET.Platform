library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;

entity UartRx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_VALID : out std_logic;
    READ_SYMBOL : out std_logic_vector(7 downto 0);
    READ_BUSY : out std_logic;

    SYNCED_UART_RX : in std_logic
);
end UartRx;

architecture rtl of UartRx is

constant bit_baud : integer range 0 to 128 := 25; -- 25*20ns ---> 2M Baud @ 50Mhz
constant bit_start : integer range 0 to 128 := 15;
constant bit_0 : integer range 0 to 4096 := bit_start + bit_baud;
constant bit_1 : integer range 0 to 4096 := bit_0 + bit_baud;
constant bit_2 : integer range 0 to 4096 := bit_1 + bit_baud;
constant bit_3 : integer range 0 to 4096 := bit_2 + bit_baud;
constant bit_4 : integer range 0 to 4096 := bit_3 + bit_baud;
constant bit_5 : integer range 0 to 4096 := bit_4 + bit_baud;
constant bit_6 : integer range 0 to 4096 := bit_5 + bit_baud;
constant bit_7 : integer range 0 to 4096 := bit_6 + bit_baud;
constant bit_stop : integer range 0 to 4096 := bit_7 + bit_baud;

type SYMBOL_SM is
(
    SYMBOL_IDLE,
    SYMBOL_PROCESS,
    SYMBOL_READY,
    SYMBOL_DONE
);
signal symbol_state: SYMBOL_SM := SYMBOL_IDLE;

signal symbol_byte : std_logic_vector(7 downto 0) := (others => '0');
signal symbol_process_timer : integer range 0 to 4096 := 0;
signal symbol_trigger : std_logic := '0';

signal uart_rx_edge : std_logic := '1';

signal edge_timer : integer range 0 to 4096 := 0;
signal edge_error : integer range 0 to 4096 := 0;

begin

state_machine_process:
process(CLOCK, RESET)
begin
    ---------------------------------------------------------------------------------------------------
    -- RESET VARIABLES
    ---------------------------------------------------------------------------------------------------
    if RESET = '1' then
        uart_rx_edge <= '1';
        edge_timer <= 0;
        edge_error <= 0;
        symbol_state <= SYMBOL_IDLE;
        symbol_byte <= (others => '0');
        symbol_process_timer <= 0;
        symbol_trigger <= '0';
        READ_BUSY <= '0';
        READ_VALID <= '0';
        READ_SYMBOL <= (others => '0');
    elsif rising_edge(CLOCK) then
        ---------------------------------------------------------------------------------------------------
        -- RUNTIME RESET
        ---------------------------------------------------------------------------------------------------

        ---------------------------------------------------------------------------------------------------
        -- STATE MACHINE
        ---------------------------------------------------------------------------------------------------
        case symbol_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_IDLE =>
                edge_error <= 0;
                if SYNCED_UART_RX = '0' then
                    symbol_state <= SYMBOL_PROCESS;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_PROCESS =>
                if symbol_process_timer = 4096 then
                else
                    if symbol_process_timer = bit_start + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT START
                        ---------------------------------------------------------------------------------------------------

                    elsif symbol_process_timer = bit_0 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 0
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(0) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_1 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 1
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(1) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_2 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 2
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(2) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_3 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 3
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(3) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_4 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 4
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(4) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_5 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 5
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(5) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_6 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 6
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(6) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_7 + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 7
                        ---------------------------------------------------------------------------------------------------
                        symbol_byte(7) <= SYNCED_UART_RX;

                    elsif symbol_process_timer = bit_stop + edge_error - 2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT STOP
                        ---------------------------------------------------------------------------------------------------
                        symbol_state <= SYMBOL_READY;

                    end if;
                    ---------------------------------------------------------------------------------------------------
                    -- SYMBOL COUNTER
                    ---------------------------------------------------------------------------------------------------
                    symbol_process_timer <= symbol_process_timer + 1;

                    ---------------------------------------------------------------------------------------------------
                    -- Edge Detector
                    ---------------------------------------------------------------------------------------------------
                    uart_rx_edge <= SYNCED_UART_RX;

                    ---------------------------------------------------------------------------------------------------
                    -- Ignore First Edge
                    ---------------------------------------------------------------------------------------------------
                    if uart_rx_edge /= SYNCED_UART_RX then
                        if symbol_process_timer < bit_0 then
                            if edge_timer > bit_baud - 2 then
                                edge_error <= (edge_error + ((edge_timer + 2) mod bit_baud));
                            end if;
                            edge_timer <= 0;
                        else
                            if edge_timer > bit_baud - 2 then
                                edge_error <= (edge_error + ((edge_timer + 1) mod bit_baud));
                            end if;
                            edge_timer <= 0;
                        end if;
                    else
                        edge_timer <= edge_timer + 1;
                    end if;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_READY =>
                symbol_trigger <= '1';
                symbol_state <= SYMBOL_DONE;
                READ_SYMBOL <= symbol_byte;

            ---------------------------------------------------------------------------------------------------
            -- DONE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_DONE =>
                edge_timer <= 0;
                symbol_trigger <= '0';
                symbol_byte <= (others => '0');
                symbol_process_timer <= 0;
                symbol_state <= SYMBOL_IDLE;

            when others =>
                symbol_state <= SYMBOL_IDLE;

        end case;
        ---------------------------------------------------------------------------------------------------
        -- VALID
        ---------------------------------------------------------------------------------------------------
        READ_VALID <= symbol_trigger;

    end if;
end process;

end architecture;
