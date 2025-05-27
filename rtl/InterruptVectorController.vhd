library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity InterruptVectorController is
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    FIFO_PRIMARY_MOSI : out std_logic_vector(7 downto 0);
    FIFO_PRIMARY_WR_EN : out std_logic
);
end entity InterruptVectorController;

architecture rtl of InterruptVectorController is

type TEMP_SM is
(
    TEMP_IDLE,
    TEMP_PRODUCE,
    TEMP_DONE
);
signal temp_state: TEMP_SM := TEMP_IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if RESET = '1' then
            temp_state <= TEMP_IDLE;
        elsif rising_edge(CLOCK_50MHz) then

            case temp_state is
                when TEMP_IDLE =>

                when TEMP_PRODUCE =>

                when TEMP_DONE =>

                when others =>
                    temp_state <= TEMP_IDLE;
            end case;

        end if;
    end process interrupt_process;
end architecture rtl;
