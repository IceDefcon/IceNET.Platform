library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity IRQ_CONTROLLER is
generic
(
    VECTOR_SIZE : integer := 6
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

type VECTOR_TYPE is
(
    VECTOR_IDLE,
    VECTOR_0, -- 0x31 :: Keyboard 1
    VECTOR_1, -- 0x32 :: Keyboard 2
    VECTOR_2, -- 0x33 :: Keyboard 3
    VECTOR_3, -- 0x34 :: Keyboard 4
    VECTOR_4, -- 0x35 :: Keyboard 5
    VECTOR_5, -- 0x36 :: Keyboard 6
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
                    if interrupt_vector = "0110001" then
                        vector_state <= VECTOR_0;
                    elsif interrupt_vector = "0110010" then
                        vector_state <= VECTOR_1;
                    elsif interrupt_vector = "0110011" then
                        vector_state <= VECTOR_2;
                    elsif interrupt_vector = "0110100" then
                        vector_state <= VECTOR_3;
                    elsif interrupt_vector = "0110101" then
                        vector_state <= VECTOR_4;
                    elsif interrupt_vector = "0110110" then
                        vector_state <= VECTOR_5;
                    end if;

                when VECTOR_0 =>
                    vector_trigger(1 downto 0) <= "00";
                    vector_state <= VECTOR_DONE;

                when VECTOR_1 =>
                    vector_trigger(1 downto 0) <= "01";
                    vector_state <= VECTOR_DONE;

                when VECTOR_2 =>
                    vector_trigger(1 downto 0) <= "10";
                    vector_state <= VECTOR_DONE;

                when VECTOR_3 =>
                    vector_trigger(3 downto 2) <= "00";
                    vector_state <= VECTOR_DONE;

                when VECTOR_4 =>
                    vector_trigger(3 downto 2) <= "01";
                    vector_state <= VECTOR_DONE;

                when VECTOR_5 =>
                    vector_trigger(3 downto 2) <= "10";
                    vector_state <= VECTOR_DONE;

                when VECTOR_DONE =>
                    interrupt_vector <= (others => '0');
                    vector_state <= VECTOR_IDLE;

                when others =>
                    vector_state <= VECTOR_IDLE;
            end case;
        end if;
    end process;

VECTOR_INTERRUPT <= vector_trigger;

end architecture rtl;
