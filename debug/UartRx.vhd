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

    -- DEBUG OUTPUTS
    BIT_READY_OUT  : out std_logic;
    TEST_OUT : out std_logic;
    TEST_VECTOR_OUT : out std_logic_vector(7 downto 0)
);
end UartRx;

architecture rtl of UartRx is

constant EDGE_COUNT_OFFSET : integer range 0 to 4 := 2;
constant EDGE_OFFSET : integer range 0 to 4 := 2;
constant EDGE_SYNC : integer range 0 to 4 := 2;

constant bit_baud : integer range 0 to 64 := 25; -- 25*20ns ---> 2M Baud
constant bit_start : integer range 0 to 64 := 15;               -- 15
constant bit_0 : integer range 0 to 1024 := bit_start + bit_baud; -- 40
constant bit_1 : integer range 0 to 1024 := bit_0 + bit_baud;     -- 65
constant bit_2 : integer range 0 to 1024 := bit_1 + bit_baud;     -- 90
constant bit_3 : integer range 0 to 1024 := bit_2 + bit_baud;     -- 115
constant bit_4 : integer range 0 to 1024 := bit_3 + bit_baud;     -- 140
constant bit_5 : integer range 0 to 1024 := bit_4 + bit_baud;     -- 165
constant bit_6 : integer range 0 to 1024 := bit_5 + bit_baud;     -- 190
constant bit_7 : integer range 0 to 1024 := bit_6 + bit_baud;     -- 215
constant bit_stop : integer range 0 to 1024 := bit_7 + bit_baud;  -- 240
constant bit_run : integer range 0 to 1024 := bit_stop + bit_baud;   -- 265

signal symbol_byte : std_logic_vector(7 downto 0) := (others => '0');
signal symbol_process_timer : integer range 0 to 1024 := 0;
signal symbol_trigger : std_logic := '0';

type SYMBOL_SM is
(
    SYMBOL_IDLE,
    SYMBOL_NEXT,
    SYMBOL_PROCESS,
    SYMBOL_SYMBOL_READY,
    SYMBOL_DONE
);
signal symbol_state: SYMBOL_SM := SYMBOL_IDLE;

signal FPGA_UART_RX_s1 : std_logic := '0';
signal FPGA_UART_RX_s2 : std_logic := '0';

signal FPGA_UART_RX_d1 : std_logic := '0';
signal FPGA_UART_RX_d2 : std_logic := '0';
signal EDGE_DETECTED : std_logic := '0';
signal EDGE_COUNT : std_logic_vector(7 downto 0) := (others => '0');
signal EDGE_ERROR : integer range 0 to 1024 := 0;

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
        BIT_READY_OUT <= '0';
        EDGE_DETECTED <= '0';
        TEST_OUT <= '0';

        ---------------------------------------------------------------------------------------------------
        -- Edge Sync
        ---------------------------------------------------------------------------------------------------
        FPGA_UART_RX_s1 <= FPGA_UART_RX;
        FPGA_UART_RX_s2 <= FPGA_UART_RX_s1;

        ---------------------------------------------------------------------------------------------------
        -- State Machine
        ---------------------------------------------------------------------------------------------------
        case symbol_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_IDLE =>
                EDGE_COUNT <= (others => '0');
                EDGE_ERROR <= 0;
                FPGA_UART_RX_d1 <= '0';
                FPGA_UART_RX_d2 <= '0';
                if FPGA_UART_RX_s2 = '0' then
                    symbol_process_timer <= EDGE_OFFSET + EDGE_SYNC; -- Edge Offset + Sync UART Delay
                    symbol_state <= SYMBOL_PROCESS;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_NEXT =>
                FPGA_UART_RX_d1 <= '0';
                FPGA_UART_RX_d2 <= '0';
                EDGE_COUNT <= EDGE_COUNT + 1;
                symbol_process_timer <= bit_start + EDGE_ERROR;
                symbol_state <= SYMBOL_PROCESS;

            when SYMBOL_PROCESS =>
                if symbol_process_timer = 1024 then
                else
                    if symbol_process_timer = bit_start then
                        ---------------------------------------------------------------------------------------------------
                        -- START BIT
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                    elsif symbol_process_timer = bit_0 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 0
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(0) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_1 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 1
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(1) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_2 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 2
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(2) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_3 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 3
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(3) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_4 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 4
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(4) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_5 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 5
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(5) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_6 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 6
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(6) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_7 + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 7
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                        symbol_byte(7) <= FPGA_UART_RX_s2;

                    elsif symbol_process_timer = bit_stop + EDGE_ERROR - EDGE_OFFSET then
                        ---------------------------------------------------------------------------------------------------
                        -- STOP BIT
                        ---------------------------------------------------------------------------------------------------
                        BIT_READY_OUT <= '1';
                    elsif symbol_process_timer = bit_run + EDGE_ERROR - EDGE_OFFSET then

                        if FPGA_UART_RX_s2 = '0' then
                            ---------------------------------------------------------------------------------------------------
                            -- ANOTHER START BIT
                            ---------------------------------------------------------------------------------------------------
                            BIT_READY_OUT <= '1';
                            symbol_state <= SYMBOL_NEXT;
                            READ_SYMBOL <= symbol_byte(6 downto 0);
                        else
                            ---------------------------------------------------------------------------------------------------
                            -- TRANSMISSION OVER
                            ---------------------------------------------------------------------------------------------------
                            symbol_state <= SYMBOL_SYMBOL_READY;
                            READ_SYMBOL <= symbol_byte(6 downto 0);
                        end if;

                    end if;

                    ---------------------------------------------------------------------------------------------------
                    -- Symbol Process Timer
                    ---------------------------------------------------------------------------------------------------
                    symbol_process_timer <= symbol_process_timer + 1;

                    ---------------------------------------------------------------------------------------------------
                    -- Edge Detector
                    ---------------------------------------------------------------------------------------------------
                    FPGA_UART_RX_d1 <= FPGA_UART_RX_s2;
                    FPGA_UART_RX_d2 <= FPGA_UART_RX_d1;

                    if FPGA_UART_RX_d1 /= FPGA_UART_RX_d2 then
                        EDGE_DETECTED <= '1';
                        if to_integer(unsigned(EDGE_COUNT)) > 23 then
                            EDGE_ERROR <= (EDGE_ERROR + ((to_integer(unsigned(EDGE_COUNT)) + EDGE_COUNT_OFFSET) mod bit_baud));
                        end if;
                    end if;

                    if EDGE_DETECTED = '1' then
                        EDGE_COUNT <= (others => '0');
                        TEST_OUT <= '1';
                    else
                        EDGE_COUNT <= EDGE_COUNT + '1';
                    end if;

                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO
            ---------------------------------------------------------------------------------------------------
            when SYMBOL_SYMBOL_READY =>
                symbol_trigger <= '1';
                symbol_state <= SYMBOL_DONE;
                --READ_SYMBOL <= symbol_byte(6 downto 0);

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

TEST_VECTOR_OUT <= EDGE_COUNT;

end architecture;
