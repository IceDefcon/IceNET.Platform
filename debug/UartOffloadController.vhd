library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity UartOffloadController is
port
(
    CLOCK : in std_logic;
    RESET : in std_logic;
    -- IN
    FIFO_MOSI_DATA : in std_logic_vector(7 downto 0);
    FIFO_MISO_DATA : in std_logic_vector(7 downto 0);
    -- IN
    OFFLOAD_TRIGGER : in std_logic;
    -- OUT
    OFFLOAD_MOSI_RD_EN : out std_logic;
    OFFLOAD_MISO_RD_EN : out std_logic;
    OFFLOAD_MOSI_DATA : out std_logic_vector(7 downto 0);
    OFFLOAD_MISO_DATA : out std_logic_vector(7 downto 0);
    -- IN
    UART_FIFO_MOSI_EMPTY : in std_logic;
    UART_FIFO_MISO_EMPTY : in std_logic;
    UART_FIFO_MOSI_FULL : in std_logic;
    UART_FIFO_MISO_FULL : in std_logic;
    -- OUT
    UART_TRIGGER : out std_logic;
    -- IN
    UART_BUSY : in std_logic
);
end UartOffloadController;

architecture rtl of UartOffloadController is

------------------------------------------------------------------------------------------------------------
-- Signals
------------------------------------------------------------------------------------------------------------

    type OFFLOAD_SM is
    (
        OFFLOAD_IDLE,
        OFFLOAD_INIT,
        OFFLOAD_CONFIG,
        OFFLOAD_RD_EN,
        OFFLOAD_RD_EN_STOP,
        OFFLOAD_DATA,
        OFFLOAD_EXECUTE,
        OFFLOAD_WAIT_1,
        OFFLOAD_WAIT_2,
        OFFLOAD_WAIT_3,
        OFFLOAD_DONE
    );
    signal offload_state: OFFLOAD_SM := OFFLOAD_IDLE;

    signal delay_counter : std_logic_vector(15 downto 0) := (others => '0');

------------------------------------------------------------------------------------------------------------
-- Components
------------------------------------------------------------------------------------------------------------

begin

------------------------------------------------------------------------------------------------------------
-- Main Routine
------------------------------------------------------------------------------------------------------------

    offload_sm_process:
    process(CLOCK)
    begin
        if RESET = '1' then
            offload_state <= OFFLOAD_IDLE;
        elsif rising_edge(CLOCK) then

            case offload_state is
                when OFFLOAD_IDLE =>
                    if OFFLOAD_TRIGGER = '1' then
                        offload_state <= OFFLOAD_INIT;
                    end if;

                when OFFLOAD_INIT =>
                    if UART_FIFO_MOSI_EMPTY = '0'
                    and UART_FIFO_MISO_EMPTY = '0' then
                        offload_state <= OFFLOAD_CONFIG;
                    else
                        offload_state <= OFFLOAD_DONE;
                    end if;

                when OFFLOAD_CONFIG =>
                    if UART_BUSY = '0' then
                        offload_state <= OFFLOAD_RD_EN;
                    end if;

                when OFFLOAD_RD_EN =>
                    OFFLOAD_MOSI_RD_EN <= '1';
                    OFFLOAD_MISO_RD_EN <= '1';
                    offload_state <= OFFLOAD_RD_EN_STOP;

                when OFFLOAD_RD_EN_STOP =>
                    OFFLOAD_MOSI_RD_EN <= '0';
                    OFFLOAD_MISO_RD_EN <= '0';
                    offload_state <= OFFLOAD_DATA;

                when OFFLOAD_DATA =>
                    OFFLOAD_MOSI_DATA <= FIFO_MOSI_DATA;
                    OFFLOAD_MISO_DATA <= FIFO_MISO_DATA;
                    offload_state <= OFFLOAD_EXECUTE;

                when OFFLOAD_EXECUTE =>
                    UART_TRIGGER <= '1';
                    offload_state <= OFFLOAD_WAIT_1;

                when OFFLOAD_WAIT_1 =>
                    UART_TRIGGER <= '0';
                    offload_state <= OFFLOAD_WAIT_2;

                when OFFLOAD_WAIT_2 =>
                    offload_state <= OFFLOAD_WAIT_3;

                when OFFLOAD_WAIT_3 =>
                    if delay_counter = "1001110001000000" then -- 1ms Delay
                        delay_counter <= (others => '0');
                        offload_state <= OFFLOAD_INIT;
                    else
                        delay_counter <= delay_counter + '1';
                    end if;

                when OFFLOAD_DONE =>
                    offload_state <= OFFLOAD_IDLE;

                when others =>
                    offload_state <= OFFLOAD_IDLE;
            end case;
        end if;
    end process;


end architecture;
