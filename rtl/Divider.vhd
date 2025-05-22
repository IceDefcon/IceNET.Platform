library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Divider is
generic
(
    DEPTH : integer := 16
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    TRIGGER : in std_logic;
    DIVIDENT : in std_logic_vector(DEPTH - 1 downto 0);
    DIVISOR : in std_logic_vector(DEPTH - 1 downto 0);

    QUOTIENT : out std_logic_vector(DEPTH - 1 downto 0);
    REMINDER : out std_logic_vector(DEPTH - 1 downto 0);
    DIVISION_COMPLETE : out std_logic
);
end entity Divider;

architecture rtl of Divider is

type DIVISION_SM is
(
    DIVIDER_IDLE,
    DIVIDER_INIT,
    DIVIDER_CONFIG,
    DIVIDER_SHIFT,
    DIVIDER_UPDATE,
    DIVIDER_SUBSTRACT,
    DIVIDER_RESTORE,
    DIVIDER_DONE
);
signal division_state: DIVISION_SM := DIVIDER_IDLE;

signal A : std_logic_vector(DEPTH - 1 downto 0) := (others => '0');
signal Q : std_logic_vector(DEPTH - 1 downto 0) := (others => '0');
signal M : std_logic_vector(DEPTH - 1 downto 0) := (others => '0');

signal shift_vector : std_logic_vector(DEPTH + DEPTH - 1 downto 0) := (others => '0');

signal subtraction : std_logic_vector(DEPTH - 1 downto 0) := (others => '0');

signal iteration_counter : integer := 0;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if RESET = '1' then
            iteration_counter <= 0;
            shift_vector <= (others => '0');
            A <= (others => '0');
            Q <= (others => '0');
            M <= (others => '0');
            division_state <= DIVIDER_IDLE;
        elsif rising_edge(CLOCK_50MHz) then
            case division_state is

                when DIVIDER_IDLE =>
                    DIVISION_COMPLETE <= '0';
                    if TRIGGER = '1' then
                        division_state <= DIVIDER_INIT;
                    end if;

                when DIVIDER_INIT =>
                    iteration_counter <= 0;
                    shift_vector <= (others => '0');
                    A <= (others => '0');
                    Q <= DIVIDENT;
                    M <= DIVISOR;
                    division_state <= DIVIDER_CONFIG;

                when DIVIDER_CONFIG =>
                    if iteration_counter = DEPTH then
                        division_state <= DIVIDER_DONE;
                    else
                        shift_vector <= A & Q;
                        iteration_counter <= iteration_counter + 1;
                        division_state <= DIVIDER_SHIFT;
                    end if;

                when DIVIDER_SHIFT =>
                    for i in DEPTH + DEPTH - 1 downto 1 loop
                        shift_vector(i) <= shift_vector(i - 1);
                    end loop;
                    shift_vector(0) <= '0';
                    division_state <= DIVIDER_UPDATE;

                when DIVIDER_UPDATE =>
                    A <= shift_vector(DEPTH + DEPTH - 1 downto DEPTH);
                    Q <= shift_vector(DEPTH - 1 downto 0);
                    division_state <= DIVIDER_SUBSTRACT;

                when DIVIDER_SUBSTRACT =>
                    subtraction <= std_logic_vector(signed(A) - signed(M));
                    division_state <= DIVIDER_RESTORE;

                when DIVIDER_RESTORE =>
                    if subtraction(DEPTH - 1) = '0' then
                        A <= subtraction;
                        Q <= Q + '1';
                    elsif subtraction(DEPTH - 1) = '1' then
                        A <= std_logic_vector(unsigned(subtraction) + unsigned(M));
                    end if;
                    division_state <= DIVIDER_CONFIG;

                when DIVIDER_DONE =>
                    QUOTIENT <= Q;
                    REMINDER <= A;
                    DIVISION_COMPLETE <= '1';
                    division_state <= DIVIDER_IDLE;

                when others =>
                    division_state <= DIVIDER_IDLE;

            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
