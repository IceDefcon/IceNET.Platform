library ieee;
use ieee.std_logic_1164.all;

entity IRQ_CONTROLLER is
generic
(
    VECTOR_SIZE : integer := 10
);
Port
(
    CLOCK : in  std_logic;
    RESET : in  std_logic;

    TRIGGER : in std_logic;
    COMMAND : in std_logic_vector(6 downto 0);

    VECTOR_INTERRUPT : out std_logic_vector(VECTOR_SIZE - 1 downto 0)
);
end entity IRQ_CONTROLLER;

architecture rtl of IRQ_CONTROLLER is

constant CMD_UART_RESET : std_logic_vector(6 downto 0) := "0110000"; -- 0x30
constant CMD_OP_NORMAL : std_logic_vector(6 downto 0) := "0110001";  -- 0x31
constant CMD_OP_UART_FD : std_logic_vector(6 downto 0) := "0110010"; -- 0x32
constant CMD_OP_UART_TD : std_logic_vector(6 downto 0) := "0110011"; -- 0x33
constant CMD_SCALING_00 : std_logic_vector(6 downto 0) := "0110100"; -- 0x34
constant CMD_SCALING_01 : std_logic_vector(6 downto 0) := "0110101"; -- 0x35
constant CMD_SCALING_10 : std_logic_vector(6 downto 0) := "0110110"; -- 0x36
constant CMD_SCALING_11 : std_logic_vector(6 downto 0) := "0110111"; -- 0x37
constant CMD_UHD_SOURCE : std_logic_vector(6 downto 0) := "0111000"; -- 0x38
constant CMD_GEN_SOURCE : std_logic_vector(6 downto 0) := "0111001"; -- 0x39
constant CMD_DEBUG_A : std_logic_vector(6 downto 0) := "1000001"; -- 0x41
constant CMD_DEBUG_B : std_logic_vector(6 downto 0) := "1000010"; -- 0x42
constant CMD_DEBUG_C : std_logic_vector(6 downto 0) := "1000011"; -- 0x43
constant CMD_DEBUG_D : std_logic_vector(6 downto 0) := "1000100"; -- 0x44
constant CMD_DEBUG_E : std_logic_vector(6 downto 0) := "1000101"; -- 0x45
constant CMD_DEBUG_F : std_logic_vector(6 downto 0) := "1000110"; -- 0x46

type VECTOR_TYPE is
(
    VECTOR_IDLE,
    VECTOR_UART_RESET,  -- 0x30 :: PC Keyboard 0
    VECTOR_OP_NORMAL,   -- 0x31 :: PC Keyboard 1
    VECTOR_OP_UART_FD,  -- 0x32 :: PC Keyboard 2
    VECTOR_OP_UART_TD,  -- 0x33 :: PC Keyboard 3
    VECTOR_SCALING_00,  -- 0x34 :: PC Keyboard 4
    VECTOR_SCALING_01,  -- 0x35 :: PC Keyboard 5
    VECTOR_SCALING_10,  -- 0x36 :: PC Keyboard 6
    VECTOR_SCALING_11,  -- 0x37 :: PC Keyboard 7
    VECTOR_UHD_SOURCE,  -- 0x38 :: PC Keyboard 8
    VECTOR_GEN_SOURCE,  -- 0x39 :: PC Keyboard 9
    VECTOR_DEBUG_A,     -- 0x41 :: PC Keyboard A
    VECTOR_DEBUG_B,     -- 0x42 :: PC Keyboard B
    VECTOR_DEBUG_C,     -- 0x43 :: PC Keyboard C
    VECTOR_DEBUG_D,     -- 0x44 :: PC Keyboard D
    VECTOR_DEBUG_E,     -- 0x45 :: PC Keyboard E
    VECTOR_DEBUG_F,     -- 0x46 :: PC Keyboard F
    VECTOR_DONE
);
signal vector_state: VECTOR_TYPE := VECTOR_IDLE;

signal vector_trigger : std_logic_vector(VECTOR_SIZE - 1 downto 0) := (others => '0');
signal interrupt_vector : std_logic_vector(6 downto 0) := (others => '1');

