library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity OffloadController is
port
(    
    CLOCK_50MHz : in std_logic;

    OFFLOAD_INTERRUPT : in std_logic;
    FIFO_DATA : in std_logic_vector(7 downto 0);
    FIFO_READ_ENABLE : out std_logic;

    OFFLOAD_READY : out std_logic;
    OFFLOAD_ID : out std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : out std_logic_vector(7 downto 0);
    OFFLOAD_CTRL : out std_logic_vector(7 downto 0);
    OFFLOAD_DATA : out std_logic_vector(7 downto 0)
);
end OffloadController;

architecture rtl of OffloadController is

type STATE is 
(
    IDLE,
    DELAY_INIT,
    DELAY_CONFIG,
    READ_ID,
    READ_REGISTER,
    READ_CONTROL,
    READ_DATA
);
signal offload_state: STATE := IDLE;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin
    
offload_process:
process (CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        case offload_state is

            when IDLE =>
                OFFLOAD_READY <= '0';
                if OFFLOAD_INTERRUPT = '1' then
                    offload_state <= DELAY_INIT;
                else
                    offload_state <= IDLE;
                end if;

            when DELAY_INIT =>
                FIFO_READ_ENABLE <= '1';
                offload_state <= DELAY_CONFIG;

            when DELAY_CONFIG =>
                FIFO_READ_ENABLE <= '1';
                offload_state <= READ_CONTROL;

            when READ_CONTROL =>
                FIFO_READ_ENABLE <= '1';
                OFFLOAD_CTRL <= FIFO_DATA; -- Control
                offload_state <= READ_ID;

            When READ_ID =>
                FIFO_READ_ENABLE <= '1';
                OFFLOAD_ID <= FIFO_DATA(0) & FIFO_DATA(1) 
                & FIFO_DATA(2) & FIFO_DATA(3) 
                & FIFO_DATA(4) & FIFO_DATA(5) 
                & FIFO_DATA(6); -- Device ID :: Reverse concatenation
                offload_state <= READ_REGISTER;

            When READ_REGISTER =>
                FIFO_READ_ENABLE <= '0';
                OFFLOAD_REGISTER <= FIFO_DATA; -- Register
                offload_state <= READ_DATA;

            when READ_DATA =>
                OFFLOAD_DATA <= FIFO_DATA; -- Data
                OFFLOAD_READY <= '1';
                offload_state <= IDLE;

            when others =>
                offload_state <= IDLE;

        end case;
    end if;
end process;

end rtl;