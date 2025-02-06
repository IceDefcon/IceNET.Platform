library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Temp is
Port
(
    CLOCK_50MHz : in  std_logic;

    INT_IN : in std_logic;

    MISO : in std_logic;
    MOSI : out std_logic;
    SCK : out std_logic;
    CSN : out std_logic;

    INT_OUT : out std_logic
);
end entity Temp;

architecture rtl of Temp is

type INTERRUPT is
(
    IDLE,
    PRODUCE,
    DONE
);
signal temp_state: INTERRUPT := IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin
        if rising_edge(CLOCK_50MHz) then

            case temp_state is
                when IDLE =>

                when PRODUCE =>

                when DONE =>

                when others =>
                    temp_state <= IDLE;
            end case;
        end if;
    end process interrupt_process;
end architecture rtl;
