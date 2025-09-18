library ieee;
use ieee.std_logic_1164.all;
use work.UartTypes.all;

entity UartIrqController is
generic
(
    VECTOR_SIZE : integer := 10
);
Port
(
    CLOCK : in  std_logic;
    RESET : in  std_logic;

    VECTOR_TRIGGER : in std_logic;
    VECTOR_BYTE : in std_logic_vector(7 downto 0);

    VECTOR_INTERRUPT : out std_logic_vector(VECTOR_SIZE - 1 downto 0);

    PARAMETER_READY : out std_logic_vector(PARAMETER_NUMBER - 1 downto 0);
    PARAMETER_MATRIX : out PARAMETER_ARRAY;

    FEEDBACK_DATA : out std_logic_vector(31 downto 0);
    FEEDBACK_TRIGGER : out std_logic
);
end entity UartIrqController;

architecture rtl of UartIrqController is

------------------------------------------------------------------------------------------------------------
-- Constants
------------------------------------------------------------------------------------------------------------
constant CONTROL_BYTES_NUMBER : integer := 8;
constant PARAMETER_TIMEOUT : integer := 4096;

constant CMD_PARAMETER  : std_logic_vector(7 downto 0) := "00100100"; -- 0x24
constant CMD_250_PULSE  : std_logic_vector(7 downto 0) := "00110000"; -- 0x30
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
constant CMD_DEBUG_G : std_logic_vector(7 downto 0) := "01000111"; -- 0x47
constant CMD_DEBUG_H : std_logic_vector(7 downto 0) := "01001000"; -- 0x48

constant CMD_PARAM_BANDWIDTH : std_logic_vector(7 downto 0) := "01000001"; -- 0x41
constant CMD_PARAM_THRESHOLD : std_logic_vector(7 downto 0) := "01000010"; -- 0x42

