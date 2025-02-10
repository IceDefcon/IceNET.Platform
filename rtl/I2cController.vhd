library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity I2cController is
port
(    
    CLOCK : in std_logic;
    RESET : in std_logic;

    OFFLOAD_INT : in std_logic;
    KERNEL_INT : in std_logic;
    FPGA_INT : out std_logic; -- 1000*20ns interrupt back to Kernel
    FIFO_INT : out std_logic;
    
    I2C_SCK : inout std_logic;
    I2C_SDA : inout std_logic;

    OFFLOAD_ID : in std_logic_vector(6 downto 0);
    OFFLOAD_REGISTER : in std_logic_vector(7 downto 0);
    OFFLOAD_CONTROL : in std_logic;
    OFFLOAD_DATA : in std_logic_vector(7 downto 0);

    OFFLOAD_WAIT : out std_logic;
    FEEDBACK_DATA : out std_logic_vector(7 downto 0)
);
end I2cController;

architecture rtl of I2cController is

----------------------------------------------------------------------------------------------------------------
-- Signals
----------------------------------------------------------------------------------------------------------------

-- SM interrupt
signal kernel_interrupt : std_logic := '0';
-- SM Init
signal system_start : std_logic := '0';
--SM Parameters
signal rw : std_logic := '0';
constant smStartDelay  : std_logic_vector(15 downto 0):= "0000000000110001"; -- 1us
constant smInitDelay   : std_logic_vector(15 downto 0):= "0000000000110001"; -- 1us
constant smConfigDelay : std_logic_vector(15 downto 0):= "0000000000110001"; -- 1us
constant smReadDelay   : std_logic_vector(15 downto 0):= "0110000110100111"; -- 500us
constant smWriteDelay  : std_logic_vector(15 downto 0):= "0110000110100111"; -- 500us
constant smDoneDelay   : std_logic_vector(15 downto 0):= "0000000000110001"; -- 1us

-- SM Status Register
signal status_sck : std_logic_vector(3 downto 0) := "0000";
signal status_sda : std_logic_vector(3 downto 0) := "0000";
-- Data Index
signal index : integer range 0 to 15 := 0;
-- Delay Timers
signal system_timer : std_logic_vector(15 downto 0) := (others => '0');
signal init_timer : std_logic_vector(15 downto 0) := (others => '0');
signal config_timer : std_logic_vector(15 downto 0) := (others => '0');
signal send_timer : std_logic_vector(15 downto 0) := (others => '0');
signal done_timer : std_logic_vector(15 downto 0) := (others => '0');
-- Process Timers
signal status_timer : std_logic_vector(15 downto 0) := (others => '0');
signal sck_timer : std_logic_vector(7 downto 0) := (others => '0');
signal sda_timer : std_logic_vector(8 downto 0) := (others => '0');
signal sck_timer_toggle : std_logic := '0';
-- Parametric offset
signal sda_offset : std_logic_vector(15 downto 0) := (others => '0');
--I2c state machine
type STATE is 
(
    IDLE,
    INIT,
    CONFIG,
    RD,
    WR,
    DONE
);
--State machine indicators
signal i2c_state: STATE := IDLE;
-- fifo interrupt
signal fifo_interrupt : std_logic := '0';
signal fifo_flag : std_logic := '0';
----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin


