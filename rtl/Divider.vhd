library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Divider is
generic
(
    DEPTH : integer := 16
);
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    DIVIDENT : in std_logic_vector(DEPTH - 1 downto 0);
    DIVISOR : in std_logic_vector(DEPTH - 1 downto 0);

    QUOTIENT : out std_logic_vector(DEPTH - 1 downto 0);
    REMINDER : out std_logic_vector(DEPTH - 1 downto 0)
);
end entity Divider;

architecture rtl of Divider is

type DIVISION_SM is
(
    DIVIDER_IDLE,
    DIVIDER_SHIFTT,
    DIVIDER_SUBTRAC,
    DIVIDER_RESTORE,
    DIVIDER_DONE
);
signal division_state: DIVISION_SM := DIVIDER_IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if RESET = '1' then
            --
            -- Rest of internal signals
            --
            division_state <= DIVIDER_IDLE;
        elsif rising_edge(CLOCK_50MHz) then
            case division_state is
                when DIVIDER_IDLE =>

                when DIVIDER_SHIFTT =>

                when DIVIDER_SUBTRAC =>

                when DIVIDER_RESTORE =>

                when DIVIDER_DONE =>

                when others =>
                    division_state <= DIVIDER_IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