------------------------------------------------------------------------------------------------------------
-- Signals and Types
------------------------------------------------------------------------------------------------------------
type VECTOR_SM is
(
    VECTOR_IDLE,
    VECTOR_CHECK,
    VECTOR_ERROR,
    VECTOR_PARAMETER_ID,
    VECTOR_PARAMETER_WRITE,
    VECTOR_PARAMETER_OFFLOAD,
    VECTOR_PARAMETER_OUTPUTS,
    VECTOR_PARAMETER_ID_ERROR,
    VECTOR_PARAMETER_FEEDBACK,
    VECTOR_250_PULSE,   -- 0x30 :: ASCII "0"
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
    VECTOR_DEBUG_G,     -- 0x47 :: ASCII "G"
    VECTOR_DEBUG_H,     -- 0x48 :: ASCII "H"
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
signal interrupt_vector_count : integer range 0 to CONTROL_BYTES_NUMBER := 0;

signal interrupt_vector_parameter_id : std_logic_vector(7 downto 0) := (others => '0');
signal interrupt_vector_parameter_data : std_logic_vector(31 downto 0) := (others => '0');
signal interrupt_vector_parameter_hex : std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_vector_parameter_timeout : integer range 0 to 4096 := 0;

signal i : integer range 0 to 32 := 0;

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
            PARAMETER_READY <= (others => '0');
            PARAMETER_MATRIX <= (others => (others => '0'));
            interrupt_vector_trigger <= (others => '0');
            interrupt_vector <= (others => '0');
            uart_fifo_data_in <= (others => '0');
            uart_fifo_wr_req  <= '0';
            uart_fifo_rd_req  <= '0';
            interrupt_vector_state <= VECTOR_IDLE;
            interrupt_vector_count <= 0;
            interrupt_vector_parameter_id   <= (others => '0');
            interrupt_vector_parameter_data <= (others => '0');
            interrupt_vector_parameter_hex  <= (others => '0');
            interrupt_vector_parameter_timeout <= 0;
            FEEDBACK_DATA <= (others => '0');
            FEEDBACK_TRIGGER <= '0';
            i <= 0;
        elsif rising_edge(CLOCK) then
            ------------------------------------------------------------------------------------------------------------
            -- Anti-latch
            ------------------------------------------------------------------------------------------------------------
            uart_fifo_wr_req <= '0';
            uart_fifo_rd_req <= '0';
            FEEDBACK_TRIGGER <= '0';

            case interrupt_vector_state is
                ------------------------------------------------------------------------------------------------------------
                -- IDLE
                ------------------------------------------------------------------------------------------------------------
                when VECTOR_IDLE =>
                    interrupt_vector_parameter_timeout <= 0;
                    if VECTOR_TRIGGER = '1' then
                        interrupt_vector <= VECTOR_BYTE;
                        interrupt_vector_state <= VECTOR_CHECK;
                    end if;

                ------------------------------------------------------------------------------------------------------------
                -- VECTOR CHECK
                ------------------------------------------------------------------------------------------------------------
                when VECTOR_CHECK =>
                    ---------------------------------------------------------------------------------------------------------------
                    -- FIFO DATA
                    ---------------------------------------------------------------------------------------------------------------
                    if interrupt_vector = CMD_PARAMETER then

                        interrupt_vector_state <= VECTOR_PARAMETER_ID;
                    ---------------------------------------------------------------------------------------------------------------
                    -- RESET
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_250_PULSE then

                        interrupt_vector_state <= VECTOR_250_PULSE;
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
                    -- ASCII "A" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_A then

                        interrupt_vector_state <= VECTOR_DEBUG_A;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "B" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_B then

                        interrupt_vector_state <= VECTOR_DEBUG_B;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "C" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_C then

                        interrupt_vector_state <= VECTOR_DEBUG_C;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "D" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_D then

                        interrupt_vector_state <= VECTOR_DEBUG_D;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "E" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_E then

                        interrupt_vector_state <= VECTOR_DEBUG_E;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "F" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_F then

                        interrupt_vector_state <= VECTOR_DEBUG_F;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "G" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_G then

                        interrupt_vector_state <= VECTOR_DEBUG_G;

                    ---------------------------------------------------------------------------------------------------------------
                    -- ASCII "H" DETECTED
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_H then

                        interrupt_vector_state <= VECTOR_DEBUG_H;

                    else

                        interrupt_vector_state <= VECTOR_ERROR;

                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- BAD VECTOR RECEIVED
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_ERROR =>
                    interrupt_vector_parameter_data <= x"DEADBEEF";
                    interrupt_vector_state <= VECTOR_PARAMETER_FEEDBACK;

                ---------------------------------------------------------------------------------------------------------------
                -- GET PARAMETER ID
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_ID =>
                    if VECTOR_TRIGGER = '1' then
                        interrupt_vector_parameter_id <= VECTOR_BYTE;
                        interrupt_vector_state <= VECTOR_PARAMETER_WRITE;
                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- WRITE PARAMETER TO FIFO
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_WRITE =>
                    if CONTROL_BYTES_NUMBER = interrupt_vector_count then
                        interrupt_vector_state <= VECTOR_PARAMETER_OFFLOAD;
                        uart_fifo_rd_req <= '1'; -- 1st symbol
                    else
                        if VECTOR_TRIGGER = '1' then
                            uart_fifo_data_in <= VECTOR_BYTE;
                            uart_fifo_wr_req <= '1';
                            interrupt_vector_count <= interrupt_vector_count + 1;
                        end if;
                    end if;

                    if interrupt_vector_parameter_timeout = PARAMETER_TIMEOUT then
                        interrupt_vector_state <= VECTOR_PARAMETER_ID_ERROR;
                    else
                        interrupt_vector_parameter_timeout <= interrupt_vector_parameter_timeout + 1;
                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- OFFLOAD PARAMETER FROM FIFO
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_OFFLOAD =>

                    interrupt_vector_parameter_data(31 - i downto 28 - i) <= ASCII_TO_HEX(uart_fifo_data_out);

                    if 0 = interrupt_vector_count then
                        interrupt_vector_state <= VECTOR_PARAMETER_OUTPUTS;
                        interrupt_vector_count <= 0;
                        uart_fifo_rd_req <= '0'; -- Stop Reading
                        i <= 0;
                    else

                        if interrupt_vector_count /= 8 then -- Skip first increment
                            i <= i + 4;
                        end if;

                        if interrupt_vector_count /= 1 then -- Skip last RD
                            uart_fifo_rd_req <= '1'; -- Consecutive symbols
                        end if;

                        interrupt_vector_count <= interrupt_vector_count - 1;
                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- OFFLOAD PARAMETER FROM FIFO
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_OUTPUTS =>
                    if interrupt_vector_parameter_id = CMD_PARAM_BANDWIDTH then
                        PARAMETER_READY(0) <= '1';
                        PARAMETER_MATRIX(0) <= interrupt_vector_parameter_data;
                        interrupt_vector_state <= VECTOR_PARAMETER_FEEDBACK;
                    elsif interrupt_vector_parameter_id = CMD_PARAM_THRESHOLD then
                        PARAMETER_READY(1) <= '1';
                        PARAMETER_MATRIX(1) <= interrupt_vector_parameter_data;
                        interrupt_vector_state <= VECTOR_PARAMETER_FEEDBACK;
                    else
                        interrupt_vector_state <= VECTOR_PARAMETER_ID_ERROR;
                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- BAD PARAMETER ID RECEIVED
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_ID_ERROR =>
                    if uart_fifo_empty = '1' then
                        uart_fifo_rd_req <= '0';
                        interrupt_vector_parameter_data <= x"DEADC0DE";
                        interrupt_vector_state <= VECTOR_PARAMETER_FEEDBACK;
                    else
                        uart_fifo_rd_req <= '1';
                    end if;

                ---------------------------------------------------------------------------------------------------------------
                -- FEEDBACK DATA TO PC
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_PARAMETER_FEEDBACK =>
                    FEEDBACK_DATA <= interrupt_vector_parameter_data;
                    FEEDBACK_TRIGGER <= '1';
                    interrupt_vector_state <= VECTOR_DONE;

                ---------------------------------------------------------------------------------------------------------------
                -- GUI RESET
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_250_PULSE =>
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

                when VECTOR_DEBUG_G =>
                    interrupt_vector_trigger(8 downto 6) <= "110";
                    interrupt_vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_H =>
                    interrupt_vector_trigger(8 downto 6) <= "111";
                    interrupt_vector_state <= VECTOR_DONE;
                ---------------------------------------------------------------------------------------------------------------
                -- DONE
                ---------------------------------------------------------------------------------------------------------------
                when VECTOR_DONE =>
                    interrupt_vector_count <= 0;
                    interrupt_vector_trigger(0) <= '0';
                    interrupt_vector <= (others => '0');
                    uart_fifo_data_in  <= (others => '0');
                    interrupt_vector_parameter_id <= (others => '0');
                    interrupt_vector_parameter_data <= (others => '0');
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