state_machine_process : process(CLOCK, RESET)
begin
    if rising_edge(CLOCK) then

        --------------------------------------------
        -- Interrupt Vector
        --------------------------------------------
        kernel_interrupt <= KERNEL_INT;
        FIFO_INT <= fifo_interrupt;

        --------------------------------------------
        -- State Machine :: Start
        --------------------------------------------
        if system_start = '0' then
            if system_timer = smStartDelay then
                system_start <= '1';
                i2c_state <= IDLE;
            else
                system_timer <= system_timer + '1';
            end if;
        else
            ----------------------------------------
            -- State Machine :: Reset
            ----------------------------------------
            if RESET = '1' or OFFLOAD_INT = '1' then
                OFFLOAD_WAIT <= '1';
                i2c_state <= INIT;
            else

                case i2c_state is
                    ------------------------------------
                    -- State Machine :: IDLE
                    ------------------------------------
                    when IDLE =>
                        OFFLOAD_WAIT <= '0';
                        I2C_SCK <= 'Z';
                        I2C_SDA <= 'Z';
                    ------------------------------------
                    -- State Machine :: INIT
                    ------------------------------------
                    when INIT =>
                        if init_timer = smInitDelay then -- delay for the reset to stabilise
                            i2c_state <= CONFIG;
                        else
                            init_timer <= init_timer + '1';
                        end if;
                    ------------------------------------
                    -- State Machine :: CONFIG
                    ------------------------------------
                    when CONFIG =>
                        if config_timer = smConfigDelay then
                            if OFFLOAD_CONTROL = '0' then
                                i2c_state <= RD;
                            else
                                i2c_state <= WR;
                            end if;
                            rw <= '0';
                            sck_timer <= "11111001"; -- Reset timer so SCK is inverted @ 1st clock cycle
                            sda_timer <= "111110011"; -- Reset timer so data is passed @ 1st clock cycle
                            sda_offset <= "0000000001100011"; -- [100-1] :: SDA Offset
                        else    
                            config_timer <= config_timer + '1';
                        end if;
                    ------------------------------------
                    -- State Machine :: RD
                    ------------------------------------
                    when RD =>
                        if send_timer = smReadDelay then
                            i2c_state <= DONE;
                        else
                            if status_timer = "1111111111111111" then -- Length :: 25k clock cycles :: -----===[ RESET ]===----
                            else
    ------------------------------------------------------
    -- PIPE[0] :: Read SCK Status Registers
    ------------------------------------------------------
                                if status_timer = "0000000111110011" then -- [500-1] :: Device Address
                                    status_sck <= "0001";
                                end if;

                                if status_timer = "0000111110011111" then -- [4000-1] :: Read
                                    status_sck <= "0010";
                                end if;

                                if status_timer = "0001000110010011" then -- [4500-1] :: ACK/NAK
                                    status_sck <= "0011";
                                end if;

                                if status_timer = "0001001110000111" then -- [5000-1] :: BARIER[1]
                                    status_sck <= "0100";
                                end if;

                                if status_timer = "0001101101010111" then -- [7000-1] :: Register Address
                                    status_sck <= "0101";
                                end if;

                                if status_timer = "0010101011110111" then -- [11000-1] :: ACK/NAK
                                    status_sck <= "0110";
                                end if;

                                if status_timer = "0010110011101011" then -- [11500-1] :: BARIER[2]
                                    status_sck <= "0111";
                                end if;

                                if status_timer = "0011010010111011" then -- [13500-1] :: INIT SDA RETURN
                                    status_sck <= "1000";
                                end if;

                                if status_timer = "0011011010101111" then -- [14000-1] :: Repeted Device Address
                                    status_sck <= "1001";
                                end if;

                                if status_timer = "0100010001011011" then -- [17500-1] :: Write
                                    status_sck <= "1010";
                                end if;

                                if status_timer = "0100011001001111" then -- [18000-1] :: ACK/NAK
                                    status_sck <= "1011";
                                end if;

                                if status_timer = "0100100001000011" then -- [18500-1] :: Data From Register
                                    status_sck <= "1100";
                                end if;

                                if status_timer = "0101011111100011" then -- [22500-1] :: ACK/NAK
                                    status_sck <= "1101";
                                end if;

                                if status_timer = "0101100111010111" then -- [23000-1] :: Additional Cycle
                                    status_sck <= "1110";
                                end if;

                                if status_timer = "0101101111001011" then -- [23500-1] :: Stop Bit
                                    status_sck <= "1111";
                                end if;
    ------------------------------------------------------
    -- PIPE[0] :: Read SDA Status Registers
    ------------------------------------------------------
                                if status_timer = sda_offset then -- [100-1] :: Start bit
                                    status_sda <= "0001";
                                end if;

                                if status_timer = sda_offset + "0000000111110100" then -- [500] :: Device Address
                                    status_sda <= "0010";
                                end if;

                                if status_timer = sda_offset + "0000111110100000" then -- [4000] :: Read
                                    status_sda <= "0011";
                                end if;

                                if status_timer = sda_offset + "0001000110010100" then -- [4500] :: ACK/NAK
                                    status_sda <= "0100";
                                end if;

                                if status_timer = sda_offset + "0001001110001000" then -- [5000] :: BARIER[1]
                                    status_sda <= "0101";
                                end if;

                                if status_timer = sda_offset + "0001101101011000" then -- [7000] :: Register Address
                                    status_sda <= "0110";
                                end if;

                                if status_timer = sda_offset + "0010101011111000" then -- [11000] :: ACK/NAK
                                    status_sda <= "0111";
                                end if;

                                if status_timer = sda_offset + "0010110011101100" then -- [11500] :: BARIER[2]
                                    status_sda <= "1000";
                                end if;

                                if status_timer = sda_offset + "0011010110110110" then -- [13750] :: REPEATED START
                                    status_sda <= "1001";
                                end if;

                                if status_timer = sda_offset + "0011011010110000" then -- [14000] :: Repeted Device Address
                                    status_sda <= "1010";
                                end if;

                                if status_timer = sda_offset + "0100010001011100" then -- [17500] :: Write
                                    status_sda <= "1011";
                                end if;

                                if status_timer = sda_offset + "0100011001010000" then -- [18000] :: ACK/NAK
                                    status_sda <= "1100";
                                end if;

                                if status_timer = sda_offset + "0100100001000100" then -- [18500] :: Data From Register
                                    status_sda <= "1101";
                                end if;

                                if status_timer = sda_offset + "0101011111100100" then -- [22500] :: ACK/NAK
                                    status_sda <= "1110";
                                end if;

                                if status_timer = sda_offset + "0101100111011000" then -- [23000] :: Stop Bit
                                    status_sda <= "1111";
                                end if;

                                if status_timer = sda_offset + "0101110111000000" then -- [24000] :: Final BARIER
                                    status_sda <= "0000";
                                end if;
    ------------------------------------------------------
    -- PIPE[1] :: Process SCK Status Register
    ------------------------------------------------------
                                if status_sck = "0001" -- [500-1] :: Device Address
                                or status_sck = "0010" -- [4000-1] :: Read
                                or status_sck = "0011" -- [4500-1] :: ACK/NAK
                                or status_sck = "0101" -- [7000-1] :: Register Address
                                or status_sck = "0110" -- [11000-1] :: ACK/NAK
                                or status_sck = "1001" -- [14000-1] :: Repeted Device Address
                                or status_sck = "1010" -- [17500-1] :: Write
                                or status_sck = "1011" -- [18000-1] :: ACK/NAK
                                or status_sck = "1100" -- [18500-1] :: Data From Register
                                or status_sck = "1101" -- [22500-1] :: ACK/NAK
                                or status_sck = "1110" -- [23000-1] :: Additional Cycle
                                then
                                    if sck_timer = "11111001" then -- Half bit time
                                        sck_timer_toggle <= not sck_timer_toggle;

                                        if sck_timer_toggle = '1' then
                                            sck_timer <= (others => '0');
                                            I2C_SCK <= '1';
                                        else
                                            sck_timer <= (others => '0');
                                            I2C_SCK <= '0';
                                        end if;
                                    else
                                        sck_timer <= sck_timer + '1';
                                    end if;
                                end if;

                                if status_sck = "0100" -- [5000-1] :: BARIER[1]
                                or status_sck = "0111" -- [11500-1] :: BARIER[2]
                                then
                                    I2C_SCK <= '0';
                                end if;

                                if status_sck = "1000" -- [13500-1] :: INIT SDA RETURN
                                or status_sck = "1111" -- [23500-1] :: Stop Bit
                                then
                                    I2C_SCK <= '1';
                                end if;
    ------------------------------------------------------
    -- PIPE[1] :: Process SDA Status Register
    ------------------------------------------------------
                                if status_sda = "0001" then -- [100-1] :: Start bit
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= '0';
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0010" then -- [500] :: Device Address
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_ID(index);
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0011" then -- [4000] :: Read
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= rw;
                                        index <= 0;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0110" then -- [7000] :: Register Address
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_REGISTER(7 - index);
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1001" then -- [13750] :: REPEATED START
                                    I2C_SDA <= '0';
                                end if;

                                if status_sda = "1010" then -- [14000] :: Repeted Device Address
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_ID(index);
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1011" then -- [17500] :: Write
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= not rw;
                                        index <= 0;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1101" then -- [18500] :: Data From Register
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        FEEDBACK_DATA(7 - index) <= I2C_SDA;
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1111" then -- [23000] :: Stop Bit
                                    FPGA_INT <= '1';

                                    if fifo_interrupt = '0' and fifo_flag = '0' then
                                        fifo_interrupt <= '1';
                                        fifo_flag <= '1';
                                    else
                                        fifo_interrupt <= '0';
                                    end if;

                                    I2C_SDA <= '0';
                                end if;

                                if status_sda = "0100" -- [4500] :: ACK/NAK
                                or status_sda = "0101" -- [5000] :: BARIER[1]
                                or status_sda = "0111" -- [11000] :: ACK/NAK
                                or status_sda = "1000" -- [11500] :: BARIER[2]
                                or status_sda = "1100" -- [18000] :: ACK/NAK
                                or status_sda = "1110" -- [22500] :: ACK/NAK
                                then -- BARIER :: 'Z'
                                    I2C_SDA <= 'Z';
                                end if;

                                if status_sda = "0000" -- [24000] :: Final BARIER
                                then -- BARIER :: 'Z'
                                    FPGA_INT <= '0';
                                    I2C_SDA <= 'Z';
                                end if;
    ------------------------------------------------------
    -- PIPE[1] :: Increment Status Timer
    ------------------------------------------------------
                                status_timer <= status_timer + '1';
                            end if;
                            send_timer <= send_timer + '1';
                        end if;
                    ------------------------------------
                    -- State Machine :: WR
                    ------------------------------------
                    when WR =>
                        if send_timer = smWriteDelay then
                            i2c_state <= DONE;
                        else
                            if status_timer = "1111111111111111" then -- Length :: 25k clock cycles :: -----===[ RESET ]===----
                            else
    ------------------------------------------------------
    -- PIPE[0] :: Read SCK Status Registers
    ------------------------------------------------------
                                if status_timer = "0000000111110011" then -- [500-1] :: Device Address
                                    status_sck <= "0001";
                                end if;

                                if status_timer = "0000111110011111" then -- [4000-1] :: Write
                                    status_sck <= "0010";
                                end if;

                                if status_timer = "0001000110010011" then -- [4500-1] :: ACK/NAK
                                    status_sck <= "0011";
                                end if;

                                if status_timer = "0001001110000111" then -- [5000-1] :: BARIER[1]
                                    status_sck <= "0100";
                                end if;

                                if status_timer = "0001101101010111" then -- [7000-1] :: Register Address
                                    status_sck <= "0101";
                                end if;

                                if status_timer = "0010101011110111" then -- [11000-1] :: ACK/NAK
                                    status_sck <= "0110";
                                end if;

                                if status_timer = "0010110011101011" then -- [11500-1] :: Register Data
                                    status_sck <= "0111";
                                end if;

                                if status_timer = "0011110010001011" then -- [15500-1] :: ACK/NAK
                                    status_sck <= "1000";
                                end if;

                                if status_timer = "0011111001111111" then -- [16000-1] :: Additional Cycle
                                    status_sck <= "1001";
                                end if;

                                if status_timer = "0100000001110011" then -- [16500-1] :: Stop Bit
                                    status_sck <= "1010";
                                end if;

    ------------------------------------------------------
    -- PIPE[0] :: Read SDA Status Registers
    ------------------------------------------------------
                                if status_timer = sda_offset then -- [100-1] :: Start bit
                                    status_sda <= "0001";
                                end if;

                                if status_timer = sda_offset + "0000000111110100" then -- [500] :: Device Address
                                    status_sda <= "0010";
                                end if;

                                if status_timer = sda_offset + "0000111110100000" then -- [4000] :: Write
                                    status_sda <= "0011";
                                end if;

                                if status_timer = sda_offset + "0001000110010100" then -- [4500] :: ACK/NAK
                                    status_sda <= "0100";
                                end if;

                                if status_timer = sda_offset + "0001001110001000" then -- [5000] :: BARIER[1]
                                    status_sda <= "0101";
                                end if;

                                if status_timer = sda_offset + "0001101101011000" then -- [7000] :: Register Address
                                    status_sda <= "0110";
                                end if;

                                if status_timer = sda_offset + "0010101011111000" then -- [11000] :: ACK/NAK
                                    status_sda <= "0111";
                                end if;

                                if status_timer = sda_offset + "0010110011101100" then -- [11500] :: Register Data
                                    status_sda <= "1000";
                                end if;

                                if status_timer = sda_offset + "0011110010001100" then -- [15500] :: ACK/NAK
                                    status_sda <= "1001";
                                end if;

                                if status_timer = sda_offset + "0011111010000000" then -- [16000] :: Stop Bit
                                    status_sda <= "1010";
                                end if;

                                if status_timer = sda_offset + "0100001001101000" then -- [17000] :: Final BARIER
                                    status_sda <= "1011";
                                end if;
     
    ------------------------------------------------------
    -- PIPE[1] :: Process SCK Status Register
    ------------------------------------------------------
                                if status_sck = "0001" -- [500-1] :: Device Address
                                or status_sck = "0010" -- [4000-1] :: Write
                                or status_sck = "0011" -- [4500-1] :: ACK/NAK
                                or status_sck = "0101" -- [7000-1] :: Register Address
                                or status_sck = "0110" -- [11000-1] :: ACK/NAK
                                or status_sck = "0111" -- [11500-1] :: Register Data
                                or status_sck = "1000" -- [15500-1] :: ACK/NAK
                                or status_sck = "1001" -- [16000-1] :: Additional Cycle
                                then
                                    if sck_timer = "11111001" then -- Half bit time
                                        sck_timer_toggle <= not sck_timer_toggle;

                                        if sck_timer_toggle = '1' then
                                            sck_timer <= (others => '0');
                                            I2C_SCK <= '1';
                                        else
                                            sck_timer <= (others => '0');
                                            I2C_SCK <= '0';
                                        end if;
                                    else
                                        sck_timer <= sck_timer + '1';
                                    end if;
                                end if;

                                if status_sck = "0100" -- [5000-1] :: BARIER[1]
                                then
                                    I2C_SCK <= '0';
                                end if;

                                if status_sck = "1010" -- [16000-1] :: Stop Bit
                                then
                                    I2C_SCK <= '1';
                                end if;
    ------------------------------------------------------
    -- PIPE[1] :: Process SDA Status Register
    ------------------------------------------------------
                                if status_sda = "0001" then -- [100-1] :: Start bit
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= '0';
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0010" then -- [500] :: Device Address
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_ID(index); -- Address Data 1
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0011" then -- [4000] :: Write
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= rw;
                                        index <= 0;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "0110" then -- [7000] :: Register Address
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_REGISTER(7 - index);
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1000" then -- [11500] :: Register Data
                                    if sda_timer = "111110011" then -- Half bit time
                                        sda_timer <= (others => '0');
                                        I2C_SDA <= OFFLOAD_DATA(7 - index);
                                        index <= index + 1;
                                    else
                                        sda_timer <= sda_timer + '1';
                                    end if;
                                end if;

                                if status_sda = "1010" then -- [16000] :: Stop Bit
                                    I2C_SDA <= '0';
                                    --FEEDBACK_DATA <= "01111110"; -- 0x7e
                                    FEEDBACK_DATA <= OFFLOAD_REGISTER;
                                    FPGA_INT <= '1';
                                end if;

                                if status_sda = "0100" -- [4500] :: ACK/NAK
                                or status_sda = "0101" -- [5000] :: BARIER[1]
                                or status_sda = "0111" -- [11000] :: ACK/NAK
                                or status_sda = "1001" -- [15500] :: ACK/NAK
                                then -- BARIER :: 'Z'
                                    I2C_SDA <= 'Z';
                                end if;

                                if status_sda = "1011" -- [17000] :: Final BARIER
                                then -- BARIER :: 'Z'
                                    I2C_SDA <= 'Z';
                                    FEEDBACK_DATA <= "00000000";
                                    FPGA_INT <= '0';
                                end if;
    ------------------------------------------------------
    -- PIPE[1] :: Increment Status Timer
    ------------------------------------------------------
                                status_timer <= status_timer + '1';
                            end if;
                            send_timer <= send_timer + '1';
                        end if;
                    ------------------------------------
                    -- State Machine :: DONE
                    ------------------------------------
                    when DONE =>
                        if done_timer = smDoneDelay then
                            -- Reset Timers
                            status_timer <= (others => '0');
                            sda_timer <= (others => '0');
                            sck_timer <= (others => '0');
                            init_timer <= (others => '0');
                            config_timer <= (others => '0');
                            send_timer <= (others => '0');
                            done_timer <= (others => '0');
                            -- Reset Status registers
                            status_sck <= "0000";
                            status_sda <= "0000";
                            -- Switch to IDLE
                            i2c_state <= IDLE;
                            -- Rest fifo interrupt flag
                            fifo_flag <= '0';
                        else
                            done_timer <= done_timer + '1';
                        end if;

                    when others =>
                        i2c_state <= IDLE;

                end case;
            end if;
        end if;
    end if;
end process;

end rtl;
