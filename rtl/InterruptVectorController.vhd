library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity InterruptVectorController is
Port
(
    CLOCK_50MHz : in  std_logic;
    RESET : in  std_logic;

    PARALLEL_PRIMARY_MOSI : in std_logic_vector(7 downto 0);
    PARALLEL_CONVERSION_COMPLETE : in std_logic;

    VECTOR_INTERRUPT_EXTERNAL_OFFLOAD : out std_logic;
    VECTOR_INTERRUPT_PRIMARY_OFFLOAD : out std_logic;
    VECTOR_INTERRUPT_ENABLE : out std_logic;
    VECTOR_INTERRUPT_START : out std_logic;
    VECTOR_INTERRUPT_TRIGGER : out std_logic;

    FIFO_PARALLEL_PRIMARY_MOSI : out std_logic_vector(7 downto 0);
    FIFO_PARALLEL_PRIMARY_WR_EN : out std_logic
);
end entity InterruptVectorController;

architecture rtl of InterruptVectorController is

type VECTOR_TYPE is
(
    VECTOR_IDLE,
    VECTOR_RESERVED,            -- "0000"
    VECTOR_OFFLOAD_PRIMARY,     -- "0001"
    VECTOR_ENABLE,              -- "0010"
    VECTOR_DISABLE,             -- "0011"
    VECTOR_START,               -- "0100"
    VECTOR_STOP,                -- "0101"
    VECTOR_OFFLOAD_EXTERNAL,    -- "0110"
    VECTOR_TRIGGER,             -- "0111"
    VECTOR_F1,                  -- "1000"
    VECTOR_F2,                  -- "1001"
    VECTOR_F3,                  -- "1010"
    VECTOR_UNUSED_11,           -- "1011"
    VECTOR_UNUSED_12,           -- "1100"
    VECTOR_UNUSED_13,           -- "1101"
    VECTOR_UNUSED_14,           -- "1110"
    VECTOR_UNUSED_15,           -- "1111"
    VECTOR_DONE
);
signal vector_state: VECTOR_TYPE := VECTOR_IDLE;

signal interrupt_vector : std_logic_vector(3 downto 0) := (others => '0');
signal interrupt_vector_busy : std_logic := '0';

signal REG_primary_fifo_wr_en : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_0 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_1 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_2 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_3 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_4 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_5 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_6 : std_logic_vector(2047 downto 0) := (others => '0');
signal REG_primary_parallel_MOSI_7 : std_logic_vector(2047 downto 0) := (others => '0');

signal primary_conversion_run : std_logic := '0';
signal primary_conversion_reset : integer range 0 to 2048 := 0;
signal primary_conversion_count : integer range 0 to 256 := 0;

signal STAGE_1_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');
signal STAGE_2_primary_parallel_MOSI : std_logic_vector(7 downto 0) := (others => '0');

