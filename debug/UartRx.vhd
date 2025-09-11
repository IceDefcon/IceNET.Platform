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

    UART_DEBUG  : out std_logic_vector(5 downto 0)
);
end UartRx;

architecture rtl of UartRx is

---------------------------------------------------------------------------------------------------
-- Constand definitions
---------------------------------------------------------------------------------------------------
constant PROCESS_SIZE : integer := 2048;
constant EDGE_COUNT_OFFSET : integer range 0 to 4 := 2;
constant EDGE_OFFSET : integer range 0 to 4 := 2;
constant EDGE_SYNC : integer range 0 to 4 := 2;

constant BIT_BAUD : integer range 0 to 64 := 25; -- 25*20ns ---> 2M Baud
constant BIT_START : integer range 0 to 64 := 15;               -- 15
constant BIT_0 : integer range 0 to PROCESS_SIZE := BIT_START + BIT_BAUD; -- 40
constant BIT_1 : integer range 0 to PROCESS_SIZE := BIT_0 + BIT_BAUD;     -- 65
constant BIT_2 : integer range 0 to PROCESS_SIZE := BIT_1 + BIT_BAUD;     -- 90
constant BIT_3 : integer range 0 to PROCESS_SIZE := BIT_2 + BIT_BAUD;     -- 115
constant BIT_4 : integer range 0 to PROCESS_SIZE := BIT_3 + BIT_BAUD;     -- 140
constant BIT_5 : integer range 0 to PROCESS_SIZE := BIT_4 + BIT_BAUD;     -- 165
constant BIT_6 : integer range 0 to PROCESS_SIZE := BIT_5 + BIT_BAUD;     -- 190
constant BIT_7 : integer range 0 to PROCESS_SIZE := BIT_6 + BIT_BAUD;     -- 215
constant BIT_STOP : integer range 0 to PROCESS_SIZE := BIT_7 + BIT_BAUD;  -- 240
constant BIT_NEXT : integer range 0 to PROCESS_SIZE := BIT_STOP + BIT_BAUD;   -- 265

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

signal fpga_uart_rx_s1 : std_logic := '0';
signal fpga_uart_rx_s2 : std_logic := '0';
signal fpga_uart_rx_d1 : std_logic := '0';
signal fpga_uart_rx_d2 : std_logic := '0';

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
-- Components Declaration
---------------------------------------------------------------------------------------------------
component UART_FIFO
port
(
    aclr : IN STD_LOGIC ;
    clock : IN STD_LOGIC ;
    -- IN
    data : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdreq : IN STD_LOGIC ;
    wrreq : IN STD_LOGIC ;
    -- OUT
    empty : OUT STD_LOGIC ;
    full : OUT STD_LOGIC ;
    q : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
);
end component;

begin

