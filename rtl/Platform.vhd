library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

----------------------------------------
-- Author: Ice.Marek
-- IceNET Technology 2024
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
----------------------------------------
entity Platform is
port
(
    CLOCK_50MHz : in std_logic; -- PIN_T2

    LED_1 : out std_logic; -- PIN_U7
    LED_2 : out std_logic; -- PIN_U8
    LED_3 : out std_logic; -- PIN_R7
    LED_4 : out std_logic; -- PIN_T8
    LED_5 : out std_logic; -- PIN_R8
    LED_6 : out std_logic; -- PIN_P8
    LED_7 : out std_logic; -- PIN_M8
    LED_8 : out std_logic; -- PIN_N8

    KERNEL_CS : in std_logic;    -- PIN_A5   :: BBB P9_17 :: PULPLE  :: SPI0_CS0
    KERNEL_MOSI : in std_logic;  -- PIN_A7   :: BBB P9_18 :: BLUE    :: SPI0_D1
    KERNEL_MISO : out std_logic; -- PIN_A6   :: BBB P9_21 :: BROWN   :: SPI0_D0
    KERNEL_SCLK : in std_logic;  -- PIN_A8   :: BBB P9_22 :: BLACK   :: SPI0_SCLK

    I2C_SDA : inout std_logic; -- PIN_A9   :: BBB P9_20 :: CPU.BLUE <> FPGA.BLUE <> GYRO.WHITE
    I2C_SCK : inout std_logic; -- PIN_A10  :: BBB P9_19 :: CPU.ORANGE <> FPGA.GREEN <> GYRO.PURPLE
	 
	 I2C_SDA_TEST : in std_logic; -- PIN_B9 :: YELLOW
	 I2C_SCK_TEST : in std_logic; -- PIN_B10 :: GREEN
	 
    FPGA_INT : out std_logic;  -- PIN_A3   :: BBB P9_12 :: BLACK
    KERNEL_INT : in std_logic; -- PIN_A4   :: BBB P9_14 :: WHITE

    BUTTON_1 : in std_logic; -- PIN_H20  :: Reset
    BUTTON_2 : in std_logic; -- PIN_K19  :: Doesnt Work :: Broken Button or Incorrect Schematic
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic  -- PIN_K18
);
end Platform;

architecture rtl of Platform is

----------------------------------------------------------------------------------------------------------------
-- Signals
----------------------------------------------------------------------------------------------------------------

-- Reset
signal reset_button : std_logic := '0';
-- SM Init
signal system_start : std_logic := '0';
-- Interrupt
signal kernel_interrupt : std_logic := '0';

-- Interrupt Pulse Generator
signal interrupt_divider : integer := 2;
signal interrupt_period : std_logic_vector(25 downto 0) := "10111110101111000001111111";
signal interrupt_length : std_logic_vector(3 downto 0) := "1111";
signal interrupt_signal : std_logic := '0';

-- I2C & SPA Data
signal sda_index : integer range 0 to 15 := 0;
constant data_SPI : std_logic_vector(7 downto 0) := "10001000"; -- 0x88
constant address_I2C : std_logic_vector(9 downto 0) := "0010011110"; -- 0x69

-- SPI Synchronise
signal synced_sclk : std_logic := '0';
signal synced_cs : std_logic := '0';
signal synced_mosi : std_logic := '0';
signal synced_miso : std_logic := '0';

-- Timers
signal system_timer : std_logic_vector(26 downto 0) := (others => '0');
signal idle_timer : std_logic_vector(24 downto 0) := (others => '0');
signal init_timer : std_logic_vector(24 downto 0) := (others => '0');
signal config_timer : std_logic_vector(24 downto 0) := (others => '0');
signal send_timer : std_logic_vector(24 downto 0) := (others => '0');
signal done_timer : std_logic_vector(24 downto 0) := (others => '0');
signal byte_timer : std_logic_vector(7 downto 0) := (others => '0');
signal bit_timer : std_logic_vector(3 downto 0) := (others => '0');
signal sck_timer : std_logic_vector(11 downto 0) := (others => '0');
signal sda_timer : std_logic_vector(11 downto 0) := (others => '0');

--i2c state machine
type TX is (IDLE, INIT, CONFIG, SEND, DONE, RECEIVE);
signal tx_current_state, tx_next_state: TX := IDLE;

-- i2c signals 
signal write_sda : std_logic := '0';
signal write_sck : std_logic := '0';
signal read_sda : std_logic := '1';
signal read_sck : std_logic := '1';