begin

    interrupt_process: process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            VECTOR_INTERRUPT_PRIMARY_OFFLOAD <= '0';
            VECTOR_INTERRUPT_EXTERNAL_OFFLOAD <= '0';
            VECTOR_INTERRUPT_ENABLE <= '0';
            VECTOR_INTERRUPT_START <= '0';
            interrupt_vector_busy <= '0';
            primary_conversion_run <= '0';
            interrupt_vector <= (others => '0');
            REG_primary_fifo_wr_en <= (others => '0');
            REG_primary_parallel_MOSI_0 <= (others => '0');
            REG_primary_parallel_MOSI_1 <= (others => '0');
            REG_primary_parallel_MOSI_2 <= (others => '0');
            REG_primary_parallel_MOSI_3 <= (others => '0');
            REG_primary_parallel_MOSI_4 <= (others => '0');
            REG_primary_parallel_MOSI_5 <= (others => '0');
            REG_primary_parallel_MOSI_6 <= (others => '0');
            REG_primary_parallel_MOSI_7 <= (others => '0');
            STAGE_1_primary_parallel_MOSI <= (others => '0');
            STAGE_2_primary_parallel_MOSI <= (others => '0');
            primary_conversion_reset <= 0;
            primary_conversion_count <= 0;
            vector_state <= VECTOR_IDLE;
        elsif rising_edge(CLOCK_50MHz) then
            ----------------------------------------------------------------------------------------------------------------------
            --
            -- Logic Ligic Logic
            --
            ----------------------------------------------------------------------------------------------------------------------
            REG_primary_fifo_wr_en <= REG_primary_fifo_wr_en(2046 downto 0) & PARALLEL_CONVERSION_COMPLETE;
            REG_primary_parallel_MOSI_7 <= REG_primary_parallel_MOSI_0(2046 downto 0) & PARALLEL_PRIMARY_MOSI(7);
            REG_primary_parallel_MOSI_6 <= REG_primary_parallel_MOSI_1(2046 downto 0) & PARALLEL_PRIMARY_MOSI(6);
            REG_primary_parallel_MOSI_5 <= REG_primary_parallel_MOSI_2(2046 downto 0) & PARALLEL_PRIMARY_MOSI(5);
            REG_primary_parallel_MOSI_4 <= REG_primary_parallel_MOSI_3(2046 downto 0) & PARALLEL_PRIMARY_MOSI(4);
            REG_primary_parallel_MOSI_3 <= REG_primary_parallel_MOSI_4(2046 downto 0) & PARALLEL_PRIMARY_MOSI(3);
            REG_primary_parallel_MOSI_2 <= REG_primary_parallel_MOSI_5(2046 downto 0) & PARALLEL_PRIMARY_MOSI(2);
            REG_primary_parallel_MOSI_1 <= REG_primary_parallel_MOSI_6(2046 downto 0) & PARALLEL_PRIMARY_MOSI(1);
            REG_primary_parallel_MOSI_0 <= REG_primary_parallel_MOSI_7(2046 downto 0) & PARALLEL_PRIMARY_MOSI(0);
            ----------------------------------------------------------------------------------------------------------------------
            --
            --
            --
            ----------------------------------------------------------------------------------------------------------------------
            if PARALLEL_CONVERSION_COMPLETE = '1' or REG_primary_fifo_wr_en(2047) = '1' then
                if primary_conversion_count < 2 then
                    STAGE_2_primary_parallel_MOSI <= PARALLEL_PRIMARY_MOSI;
                    STAGE_1_primary_parallel_MOSI <= STAGE_2_primary_parallel_MOSI;
                end if;
                primary_conversion_reset <= 0;
                primary_conversion_run <= '1';
                primary_conversion_count <= primary_conversion_count + 1;
            else
                if primary_conversion_reset = 2000 then
                    interrupt_vector_busy <= '0';
                    primary_conversion_run <= '0';
                    primary_conversion_reset <= 0;
                    primary_conversion_count <= 0;
                    STAGE_2_primary_parallel_MOSI <= (others => '0');
                    STAGE_1_primary_parallel_MOSI <= (others => '0');
                else
                    primary_conversion_reset <= primary_conversion_reset + 1;
                end if;
            end if;
            ----------------------------------------------------------------------------------------------------------------------
            --
            -- IRQ Vector Condition
            --
            ----------------------------------------------------------------------------------------------------------------------
            if STAGE_1_primary_parallel_MOSI(7) =  '1'  ---------------------------------
            and STAGE_1_primary_parallel_MOSI(2) =  '1' ----===[ IRQ Vector Base ]===----
            and STAGE_1_primary_parallel_MOSI(1) =  '1' ---------------------------------
            and STAGE_2_primary_parallel_MOSI = "10101111" -- [Vector, 0xAF, 0xAE, 0xAD]
            and interrupt_vector_busy = '0'
            then
                interrupt_vector <= STAGE_1_primary_parallel_MOSI(6 downto 3);
                interrupt_vector_busy <= '1';
            else
                interrupt_vector <= "0000";
            end if;
            ----------------------------------------------------------------------------------------------------------------------
            --
            -- If an IRQ Vector condition are meet
            -- Data from the Primary DMA is not
            -- Processed further via FIFO
            --
            -- 4-Bit IRQ Vector is therefor processed
            --
            ----------------------------------------------------------------------------------------------------------------------
            if interrupt_vector_busy = '1' then
                FIFO_PARALLEL_PRIMARY_MOSI <= (others => '0');
                FIFO_PARALLEL_PRIMARY_WR_EN <= '0';
            else
                FIFO_PARALLEL_PRIMARY_MOSI <= REG_primary_parallel_MOSI_0(2047) & REG_primary_parallel_MOSI_1(2047) &
                                                REG_primary_parallel_MOSI_2(2047) & REG_primary_parallel_MOSI_3(2047) &
                                                REG_primary_parallel_MOSI_4(2047) & REG_primary_parallel_MOSI_5(2047) &
                                                REG_primary_parallel_MOSI_6(2047) & REG_primary_parallel_MOSI_7(2047);

                FIFO_PARALLEL_PRIMARY_WR_EN <= REG_primary_fifo_wr_en(2047);
            end if;
            ----------------------------------------------------------------------------------------------------------------------
            --
            --
            --
            --
            --
            ----------------------------------------------------------------------------------------------------------------------
            case vector_state is
                when VECTOR_IDLE =>
                    if interrupt_vector = "0001" then
                        vector_state <= VECTOR_OFFLOAD_PRIMARY;
                    elsif interrupt_vector = "0010" then
                        vector_state <= VECTOR_ENABLE;
                    elsif interrupt_vector = "0011" then
                        vector_state <= VECTOR_DISABLE;
                    elsif interrupt_vector = "0100" then
                        vector_state <= VECTOR_START;
                    elsif interrupt_vector = "0101" then
                        vector_state <= VECTOR_STOP;
                    elsif interrupt_vector = "0110" then
                        vector_state <= VECTOR_OFFLOAD_EXTERNAL;
                    elsif interrupt_vector = "0111" then
                        vector_state <= VECTOR_TRIGGER;
                    elsif interrupt_vector = "1000" then
                        vector_state <= VECTOR_F1;
                    elsif interrupt_vector = "1001" then
                        vector_state <= VECTOR_F2;
                    elsif interrupt_vector = "1010" then
                        vector_state <= VECTOR_F3;
                    elsif interrupt_vector = "1011" then
                        vector_state <= VECTOR_UNUSED_11;
                    elsif interrupt_vector = "1100" then
                        vector_state <= VECTOR_UNUSED_12;
                    elsif interrupt_vector = "1101" then
                        vector_state <= VECTOR_UNUSED_13;
                    elsif interrupt_vector = "1110" then
                        vector_state <= VECTOR_UNUSED_14;
                    elsif interrupt_vector = "1111" then
                        vector_state <= VECTOR_UNUSED_15;
                    end if;

                when VECTOR_RESERVED =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_OFFLOAD_PRIMARY =>
                    VECTOR_INTERRUPT_PRIMARY_OFFLOAD <= '1';
                    vector_state <= VECTOR_DONE;

                when VECTOR_ENABLE =>
                    VECTOR_INTERRUPT_ENABLE <= '1';
                    vector_state <= VECTOR_DONE;

                when VECTOR_DISABLE =>
                    VECTOR_INTERRUPT_ENABLE <= '0';
                    vector_state <= VECTOR_DONE;

                when VECTOR_START =>
                    VECTOR_INTERRUPT_START <= '1';
                    vector_state <= VECTOR_DONE;

                when VECTOR_STOP =>
                    VECTOR_INTERRUPT_START <= '0';
                    vector_state <= VECTOR_DONE;

                when VECTOR_OFFLOAD_EXTERNAL =>
                    VECTOR_INTERRUPT_EXTERNAL_OFFLOAD <= '1';
                    vector_state <= VECTOR_DONE;

                when VECTOR_TRIGGER =>
                    VECTOR_INTERRUPT_TRIGGER <= '1';
                    vector_state <= VECTOR_DONE;

                when VECTOR_F1 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_F2 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_F3 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_UNUSED_11 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_UNUSED_12 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_UNUSED_13 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_UNUSED_14 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_UNUSED_15 =>
                    vector_state <= VECTOR_DONE;

                when VECTOR_DONE =>
                    VECTOR_INTERRUPT_EXTERNAL_OFFLOAD <= '0';
                    VECTOR_INTERRUPT_PRIMARY_OFFLOAD <= '0';
                    VECTOR_INTERRUPT_TRIGGER <= '0';
                    vector_state <= VECTOR_IDLE;

                when others =>
                    vector_state <= VECTOR_IDLE;
            end case;
        end if;
    end process;
end architecture rtl;
