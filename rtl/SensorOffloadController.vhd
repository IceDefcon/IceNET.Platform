library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SensorFifo_OffloadController is
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    IRQ_VECTOR_OFFLOAD  : in  std_logic;

    OFFLOAD_READ_ENABLE : out std_logic;
    SECONDARY_DMA_TRIGGER  : out  std_logic
);
end entity SensorFifo_OffloadController;

architecture rtl of SensorFifo_OffloadController is

type OFFLOAD_TYPE is
(
    OFFLOAD_IDLE,
    OFFLOAD_INIT,
    OFFLOAD_CONFIG,
    OFFLOAD_TRANSFER,
    OFFLOAD_DONE
);
signal offload_state: OFFLOAD_TYPE := OFFLOAD_IDLE;

begin

    interrupt_process: process(CLOCK_50MHz)
    begin

        if RESET = '1' then
            offload_state <= OFFLOAD_IDLE;
            OFFLOAD_READ_ENABLE <= '0';
            SECONDARY_DMA_TRIGGER <= '0';
        elsif rising_edge(CLOCK_50MHz) then

            case offload_state is

                when OFFLOAD_IDLE =>
                    if IRQ_VECTOR_OFFLOAD = '1' then
                        offload_state <= OFFLOAD_INIT;
                    end if;

                when OFFLOAD_INIT =>
                    offload_state <= OFFLOAD_CONFIG;

                when OFFLOAD_CONFIG =>
                    offload_state <= OFFLOAD_TRANSFER;

                when OFFLOAD_TRANSFER =>
                    SECONDARY_DMA_TRIGGER <= '1';
                    offload_state <= OFFLOAD_DONE;

                when OFFLOAD_DONE =>
                    SECONDARY_DMA_TRIGGER <= '0';
                    offload_state <= OFFLOAD_IDLE;

                when others =>
                    offload_state <= OFFLOAD_IDLE;
            end case;

        end if;
    end process interrupt_process;
end architecture rtl;
