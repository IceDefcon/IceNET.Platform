library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SensorFifo_OffloadController is
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in std_logic;

    VECTOR_SWITCH  : in  std_logic;

    OFFLOAD_INTERRUPT  : in  std_logic;
    OFFLOAD_BIT_COUNT : in std_logic_vector(3 downto 0);
    OFFLOAD_FIFO_EMPTY  : in  std_logic;

    OFFLOAD_READ_ENABLE : out std_logic;
    OFFLOAD_SECONDARY_DMA_TRIGGER : out  std_logic;

    OFFLOAD_DEBUG : out std_logic
);
end entity SensorFifo_OffloadController;

architecture rtl of SensorFifo_OffloadController is

type OFFLOAD_TYPE is
(
    OFFLOAD_IDLE,
    OFFLOAD_INIT,
    OFFLOAD_FIRST_READ_FIFO,
    OFFLOAD_FIRST_DMA_TRIGGER,
    OFFLOAD_FIRST_DONE,
    --
    -- TODO
    --
    -- There is asynchronous FIFO clear flag
    --
    OFFLOAD_CONFIG,
    OFFLOAD_FIFO_READ,
    OFFLOAD_FIFO_CLEAN,
    OFFLOAD_TRANSFER,
    OFFLOAD_DONE
);
signal offload_state: OFFLOAD_TYPE := OFFLOAD_IDLE;
signal check_delay : integer := 400000;
signal offload_counter : integer range 0 to 400000 := 0;
signal offload_fifo_empty_counter : integer range 0 to 256 := 0;
begin

    interrupt_process: process(CLOCK_50MHz)
    begin

        if RESET = '1' then
            offload_state <= OFFLOAD_IDLE;
            OFFLOAD_READ_ENABLE <= '0';
            OFFLOAD_SECONDARY_DMA_TRIGGER <= '0';
            check_delay <= 400000;
        elsif rising_edge(CLOCK_50MHz) then

            if VECTOR_SWITCH = '1' then
                check_delay <= 25000;
            else
                check_delay <= 400000;
            end if;

            case offload_state is

                when OFFLOAD_IDLE =>
                    OFFLOAD_DEBUG <= '0';
                    if OFFLOAD_INTERRUPT = '1' then
                        offload_state <= OFFLOAD_INIT;
                    end if;

                when OFFLOAD_INIT =>
                    offload_state <= OFFLOAD_FIRST_READ_FIFO;

                when OFFLOAD_FIRST_READ_FIFO =>
                    OFFLOAD_READ_ENABLE <= '1';
                    offload_state <= OFFLOAD_FIRST_DMA_TRIGGER;

                when OFFLOAD_FIRST_DMA_TRIGGER =>
                    OFFLOAD_READ_ENABLE <= '0';
                    OFFLOAD_SECONDARY_DMA_TRIGGER <= '1';
                    offload_state <= OFFLOAD_FIRST_DONE;

                when OFFLOAD_FIRST_DONE =>
                    OFFLOAD_SECONDARY_DMA_TRIGGER <= '0';
                    offload_state <= OFFLOAD_CONFIG;

                when OFFLOAD_CONFIG =>
                    offload_counter <= 0;
                    if OFFLOAD_FIFO_EMPTY = '0' then
                        offload_state <= OFFLOAD_FIFO_READ;
                    elsif OFFLOAD_FIFO_EMPTY = '1' then
                        offload_state <= OFFLOAD_IDLE;
                    end if;

                when OFFLOAD_FIFO_READ =>
                    if OFFLOAD_BIT_COUNT = "1000" then
                        OFFLOAD_READ_ENABLE <= '1';
                        offload_state <= OFFLOAD_TRANSFER;
                    end if;

                    if offload_counter = check_delay then
                        offload_state <= OFFLOAD_FIFO_CLEAN;
                    else
                        offload_counter <= offload_counter + 1;
                    end if;

                when OFFLOAD_FIFO_CLEAN =>
                    OFFLOAD_DEBUG <= '1';
                    if offload_fifo_empty_counter = 256 then
                        offload_fifo_empty_counter <= 0;
                        OFFLOAD_READ_ENABLE <= '0';
                        offload_state <= OFFLOAD_IDLE;
                    else
                        OFFLOAD_READ_ENABLE <= '1';
                        offload_fifo_empty_counter <= offload_fifo_empty_counter + 1;
                    end if;

                when OFFLOAD_TRANSFER =>
                    OFFLOAD_READ_ENABLE <= '0';
                    offload_state <= OFFLOAD_DONE;

                when OFFLOAD_DONE =>
                    offload_state <= OFFLOAD_CONFIG;

                when others =>
                    offload_state <= OFFLOAD_IDLE;
            end case;

        end if;
    end process interrupt_process;
end architecture rtl;
