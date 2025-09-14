library ieee;
use ieee.std_logic_1164.all;

entity IrqController is
generic
(
    VECTOR_SIZE : integer := 10
);
Port
(
    CLOCK : in  std_logic;
    RESET : in  std_logic;

    TRIGGER : in std_logic;
    COMMAND : in std_logic_vector(7 downto 0);

    VECTOR_INTERRUPT : out std_logic_vector(VECTOR_SIZE - 1 downto 0);

    OFFLOAD_DATA_BYTE : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA_READY : out std_logic;

    FEEDBACK_DATA : out std_logic_vector(31 downto 0);
    FEEDBACK_TRIGGER : out std_logic
);
end entity IrqController;

architecture rtl of IrqController is

------------------------------------------------------------------------------------------------------------
-- Constants
------------------------------------------------------------------------------------------------------------
constant CONTROL_BYTES_NUMBER : integer := 4;

constant CMD_FIFO_DATA  : std_logic_vector(7 downto 0) := "00100100"; -- 0x24
constant CMD_UART_RESET : std_logic_vector(7 downto 0) := "00110000"; -- 0x30
constant CMD_OP_NORMAL  : std_logic_vector(7 downto 0) := "00110001"; -- 0x31
constant CMD_OP_UART_FD : std_logic_vector(7 downto 0) := "00110010"; -- 0x32
constant CMD_OP_UART_TD : std_logic_vector(7 downto 0) := "00110011"; -- 0x33
constant CMD_SCALING_00 : std_logic_vector(7 downto 0) := "00110100"; -- 0x34
constant CMD_SCALING_01 : std_logic_vector(7 downto 0) := "00110101"; -- 0x35
constant CMD_SCALING_10 : std_logic_vector(7 downto 0) := "00110110"; -- 0x36
constant CMD_SCALING_11 : std_logic_vector(7 downto 0) := "00110111"; -- 0x37
constant CMD_UHD_SOURCE : std_logic_vector(7 downto 0) := "00111000"; -- 0x38
constant CMD_GEN_SOURCE : std_logic_vector(7 downto 0) := "00111001"; -- 0x39
constant CMD_DEBUG_A : std_logic_vector(7 downto 0) := "01000001"; -- 0x41
constant CMD_DEBUG_B : std_logic_vector(7 downto 0) := "01000010"; -- 0x42
constant CMD_DEBUG_C : std_logic_vector(7 downto 0) := "01000011"; -- 0x43
constant CMD_DEBUG_D : std_logic_vector(7 downto 0) := "01000100"; -- 0x44
constant CMD_DEBUG_E : std_logic_vector(7 downto 0) := "01000101"; -- 0x45
constant CMD_DEBUG_F : std_logic_vector(7 downto 0) := "01000110"; -- 0x46

------------------------------------------------------------------------------------------------------------
-- Signals and Types
------------------------------------------------------------------------------------------------------------
type VECTOR_SM is
(
    VECTOR_IDLE,
    VECTOR_CHECK,       -- 0x24 :: ASCII "$"
    VECTOR_WRITE,       -- 0x24 :: ASCII "$"
    VECTOR_FEEDBACK,    -- 0x24 :: ASCII "$"
    VECTOR_UART_RESET,  -- 0x30 :: ASCII "0"
    VECTOR_OP_NORMAL,   -- 0x31 :: ASCII "1"
    VECTOR_OP_UART_FD,  -- 0x32 :: ASCII "2"
    VECTOR_OP_UART_TD,  -- 0x33 :: ASCII "3"
    VECTOR_SCALING_00,  -- 0x34 :: ASCII "4"
    VECTOR_SCALING_01,  -- 0x35 :: ASCII "5"
    VECTOR_SCALING_10,  -- 0x36 :: ASCII "6"
    VECTOR_SCALING_11,  -- 0x37 :: ASCII "7"
    VECTOR_UHD_SOURCE,  -- 0x38 :: ASCII "8"
    VECTOR_GEN_SOURCE,  -- 0x39 :: ASCII "9"
    VECTOR_DEBUG_A,     -- 0x41 :: ASCII "A"
    VECTOR_DEBUG_B,     -- 0x42 :: ASCII "B"
    VECTOR_DEBUG_C,     -- 0x43 :: ASCII "C"
    VECTOR_DEBUG_D,     -- 0x44 :: ASCII "D"
    VECTOR_DEBUG_E,     -- 0x45 :: ASCII "E"
    VECTOR_DEBUG_F,     -- 0x46 :: ASCII "F"
    VECTOR_DONE
);
signal interrupt_vector_state: VECTOR_SM := VECTOR_IDLE;
signal interrupt_vector_trigger : std_logic_vector(VECTOR_SIZE - 1 downto 0) := (others => '0');


