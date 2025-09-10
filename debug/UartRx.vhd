library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity UartRx is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    READ_ENABLE : out std_logic;
    READ_SYMBOL : out std_logic_vector(6 downto 0);
    READ_BUSY : out std_logic;

    FPGA_UART_RX : in std_logic;

    UART_DEBUG  : out std_logic_vector(5 downto 0)
);
end UartRx;

architecture rtl of UartRx is

---------------------------------------------------------------------------------------------------
-- Constant definitions
---------------------------------------------------------------------------------------------------
constant PROCESS_SIZE : integer := 2048;
constant EDGE_COUNT_OFFSET : integer range 0 to 4 := 2;
constant EDGE_OFFSET : integer range 0 to 4 := 2;
constant EDGE_SYNC : integer range 0 to 4 := 2;

constant BIT_BAUD : integer range 0 to 128 := 25; -- example: 100 clocks per bit
constant BIT_START : integer range 0 to 128 := 15;
constant BIT_0 : integer range 0 to PROCESS_SIZE := BIT_START + BIT_BAUD;
constant BIT_1 : integer range 0 to PROCESS_SIZE := BIT_0 + BIT_BAUD;
constant BIT_2 : integer range 0 to PROCESS_SIZE := BIT_1 + BIT_BAUD;
constant BIT_3 : integer range 0 to PROCESS_SIZE := BIT_2 + BIT_BAUD;
constant BIT_4 : integer range 0 to PROCESS_SIZE := BIT_3 + BIT_BAUD;
constant BIT_5 : integer range 0 to PROCESS_SIZE := BIT_4 + BIT_BAUD;
constant BIT_6 : integer range 0 to PROCESS_SIZE := BIT_5 + BIT_BAUD;
constant BIT_7 : integer range 0 to PROCESS_SIZE := BIT_6 + BIT_BAUD;
constant BIT_STOP : integer range 0 to PROCESS_SIZE := BIT_7 + BIT_BAUD;
constant BIT_NEXT : integer range 0 to PROCESS_SIZE := BIT_STOP + BIT_BAUD;

---------------------------------------------------------------------------------------------------
-- Signals
---------------------------------------------------------------------------------------------------
signal symbol_byte : std_logic_vector(7 downto 0) := (others => '0');
signal symbol_process_timer : integer range 0 to PROCESS_SIZE := 0;
signal symbol_trigger : std_logic := '0';

type UART_STATE_MACHINE is
(
    UART_IDLE,
    UART_NEXT_SYMBOL,
    UART_PROCESS_SYMBOL,
    UART_PROCESS_SYMBOL_COMPLETE,
    UART_PROCESS_COMPLETE
);
signal uart_state: UART_STATE_MACHINE := UART_IDLE;

-- 2-stage synchronizer + previous value for edge detection
signal rx_sync_0 : std_logic := '1';
signal rx_sync_1 : std_logic := '1';
signal rx_prev   : std_logic := '1';

signal edge_detected : std_logic := '0';
signal edge_timer : integer range 0 to 1024 := 0;
signal edge_error_correction : integer range 0 to PROCESS_SIZE := 0;

signal uart_fifo_data_I : std_logic_vector(7 downto 0) := (others => '0');
signal uart_fifo_rd_en : std_logic := '0';
signal uart_fifo_wr_en : std_logic := '0';
signal uart_fifo_empty : std_logic := '0';
signal uart_fifo_full : std_logic := '0';
signal uart_fifo_data_O : std_logic_vector(7 downto 0) := (others => '0');

signal uart_debug_vector : std_logic_vector(5 downto 0) := (others => '0');
---------------------------------------------------------------------------------------------------
-- Components Declaration (kept commented)
---------------------------------------------------------------------------------------------------
--component UART_FIFO
--port
--(
--    aclr : IN STD_LOGIC ;
--    clock : IN STD_LOGIC ;
--    data : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
--    rdreq : IN STD_LOGIC ;
--    wrreq : IN STD_LOGIC ;
--    empty : OUT STD_LOGIC ;
--    full : OUT STD_LOGIC ;
--    q : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
--);
--end component;

begin

