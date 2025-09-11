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

    FPGA_UART_RX : in std_logic
);
end UartRx;

architecture rtl of UartRx is

constant bit_baud : integer range 0 to 128 := 100; -- 100*5ns ---> 2M Baud
constant bit_start : integer range 0 to 64 := 60;                 -- 40
constant bit_0 : integer range 0 to 2048 := bit_start + bit_baud; -- 140
constant bit_1 : integer range 0 to 2048 := bit_0 + bit_baud;     -- 240
constant bit_2 : integer range 0 to 2048 := bit_1 + bit_baud;     -- 340
constant bit_3 : integer range 0 to 2048 := bit_2 + bit_baud;     -- 440
constant bit_4 : integer range 0 to 2048 := bit_3 + bit_baud;     -- 540
constant bit_5 : integer range 0 to 2048 := bit_4 + bit_baud;     -- 640
constant bit_6 : integer range 0 to 2048 := bit_5 + bit_baud;     -- 740
constant bit_7 : integer range 0 to 2048 := bit_6 + bit_baud;     -- 840
constant bit_stop : integer range 0 to 2048 := bit_7 + bit_baud;  -- 940

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

        ---------------------------------------------------------------------------------------------------
        -- State Machine
        ---------------------------------------------------------------------------------------------------
        case symbol_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_IDLE =>
                if FPGA_UART_RX = '0' then
                    symbol_state <= SYMBOL_PROCESS;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_PROCESS =>
                if symbol_process_timer = 2048 then
                else
                    if symbol_process_timer = bit_start then
                        --
                        --
                        --
                    elsif symbol_process_timer = bit_0 then

                        symbol_byte(0) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_1 then

                        symbol_byte(1) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_2 then

                        symbol_byte(2) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_3 then

                        symbol_byte(3) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_4 then

                        symbol_byte(4) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_5 then

                        symbol_byte(5) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_6 then

                        symbol_byte(6) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_7 then

                        symbol_byte(7) <= FPGA_UART_RX;

                    elsif symbol_process_timer = bit_stop then

                        symbol_state <= SYMBOL_SYMBOL_READY;

                    end if;

                    symbol_process_timer <= symbol_process_timer + 1;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_SYMBOL_READY =>
                symbol_trigger <= '1';
                symbol_state <= SYMBOL_DONE;
                READ_SYMBOL <= symbol_byte(6 downto 0);

            ---------------------------------------------------------------------------------------------------
            -- DONE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_DONE =>
                symbol_byte <= (others => '0');
                symbol_process_timer <= 0;
                symbol_state <= SYMBOL_IDLE;

            when others =>
                symbol_state <= SYMBOL_IDLE;

        end case;
    end if;
end process;

READ_ENABLE <= symbol_trigger;

end architecture;
