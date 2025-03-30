library ieee;
use ieee.std_logic_1164.all;

entity CanController is
port
(
    CLOCK_50MHz : in std_logic;
    RESET : in std_logic;

    CAN_BBB_TX : in std_logic;
    CAN_BBB_RX : out std_logic;

    CAN_MPP_TX : out std_logic;
    CAN_MPP_RX : in std_logic
);
end CanController;

architecture rtl of CanController is

begin

can_loopthrough_process:
process(CLOCK_50MHz)
begin
    if RESET = '0' then

    elsif rising_edge(CLOCK_50MHz) then
        CAN_MPP_TX <= CAN_BBB_TX;
        CAN_BBB_RX <= CAN_MPP_RX;
    end if;
end process;

end architecture;