begin

    interrupt_process: process(CLOCK, RESET)
    begin
        if RESET = '1' then
            vector_trigger <= (others => '0');
            interrupt_vector <= (others => '0');
            vector_state <= VECTOR_IDLE;
        elsif rising_edge(CLOCK) then

            if TRIGGER = '1' then
                interrupt_vector <= COMMAND;
            end if;

            case vector_state is
                when VECTOR_IDLE =>
                    ---------------------------------------------------------------------------------------------------------------
                    -- RESET
                    ---------------------------------------------------------------------------------------------------------------
                    if interrupt_vector = CMD_UART_RESET then

                        vector_state <= VECTOR_UART_RESET;
                    ---------------------------------------------------------------------------------------------------------------
                    -- MODE
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_OP_NORMAL then

                        vector_state <= VECTOR_OP_NORMAL;

                    elsif interrupt_vector = CMD_OP_UART_FD then

                        vector_state <= VECTOR_OP_UART_FD;

                    elsif interrupt_vector = CMD_OP_UART_TD then

                        vector_state <= VECTOR_OP_UART_TD;
                    ---------------------------------------------------------------------------------------------------------------
                    -- SCALING
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_SCALING_00 then

                        vector_state <= VECTOR_SCALING_00;

                    elsif interrupt_vector = CMD_SCALING_01 then

                        vector_state <= VECTOR_SCALING_01;

                    elsif interrupt_vector = CMD_SCALING_10 then

                        vector_state <= VECTOR_SCALING_10;

                    elsif interrupt_vector = CMD_SCALING_11 then

                        vector_state <= VECTOR_SCALING_11;
                    ---------------------------------------------------------------------------------------------------------------
                    -- TX SOURCE
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_UHD_SOURCE then

                        vector_state <= VECTOR_UHD_SOURCE;

                    elsif interrupt_vector = CMD_GEN_SOURCE then

                        vector_state <= VECTOR_GEN_SOURCE;
                    ---------------------------------------------------------------------------------------------------------------
                    -- DEBUG
                    ---------------------------------------------------------------------------------------------------------------
                    elsif interrupt_vector = CMD_DEBUG_A then

                        vector_state <= VECTOR_DEBUG_A;

                    elsif interrupt_vector = CMD_DEBUG_B then

                        vector_state <= VECTOR_DEBUG_B;

                    elsif interrupt_vector = CMD_DEBUG_C then

                        vector_state <= VECTOR_DEBUG_C;

                    elsif interrupt_vector = CMD_DEBUG_D then

                        vector_state <= VECTOR_DEBUG_D;

                    elsif interrupt_vector = CMD_DEBUG_E then

                        vector_state <= VECTOR_DEBUG_E;

                    elsif interrupt_vector = CMD_DEBUG_F then

                        vector_state <= VECTOR_DEBUG_F;

                    end if;

                ----------------------------------------------------------------------------
                -- GUI RESET
                ----------------------------------------------------------------------------
                when VECTOR_UART_RESET =>
                    vector_trigger(0) <= '1';
                    vector_state <= VECTOR_DONE;
                ----------------------------------------------------------------------------
                -- MODE OF OPERATION
                ----------------------------------------------------------------------------
                when VECTOR_OP_NORMAL =>
                    vector_trigger(2 downto 1) <= "00";
                    vector_state <= VECTOR_DONE;

                when VECTOR_OP_UART_FD =>
                    vector_trigger(2 downto 1) <= "01";
                    vector_state <= VECTOR_DONE;

                when VECTOR_OP_UART_TD =>
                    vector_trigger(2 downto 1) <= "10";
                    vector_state <= VECTOR_DONE;
                ----------------------------------------------------------------------------
                -- SCALING
                ----------------------------------------------------------------------------
                when VECTOR_SCALING_00 =>
                    vector_trigger(4 downto 3) <= "00";
                    vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_01 =>
                    vector_trigger(4 downto 3) <= "01";
                    vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_10 =>
                    vector_trigger(4 downto 3) <= "10";
                    vector_state <= VECTOR_DONE;

                when VECTOR_SCALING_11 =>
                    vector_trigger(4 downto 3) <= "11";
                    vector_state <= VECTOR_DONE;
                ----------------------------------------------------------------------------
                -- TX SOURCE
                ----------------------------------------------------------------------------
                when VECTOR_UHD_SOURCE =>
                    vector_trigger(5) <= '0';
                    vector_state <= VECTOR_DONE;

                when VECTOR_GEN_SOURCE =>
                    vector_trigger(5) <= '1';
                    vector_state <= VECTOR_DONE;
                ----------------------------------------------------------------------------
                -- DEBUG
                ----------------------------------------------------------------------------
                when VECTOR_DEBUG_A =>
                    vector_trigger(8 downto 6) <= "000";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_B =>
                    vector_trigger(8 downto 6) <= "001";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_C =>
                    vector_trigger(8 downto 6) <= "010";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_D =>
                    vector_trigger(8 downto 6) <= "011";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_E =>
                    vector_trigger(8 downto 6) <= "100";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DEBUG_F =>
                    vector_trigger(8 downto 6) <= "101";
                    vector_state <= VECTOR_DONE;
                ----------------------------------------------------------------------------
                -- DONE
                ----------------------------------------------------------------------------
                when VECTOR_DONE =>
                    vector_trigger(0) <= '0';
                    interrupt_vector <= (others => '0');
                    vector_state <= VECTOR_IDLE;

                when others =>
                    vector_state <= VECTOR_IDLE;
            end case;
        end if;
    end process;

VECTOR_INTERRUPT <= vector_trigger;

end architecture rtl;