-- Interrupt
signal diode_check : std_logic := '0';
signal diode_done : std_logic := '0';

-- LED Signals
signal isIDLE : std_logic := '0';
signal isINIT : std_logic := '0';
signal isCONFIG : std_logic := '0';
signal isDEVICE : std_logic := '0';
signal isDONE : std_logic := '0';

----------------------------------------------------------------------------------------------------------------
-- COMPONENTS DECLARATION
----------------------------------------------------------------------------------------------------------------
component SPI_Synchronizer
port
(
    CLK_50MHz : in  std_logic;
    IN_SCLK : in  std_logic;
    IN_CS : in  std_logic;
    IN_MOSI : in  std_logic;
    OUT_SCLK : out std_logic;
    OUT_CS : out std_logic;
    OUT_MOSI : out std_logic
);
end component;

component Debounce
port
(
	clock : in  std_logic;
	button_in_1 : in  std_logic;
	button_in_2 : in  std_logic;
	button_in_3 : in  std_logic;
	button_in_4 : in  std_logic;
	button_out_1 : out std_logic;
	button_out_2 : out std_logic;
	button_out_3 : out std_logic;
	button_out_4 : out std_logic
);
end component;

component SPI_Data
Port 
(
    CLOCK : in  std_logic;
    DATA : in  std_logic_vector(7 downto 0);
    synced_sclk : in std_logic;
    synced_miso : out std_logic
);
end component;

component Interrupt
Port 
(
    CLOCK : in  std_logic;
    interrupt_period : in  std_logic_vector(25 downto 0);
    interrupt_length : in  std_logic_vector(3 downto 0);
    interrupt_signal : out std_logic
);
end component;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------
begin

SPI_Synchronizer_module: SPI_Synchronizer port map 
(
    CLK_50MHz => CLOCK_50MHz,
    IN_SCLK => KERNEL_SCLK,
    IN_CS => KERNEL_CS,
    IN_MOSI => KERNEL_MOSI,
    OUT_SCLK  => synced_sclk,
    OUT_CS => synced_cs,
    OUT_MOSI  => synced_mosi
);

Debounce_module: Debounce port map 
(
	clock => CLOCK_50MHz,
	button_in_1 => BUTTON_1,
	button_in_2 => BUTTON_2,
	button_in_3 => BUTTON_3,
	button_in_4 => BUTTON_4,
	button_out_1 => reset_button,
	button_out_2 => open,
	button_out_3 => open,
	button_out_4 => open
);

SPI_Data_module: SPI_Data port map 
(
	CLOCK => CLOCK_50MHz,
	DATA => data_SPI,
	synced_sclk => synced_sclk,
	synced_miso => synced_miso
);

KERNEL_MISO <= synced_miso;

----------------------------------------
-- Interrupt pulse :: 0x2FAF07F/50 MHz
-- (49999999 + 1)/50000000 Hz = 1 sec
--
-- This is adjsted by the divider
-- Currently divider = 2
-- Gives 250ms interrupt
--
-- Interrupt length :: 0xF
-- 16 * 2ns = 32 ns
----------------------------------------
Interrupt_module: Interrupt port map 
(
	CLOCK => CLOCK_50MHz,
	interrupt_period => std_logic_vector(unsigned(interrupt_period) srl interrupt_divider),
	interrupt_length => interrupt_length,
	interrupt_signal => interrupt_signal
);

interrupt_process:
process(CLOCK_50MHz, interrupt_signal, diode_check, diode_done)
begin
	if rising_edge(CLOCK_50MHz) then
		if interrupt_signal = '1' then
			if diode_done = '0' then
				diode_check <= not diode_check;
				diode_done 	<= '1';
			end if;
		else
			diode_done <= '0';
		end if;
	end if;
end process;

---------------------------------------------------------------------------------------
-- i2c communication protocol
---------------------------------------------------------------------------------------
-- Start.a0.a1.a2.a3.a4.a5.a6.Rd/Wr.Ack/Nak.d0.d1.d2.d3.d4.d5.d6.d7.Ack/Nak.Stop
--
-- Start == SDA >> High.to.Low before SCL High.to.Low
-- Write == '0'
-- Read  == '1'
-- Ack   == '0'
-- Nak   == '1'
-- Stop  == SDA >> Low.to.High after SCL Low.to.High
--
---------------------------------------------------------------------------------------
state_machine_process:
process(CLOCK_50MHz, reset_button, kernel_interrupt, tx_current_state, tx_next_state, system_start,
	system_timer, init_timer, config_timer, send_timer, done_timer, sck_timer, byte_timer,
	write_sda, write_sck, read_sck, read_sda,
	isIDLE, isINIT, isCONFIG, isDEVICE, isDONE)
