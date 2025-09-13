library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartOffloadController is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;

    UART_SYMBOL_READY : in std_logic;
    UART_SYMBOL_BYTE : in std_logic_vector(7 downto 0);
    UART_READ_BUSY : in std_logic;

    OFFLOAD_DATA_BYTE : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA_READY : out std_logic;

    FEEDBACK_DATA : out std_logic_vector(31 downto 0);
    FEEDBACK_TRIGGER : out std_logic
);
end UartOffloadController;

architecture rtl of UartOffloadController is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------
constant DOLAR_SYMBOL : std_logic_vector(7 downto 0) := "00100100"; -- 0x24
constant CONTROL_BYTES_NUMBER : integer := 2;

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

type UART_OFFLOAD_SM is
(
    UART_OFFLOAD_IDLE,
    UART_OFFLOAD_CHECK,
    UART_OFFLOAD_FIFO_WRITE,
    UART_OFFLOAD_FEEDBACK,
    UART_OFFLOAD_DONE
);
signal uart_offload_state: UART_OFFLOAD_SM := UART_OFFLOAD_IDLE;

signal uart_fifo_data_in : std_logic_vector(7 downto 0) := (others => '0');
signal uart_fifo_rd_req : std_logic := '0';
signal uart_fifo_wr_req : std_logic := '0';
signal uart_fifo_empty : std_logic := '0';
signal uart_fifo_full : std_logic := '0';
signal uart_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');

signal uart_symbol_data : std_logic_vector(7 downto 0) := (others => '0');
signal uart_symbol_count : integer range 0 to 4 := 0;

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------

component UART_FIFO
port
(
    aclr  : IN STD_LOGIC;
    clock : IN STD_LOGIC;
    data  : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdreq : IN STD_LOGIC;
    wrreq : IN STD_LOGIC;
    empty : OUT STD_LOGIC;
    full : OUT STD_LOGIC;
    q : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
);
end component;

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------
begin
------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

uart_offload_sm_process:
process(CLOCK)
begin
    if RESET = '1' then
        uart_offload_state <= UART_OFFLOAD_IDLE;
    elsif rising_edge(CLOCK) then
        ------------------------------------------------------------------------------------------------------------
        -- Anti-latch
        ------------------------------------------------------------------------------------------------------------
        uart_fifo_wr_req <= '0';
        uart_fifo_rd_req <= '0';
        FEEDBACK_TRIGGER <= '0';

        ------------------------------------------------------------------------------------------------------------
        -- Offload State Machine
        ------------------------------------------------------------------------------------------------------------
        case uart_offload_state is

            when UART_OFFLOAD_IDLE =>
                if UART_SYMBOL_READY = '1' then
                    uart_symbol_data <= UART_SYMBOL_BYTE;
                    uart_offload_state <= UART_OFFLOAD_CHECK;
                end if;

            when UART_OFFLOAD_CHECK =>
                if DOLAR_SYMBOL = uart_symbol_data then
                    uart_offload_state <= UART_OFFLOAD_FIFO_WRITE;
                else
                    uart_offload_state <= UART_OFFLOAD_IDLE;
                end if;

            when UART_OFFLOAD_FIFO_WRITE =>
                if CONTROL_BYTES_NUMBER = uart_symbol_count then
                    uart_offload_state <= UART_OFFLOAD_FEEDBACK;
                else
                    if UART_SYMBOL_READY = '1' then
                        uart_fifo_data_in <= UART_SYMBOL_BYTE;
                        uart_fifo_wr_req <= '1';
                        uart_symbol_count <= uart_symbol_count + 1;
                    end if;
                end if;

            when UART_OFFLOAD_FEEDBACK =>
                FEEDBACK_DATA <= x"FEEDC0DE";
                FEEDBACK_TRIGGER <= '1';
                uart_offload_state <= UART_OFFLOAD_DONE;

            when UART_OFFLOAD_DONE =>
                uart_symbol_count <= 0;
                uart_symbol_data <= (others => '0');
                uart_fifo_data_in  <= (others => '0');
                uart_offload_state <= UART_OFFLOAD_IDLE;

            when others =>
                uart_offload_state <= UART_OFFLOAD_IDLE;
        end case;
    end if;
end process;

UART_FIFO_module: UART_FIFO
port map
(
    aclr => RESET,
    clock => CLOCK,

    data => uart_fifo_data_in,
    rdreq => uart_fifo_rd_req,
    wrreq => uart_fifo_wr_req,

    empty => uart_fifo_empty,
    full => uart_fifo_full,
    q => uart_fifo_data_out
);

------------------------------------------------------------------------------------------------------------
-- End
------------------------------------------------------------------------------------------------------------
end architecture;