state_machine_process:
process(CLOCK, RESET)
begin
    if RESET = '1' then
        ---------------------------------------------------------------------------------------------------
        -- RESET Values
        ---------------------------------------------------------------------------------------------------
        uart_state <= UART_IDLE;
        symbol_byte <= (others => '0');
        symbol_process_timer <= 0;
        fpga_uart_rx_s1 <= '1';
        fpga_uart_rx_s2 <= '1';
        fpga_uart_rx_d1 <= '1';
        fpga_uart_rx_d2 <= '1';
        READ_BUSY <= '0';
    elsif rising_edge(CLOCK) then
        ---------------------------------------------------------------------------------------------------
        -- Avoid Latches
        ---------------------------------------------------------------------------------------------------
        uart_fifo_wr_en <= '0';
        edge_detected <= '0';
        symbol_trigger <= '0';
        uart_debug_vector(1) <= '0';

        ---------------------------------------------------------------------------------------------------
        -- Edge Sync
        ---------------------------------------------------------------------------------------------------
        fpga_uart_rx_s1 <= FPGA_UART_RX;
        fpga_uart_rx_s2 <= fpga_uart_rx_s1;
        uart_debug_vector(0) <= fpga_uart_rx_s1;

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
                fpga_uart_rx_d1 <= '1';
                fpga_uart_rx_d2 <= '1';
                if fpga_uart_rx_s2 = '0' then
                    READ_BUSY <= '1';
                    symbol_process_timer <= EDGE_SYNC; -- Edge Offset + Sync UART Delay
                    uart_state <= UART_PROCESS_SYMBOL;
                end if;

            ---------------------------------------------------------------------------------------------------
            -- START PROCESS
            ---------------------------------------------------------------------------------------------------
            when UART_NEXT_SYMBOL =>
                uart_debug_vector(5 downto 2) <= "0000";
                uart_debug_vector(1) <= '0';
                fpga_uart_rx_d1 <= fpga_uart_rx_s2;
                fpga_uart_rx_d2 <= fpga_uart_rx_d1;
                edge_timer <= edge_timer + 1;
                symbol_process_timer <= BIT_START + edge_error_correction;
                uart_state <= UART_PROCESS_SYMBOL;

            when UART_PROCESS_SYMBOL =>
                if symbol_process_timer = PROCESS_SIZE then
                else
                    if symbol_process_timer = BIT_START then
                        uart_debug_vector(5 downto 2) <= "0001";
                        ---------------------------------------------------------------------------------------------------
                        -- START BIT
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                    elsif symbol_process_timer = BIT_0 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0010";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 0
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(0) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_1 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0011";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 1
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(1) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_2 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0100";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 2
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(2) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_3 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0101";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 3
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(3) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_4 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0110";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 4
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(4) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_5 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "0111";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 5
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(5) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_6 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1000";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 6
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(6) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_7 + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1001";
                        ---------------------------------------------------------------------------------------------------
                        -- BIT 7
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                        symbol_byte(7) <= fpga_uart_rx_s2;

                    elsif symbol_process_timer = BIT_STOP + edge_error_correction - EDGE_OFFSET then
                        uart_debug_vector(5 downto 2) <= "1010";
                        ---------------------------------------------------------------------------------------------------
                        -- STOP BIT
                        ---------------------------------------------------------------------------------------------------
                        uart_debug_vector(1) <= '1';
                    elsif symbol_process_timer = BIT_NEXT + edge_error_correction - EDGE_OFFSET then

                        if fpga_uart_rx_s2 = '0' then
                            uart_debug_vector(5 downto 2) <= "1011";
                            uart_debug_vector(1) <= '1';
                            ---------------------------------------------------------------------------------------------------
                            -- ANOTHER START BIT
                            ---------------------------------------------------------------------------------------------------
                            uart_fifo_wr_en <= '1';
                            uart_state <= UART_NEXT_SYMBOL;
                            uart_fifo_data_I <= "0" & symbol_byte(6 downto 0);
                        else
                            uart_debug_vector(5 downto 2) <= "1100";
                            uart_debug_vector(1) <= '0';
                            ---------------------------------------------------------------------------------------------------
                            -- TRANSMISSION OVER
                            ---------------------------------------------------------------------------------------------------
                            uart_fifo_wr_en <= '1';
                            uart_state <= UART_PROCESS_SYMBOL_COMPLETE;
                            uart_fifo_data_I <= "0" & symbol_byte(6 downto 0);
                        end if;
                    else
                        uart_debug_vector(1) <= '0';
                    end if;

                    ---------------------------------------------------------------------------------------------------
                    -- Symbol Process Timer
                    ---------------------------------------------------------------------------------------------------
                    symbol_process_timer <= symbol_process_timer + 1;

                    ---------------------------------------------------------------------------------------------------
                    -- Edge Detector
                    ---------------------------------------------------------------------------------------------------
                    fpga_uart_rx_d1 <= fpga_uart_rx_s2;
                    fpga_uart_rx_d2 <= fpga_uart_rx_d1;

                    if fpga_uart_rx_d1 /= fpga_uart_rx_d2 then
                        ---------------------------------------------------------------------------------------------------
                        -- Ignore First Edge
                        ---------------------------------------------------------------------------------------------------
                        if symbol_process_timer > BIT_START then
                            edge_detected <= '1';
                            if edge_timer > (BIT_BAUD - EDGE_COUNT_OFFSET) then
                                edge_error_correction <= (edge_error_correction + ((edge_timer + EDGE_COUNT_OFFSET) mod BIT_BAUD));
                            end if;
                        end if;
                    end if;

                    if edge_detected = '1' then
                        edge_timer <= 0;
                    else
                        edge_timer <= edge_timer + 1;
                    end if;

                end if;

            ---------------------------------------------------------------------------------------------------
            -- WRITE TO FIFO
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

READ_ENABLE <= symbol_trigger;

UART_FIFO_inst: UART_FIFO
port map
(
    aclr  => RESET,
    clock => CLOCK,
    -- IN
    data  => uart_fifo_data_I,
    rdreq => uart_fifo_rd_en,
    wrreq => uart_fifo_wr_en,
    -- OUT
    empty => uart_fifo_empty,
    full  => uart_fifo_full,
    q     => uart_fifo_data_O
);

UART_DEBUG <= uart_debug_vector;

end architecture;