begin
    if rising_edge(CLOCK_50MHz) then

    	kernel_interrupt <= KERNEL_INT;

        --------------------------------------------
        -- State Machine :: Start
        --------------------------------------------
		if system_start = '0' then
			if system_timer = "101111101011110000011111111" then
				system_start <= '1';
	            tx_next_state <= IDLE;
			else
				system_timer <= system_timer + '1';
			end if;
		else
	        ----------------------------------------
	        -- State Machine :: Reset
	        ----------------------------------------
	        if reset_button = '1' or kernel_interrupt <= '0' then
	        	tx_next_state <= INIT;
	    	else
		        ------------------------------------
		        -- State Machine :: IDLE
		        ------------------------------------
		    	if tx_current_state = IDLE then
	            	write_sda <= 'Z';
					write_sck <= 'Z';
					isIDLE <= '1';
					isINIT <= '0';
					isCONFIG <= '0';
					isDEVICE <= '0';
					isDONE <= '0';
				end if;
		        ------------------------------------
		        -- State Machine :: INIT
		        ------------------------------------
		        if tx_current_state = INIT then
		            if init_timer = "1011111010111100000111111" then -- delay for the reset to stabilise
		            	write_sda <= '1';
						write_sck <= '1';
		            	tx_next_state <= CONFIG;
		            else
		                init_timer <= init_timer + '1';
		            end if;
					isIDLE <= '0';
					isINIT <= '1';
					isCONFIG <= '0';
					isDEVICE <= '0';
					isDONE <= '0';
			    end if;
		        ------------------------------------
		        -- State Machine :: CONFIG
		        ------------------------------------
		        if tx_current_state = CONFIG then
		            if config_timer = "1011111010111100000111111" then
		            	tx_next_state <= SEND;
		            	----------------------------
		            	-- Body
		            	----------------------------
		            	byte_timer <= "11111001"; -- Clock bit @ Zero [0]
		            else
		                config_timer <= config_timer + '1';
		            end if;

					isIDLE <= '0';
					isINIT <= '0';
					isCONFIG <= '1';
					isDEVICE <= '0';
					isDONE <= '0';
			    end if;
		        ------------------------------------
		        -- State Machine :: SEND
		        ------------------------------------
		        if tx_current_state = SEND then
		        	if send_timer = "1011111010111100000111111" then
		        		--
		        		--
		        		-- Devel
		        		--
		        		--
			        	tx_next_state <= DONE;
					else
						send_timer <= send_timer + '1';
					end if;

					isIDLE <= '0';
					isINIT <= '0';
					isCONFIG <= '0';
					isDEVICE <= '1';
					isDONE <= '0';
			    end if;
		        ------------------------------------
		        -- State Machine :: DONE
		        ------------------------------------
		        if tx_current_state = DONE then
		            if done_timer = "1011111010111100000111111" then
		            	-- Reset Timers
		            	sda_timer <= (others => '0');
		            	sck_timer <= (others => '0');
		            	byte_timer <= (others => '0');
		            	bit_timer <= (others => '0');
		                init_timer <= (others => '0');
		                config_timer <= (others => '0');
		            	send_timer <= (others => '0');
		            	done_timer <= (others => '0');
			        	tx_next_state <= IDLE;
		            else
		                done_timer <= done_timer + '1';
		            end if;
		            
					isIDLE <= '0';
					isINIT <= '0';
					isCONFIG <= '0';
					isDEVICE <= '0';
					isDONE <= '1';
			    end if;
		        ------------------------------------
		        -- State Machine :: Update
		        ------------------------------------
		        tx_current_state <= tx_next_state;
		        ------------------------------------
		        -- State Machine :: Output
		        ------------------------------------
				I2C_SCK <= write_sck;
				I2C_SDA <= write_sda;
				read_sck <= I2C_SCK_TEST;
				read_sda <= I2C_SDA_TEST;
		        ------------------------------------
		        -- State Machine :: Status
		        ------------------------------------
		    	LED_1 <= isIDLE;
		    	LED_2 <= isINIT;
		    	LED_3 <= isCONFIG;
		    	LED_4 <= isDEVICE;
		    	LED_5 <= isDONE;
		    	LED_6 <= '0';
		    	LED_7 <= '0';
		    	LED_8 <= '0';
	        end if;
    	end if;
    end if;
end process;

-----------------------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------------------
FPGA_INT <= '0'; --interrupt_signal;

end rtl;


