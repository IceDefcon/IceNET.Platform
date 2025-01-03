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
    INIT,
    READ_EN,
    HEADER
);
signal offload_state: STATE := IDLE;

signal header_flag : std_logic := '0';
signal header_size : integer := 0;

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
                	FIFO_READ_ENABLE <= '1';
                    offload_state <= INIT;
                else
                    offload_state <= IDLE;
                end if;

            when INIT =>
                FIFO_READ_ENABLE <= '1';

                if FIFO_DATA > "00000000" then
                    header_size <= to_integer(unsigned(FIFO_DATA)) - 1;
                end if;

            when HEADER =>
            	if header_flag = '0' then
            		header_size <= to_integer(unsigned(FIFO_DATA));
            		header_flag <= '1';
            	elsif header_size > 0 then
                	--FIFO_READ_ENABLE <= '1';
                	header_size <= header_size - 1;
                else
                	offload_state <= IDLE;
                end if;

            when others =>
                offload_state <= IDLE;

        end case;
    end if;
end process;

end rtl;