state_machine_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        ---------------------------------------------------------------------------------------------------
        -- RESET Values (explicit outputs set)
        ---------------------------------------------------------------------------------------------------
        uart_state <= UART_IDLE;
        symbol_byte <= (others => '0');
        symbol_process_timer <= 0;
        rx_sync_0 <= '1';
        rx_sync_1 <= '1';
        rx_prev <= '1';
        edge_detected <= '0';
        edge_timer <= 0;
        edge_error_correction <= 0;
        uart_fifo_wr_en <= '0';
        uart_fifo_rd_en <= '0';
        uart_fifo_data_I <= (others => '0');
        uart_debug_vector <= (others => '0');

        -- outputs
        READ_BUSY <= '0';
        --READ_ENABLE <= '0';
        READ_SYMBOL <= (others => '0');

    elsif rising_edge(CLOCK) then
        ---------------------------------------------------------------------------------------------------
        -- Default (avoid latches)
        ---------------------------------------------------------------------------------------------------
        uart_fifo_wr_en <= '0';
        edge_detected <= '0';
        symbol_trigger <= '0';
        uart_debug_vector(1) <= '0';

        ---------------------------------------------------------------------------------------------------
        -- 2-stage Edge Sync
        ---------------------------------------------------------------------------------------------------
        rx_sync_0 <= FPGA_UART_RX;
        rx_sync_1 <= rx_sync_0;
        uart_debug_vector(0) <= rx_sync_1;

        -- edge detection compared with previous sampled value
        if rx_sync_1 /= rx_prev then
            edge_detected <= '1';
        else
            edge_detected <= '0';
        end if;
        rx_prev <= rx_sync_1;

        ---------------------------------------------------------------------------------------------------
        -- State Machine
        ---------------------------------------------------------------------------------------------------
        case uart_state is

            ---------------------------------------------------------------------------------------------------
            -- IDLE
            ---------------------------------------------------------------------------------------------------
            when UART_IDLE =>
                uart_debug_vector(5 downto 2) <= "1111";
                edge_timer <= 0;
                edge_error_correction <= 0;
                -- Wait for start bit (line goes low)
                if rx_sync_1 = '0' then
                    READ_BUSY <= '1';
                    symbol_process_timer <= EDGE_SYNC; -- small sync delay
                    uart_state <= UART_PROCESS_SYMBOL;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS FOR NEXT SYMBOL
            ---------------------------------------------------------------------------------------------------
            when UART_NEXT_SYMBOL =>
                uart_debug_vector(5 downto 2) <= "0000";
                uart_debug_vector(1) <= '0';
                edge_timer <= edge_timer + 1;
                symbol_process_timer <= BIT_START + edge_error_correction;
                uart_state <= UART_PROCESS_SYMBOL;

            ---------------------------------------------------------------------------------------------------
            -- PROCESS SYMBOL
            ---------------------------------------------------------------------------------------------------
            when UART_PROCESS_SYMBOL =>
                -- guard to avoid timer wrap / infinite stall
                if symbol_process_timer < PROCESS_SIZE then

                    -- sample START
                    if symbol_process_timer = BIT_START then
                        uart_debug_vector(5 downto 2) <= "0001";
                        uart_debug_vector(1) <= '1';
                    elsif symbol_process_timer = BIT_0 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0010";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(0) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_1 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0011";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(1) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_2 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0100";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(2) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_3 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0101";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(3) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_4 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0110";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(4) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_5 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0111";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(5) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_6 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1000";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(6) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_7 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1001";
                        uart_debug_vector(1) <= '1';
                        symbol_byte(7) <= rx_sync_1;

                    elsif symbol_process_timer = BIT_STOP + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1010";
                        uart_debug_vector(1) <= '1';
                        -- stop bit sampled here (you may want to validate it's '1')

                    elsif symbol_process_timer = BIT_NEXT + edge_error_correction - EDGE_OFFSET then
                        -- decide if next start bit is present (continuous stream) or end
                        if rx_sync_1 = '0' then
                            uart_debug_vector(5 downto 2) <= "1011";
                            uart_debug_vector(1) <= '1';
                            uart_fifo_wr_en <= '1';
                            uart_state <= UART_NEXT_SYMBOL;
                            uart_fifo_data_I <= "0" & symbol_byte(6 downto 0); -- keep your original behaviour
                        else
                            uart_debug_vector(5 downto 2) <= "1100";
                            uart_debug_vector(1) <= '0';
                            uart_fifo_wr_en <= '1';
                            uart_state <= UART_PROCESS_SYMBOL_COMPLETE;
                            uart_fifo_data_I <= "0" & symbol_byte(6 downto 0);
                        end if;

                    else
                        uart_debug_vector(1) <= '0';
                    end if;

                    ---------------------------------------------------------------------------------------------------
                    -- increment timer
                    ---------------------------------------------------------------------------------------------------
                    symbol_process_timer <= symbol_process_timer + 1;

                    ---------------------------------------------------------------------------------------------------
                    -- Edge detector behavior & correction
                    ---------------------------------------------------------------------------------------------------
                    if edge_detected = '1' then
                        -- Only apply correction after initial start sampling window
                        if symbol_process_timer > BIT_START then
                            -- accumulate correction using measured edge period
                            if edge_timer > (BIT_BAUD - EDGE_COUNT_OFFSET) then
                                edge_error_correction <= edge_error_correction + ((edge_timer + EDGE_COUNT_OFFSET) mod BIT_BAUD);
                            end if;
                        end if;
                        edge_timer <= 0;
                    else
                        edge_timer <= edge_timer + 1;
                    end if;

                else
                    -- Timer overflow/recovery: reset to IDLE to avoid permanent stall
                    symbol_process_timer <= 0;
                    edge_error_correction <= 0;
                    READ_BUSY <= '0';
                    uart_state <= UART_IDLE;
                    uart_debug_vector(5 downto 2) <= "1111";
                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO: notify external logic
            ---------------------------------------------------------------------------------------------------
            when UART_PROCESS_SYMBOL_COMPLETE =>
                uart_debug_vector(5 downto 2) <= "1101";
                symbol_process_timer <= 0;
                symbol_trigger <= '1';
                READ_SYMBOL <= symbol_byte(6 downto 0);
                uart_state <= UART_PROCESS_COMPLETE;

            ---------------------------------------------------------------------------------------------------
            -- DATA PROCESS COMPLETE
            ---------------------------------------------------------------------------------------------------
            when UART_PROCESS_COMPLETE =>
                uart_debug_vector(5 downto 2) <= "1110";
                READ_BUSY <= '0';
                symbol_byte <= (others => '0');
                uart_state <= UART_IDLE;

            when others =>
                uart_state <= UART_IDLE;
        end case;
    end if;
end process;

-- Output handshake
READ_ENABLE <= symbol_trigger;

-- FIFO instance: uncomment and connect if you have a FIFO
--UART_FIFO_inst: UART_FIFO
--port map
--(
--    aclr  => RESET,
--    clock => CLOCK,
--    data  => uart_fifo_data_I,
--    rdreq => uart_fifo_rd_en,
--    wrreq => uart_fifo_wr_en,
--    empty => uart_fifo_empty,
--    full  => uart_fifo_full,
--    q     => uart_fifo_data_O
--);

UART_DEBUG <= uart_debug_vector;

end architecture;
