library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;

entity UartTx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    WRITE_ENABLE : in std_logic;
    WRITE_SYMBOL : in std_logic_vector(7 downto 0);
    WRITE_BUSY : out std_logic;

    SYNCED_UART_TX : out std_logic
);
end UartTx;

architecture rtl of UartTx is

constant bit_baud : integer range 0 to 128 := 25; -- 25*20ns ---> 2M Baud @ 50Mhz
constant bit_start : integer range 0 to 128 := 0; -- 0
constant bit_0 : integer range 0 to 4096 := bit_start + bit_baud;   -- 25
constant bit_1 : integer range 0 to 4096 := bit_0 + bit_baud;       -- 50
constant bit_2 : integer range 0 to 4096 := bit_1 + bit_baud;       -- 75
constant bit_3 : integer range 0 to 4096 := bit_2 + bit_baud;       -- 125
constant bit_4 : integer range 0 to 4096 := bit_3 + bit_baud;       -- 150
constant bit_5 : integer range 0 to 4096 := bit_4 + bit_baud;       -- 175
constant bit_6 : integer range 0 to 4096 := bit_5 + bit_baud;       -- 200
constant bit_7 : integer range 0 to 4096 := bit_6 + bit_baud;       -- 225
constant bit_stop : integer range 0 to 4096 := bit_7 + bit_baud;    -- 250
constant bit_next : integer range 0 to 4096 := bit_stop + bit_baud; -- 275

signal symbol_process_timer : integer range 0 to 4096 := 0;

type SYMBOL_SM is
(
    SYMBOL_IDLE,
    SYMBOL_PROCESS,
    SYMBOL_DONE
);
signal symbol_state: SYMBOL_SM := SYMBOL_IDLE;

signal uart_tx : std_logic := '1';

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
        WRITE_BUSY <= '0';
        uart_tx <= '1';
    elsif rising_edge(CLOCK) then
        ---------------------------------------------------------------------------------------------------
        -- Avoid Latches
        ---------------------------------------------------------------------------------------------------


        ---------------------------------------------------------------------------------------------------
        -- State Machine
        ---------------------------------------------------------------------------------------------------
        case symbol_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_IDLE =>
                if WRITE_ENABLE = '1' then
                    WRITE_BUSY <= '1';
                    symbol_state <= SYMBOL_PROCESS;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_PROCESS =>
                if symbol_process_timer = 4096 then
                else
                    if symbol_process_timer = bit_start then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT START
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= '0';

                    elsif symbol_process_timer = bit_0 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 0
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(0);

                    elsif symbol_process_timer = bit_1 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 1
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(1);

                    elsif symbol_process_timer = bit_2 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 2
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(2);

                    elsif symbol_process_timer = bit_3 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 3
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(3);

                    elsif symbol_process_timer = bit_4 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 4
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(4);

                    elsif symbol_process_timer = bit_5 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 5
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(5);

                    elsif symbol_process_timer = bit_6 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 6
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(6);

                    elsif symbol_process_timer = bit_7 then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 7
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= WRITE_SYMBOL(7);

                    elsif symbol_process_timer = bit_stop then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT STOP
                        ---------------------------------------------------------------------------------------------------
                        uart_tx <= '1';

                    elsif symbol_process_timer = bit_next then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT NEXT
                        ---------------------------------------------------------------------------------------------------
                        symbol_state <= SYMBOL_DONE;

                    end if;

                    symbol_process_timer <= symbol_process_timer + 1;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- DONE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_DONE =>
                WRITE_BUSY <= '0';
                symbol_process_timer <= 0;
                symbol_state <= SYMBOL_IDLE;

            when others =>
                symbol_state <= SYMBOL_IDLE;

        end case;

        SYNCED_UART_TX <= uart_tx;

    end if;
end process;

end architecture;