signal uart_fifo_data_in : std_logic_vector(7 downto 0) := (others => '0');
signal uart_fifo_rd_req : std_logic := '0';
signal uart_fifo_wr_req : std_logic := '0';
signal uart_fifo_empty : std_logic := '0';
signal uart_fifo_full : std_logic := '0';
signal uart_fifo_data_out : std_logic_vector(7 downto 0) := (others => '0');

signal interrupt_vector : std_logic_vector(7 downto 0) := (others => '0');
signal interrupt_vector_count : integer range 0 to 4 := 0;

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

    interrupt_vector_process:
    process(CLOCK, RESET)
    begin
        if RESET = '1' then
            interrupt_vector_trigger <= (others => '0');
            interrupt_vector <= (others => '0');
            interrupt_vector_state <= VECTOR_IDLE;
        elsif rising_edge(CLOCK) then
            ------------------------------------------------------------------------------------------------------------
            -- Anti-latch
            ------------------------------------------------------------------------------------------------------------
            uart_fifo_wr_req <= '0';
            uart_fifo_rd_req <= '0';
            FEEDBACK_TRIGGER <= '0';

            if TRIGGER = '1' then
                interrupt_vector <= COMMAND;
            end if;

            case interrupt_vector_state is

                ------------------------------------------------------------------------------------------------------------
                -- IDLE
                ------------------------------------------------------------------------------------------------------------
                when VECTOR_IDLE =>
                    if TRIGGER = '1' then
                        interrupt_vector <= COMMAND;
                        interrupt_vector_state <= VECTOR_CHECK;
                    end if;

                ------------------------------------------------------------------------------------------------------------
                -- VECTOR CHECK
                ------------------------------------------------------------------------------------------------------------
                when VECTOR_CHECK =>
                    ---------------------------------------------------------------------------------------------------------------
                    -- FIFO DATA
                    ---------------------------------------------------------------------------------------------------------------
                    if interrupt_vector = CMD_FIFO_DATA then

                        interrupt_vector_state <= VECTOR_WRITE;
                    ---------------------------------------------------------------------------------------------------------------
                    -- RESET
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_UART_RESET then

                        interrupt_vector_state <= VECTOR_UART_RESET;
                    ---------------------------------------------------------------------------------------------------------------
                    -- MODE
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_OP_NORMAL then

                        interrupt_vector_state <= VECTOR_OP_NORMAL;

                    elsif interrupt_vector = CMD_OP_UART_FD then

                        interrupt_vector_state <= VECTOR_OP_UART_FD;

                    elsif interrupt_vector = CMD_OP_UART_TD then

                        interrupt_vector_state <= VECTOR_OP_UART_TD;
                    ---------------------------------------------------------------------------------------------------------------
                    -- SCALING
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_SCALING_00 then

                        interrupt_vector_state <= VECTOR_SCALING_00;

                    elsif interrupt_vector = CMD_SCALING_01 then

                        interrupt_vector_state <= VECTOR_SCALING_01;

                    elsif interrupt_vector = CMD_SCALING_10 then

                        interrupt_vector_state <= VECTOR_SCALING_10;

                    elsif interrupt_vector = CMD_SCALING_11 then

                        interrupt_vector_state <= VECTOR_SCALING_11;
                    ---------------------------------------------------------------------------------------------------------------
                    -- TX SOURCE
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_UHD_SOURCE then

                        interrupt_vector_state <= VECTOR_UHD_SOURCE;

                    elsif interrupt_vector = CMD_GEN_SOURCE then

                        interrupt_vector_state <= VECTOR_GEN_SOURCE;
                    ---------------------------------------------------------------------------------------------------------------
                    -- DEBUG
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_A then

                        interrupt_vector_state <= VECTOR_DEBUG_A;

                    elsif interrupt_vector = CMD_DEBUG_B then

                        interrupt_vector_state <= VECTOR_DEBUG_B;

                    elsif interrupt_vector = CMD_DEBUG_C then

                        interrupt_vector_state <= VECTOR_DEBUG_C;

                    elsif interrupt_vector = CMD_DEBUG_D then

                        interrupt_vector_state <= VECTOR_DEBUG_D;

                    elsif interrupt_vector = CMD_DEBUG_E then

                        interrupt_vector_state <= VECTOR_DEBUG_E;

                    elsif interrupt_vector = CMD_DEBUG_F then

                        interrupt_vector_state <= VECTOR_DEBUG_F;

                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- WRITE FIFO DATA
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_WRITE =>
                    if CONTROL_BYTES_NUMBER = interrupt_vector_count then
                        interrupt_vector_state <= VECTOR_FEEDBACK;
                    else
                        if TRIGGER = '1' then
                            uart_fifo_data_in <= COMMAND;
                            uart_fifo_wr_req <= '1';
                            interrupt_vector_count <= interrupt_vector_count + 1;
                        end if;
                    end if;

                when VECTOR_FEEDBACK =>
                    FEEDBACK_DATA <= x"FEEDC0DE";
                    FEEDBACK_TRIGGER <= '1';
                    interrupt_vector_state <= VECTOR_DONE;

                ---------------------------------------------------------------------------------------------------------------
                -- GUI RESET
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_UART_RESET =>
                    interrupt_vector_trigger(0) <= '1';
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- MODE OF OPERATION
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_OP_NORMAL =>
                    interrupt_vector_trigger(2 downto 1) <= "00";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_OP_UART_FD =>
                    interrupt_vector_trigger(2 downto 1) <= "01";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_OP_UART_TD =>
                    interrupt_vector_trigger(2 downto 1) <= "10";
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- SCALING
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_SCALING_00 =>
                    interrupt_vector_trigger(4 downto 3) <= "00";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_01 =>
                    interrupt_vector_trigger(4 downto 3) <= "01";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_10 =>
                    interrupt_vector_trigger(4 downto 3) <= "10";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_11 =>
                    interrupt_vector_trigger(4 downto 3) <= "11";
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- TX SOURCE
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_UHD_SOURCE =>
                    interrupt_vector_trigger(5) <= '0';
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_GEN_SOURCE =>
                    interrupt_vector_trigger(5) <= '1';
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- DEBUG
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_DEBUG_A =>
                    interrupt_vector_trigger(8 downto 6) <= "000";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_B =>
                    interrupt_vector_trigger(8 downto 6) <= "001";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_C =>
                    interrupt_vector_trigger(8 downto 6) <= "010";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_D =>
                    interrupt_vector_trigger(8 downto 6) <= "011";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_E =>
                    interrupt_vector_trigger(8 downto 6) <= "100";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_F =>
                    interrupt_vector_trigger(8 downto 6) <= "101";
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- DONE
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_DONE =>
                    interrupt_vector_count <= 0;
                    interrupt_vector_trigger(0) <= '0';
                    interrupt_vector <= (others => '0');
                    uart_fifo_data_in  <= (others => '0');
                    interrupt_vector_state <= VECTOR_IDLE;

                when others =>
                    interrupt_vector_state <= VECTOR_IDLE;
            end case;
        end if;
    end process;

VECTOR_INTERRUPT <= interrupt_vector_trigger;

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
end architecture rtl;
