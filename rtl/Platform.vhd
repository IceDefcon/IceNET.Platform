library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

------------------------------
-- Author: Ice.Marek
-- IceNET Technology 2024
--
-- FPGA Chip
-- Cyclone IV
-- EP4CE15F23C8
------------------------------
entity Platform is
port
(
    CLOCK_50MHz : in std_logic;     -- PIN_T2

    -- For debuging
    LED_1 : out std_logic;    -- PIN_U7
    LED_2 : out std_logic;    -- PIN_U8
    LED_3 : out std_logic;    -- PIN_R7
    LED_4 : out std_logic;    -- PIN_T8
    LED_5 : out std_logic;    -- PIN_R8
    LED_6 : out std_logic;    -- PIN_P8
    LED_7 : out std_logic;    -- PIN_M8
    LED_8 : out std_logic;    -- PIN_N8

    KERNEL_CS : in std_logic;    -- PIN_A5   :: BBB P9_17 :: PULPLE  :: SPI0_CS0
    KERNEL_MOSI : in std_logic;  -- PIN_A7   :: BBB P9_18 :: BLUE        :: SPI0_D1
    KERNEL_MISO : out std_logic; -- PIN_A6   :: BBB P9_21 :: BROWN   :: SPI0_D0
    KERNEL_SCLK : in std_logic;  -- PIN_A8   :: BBB P9_22 :: BLACK   :: SPI0_SCLK

    I2C_IN_SDA : in std_logic;  -- PIN_A9   :: BBB P9_20 :: BLUE
    I2C_IN_SCK : in std_logic;  -- PIN_A10  :: BBB P9_19 :: GREEN

    I2C_OUT_SDA : out std_logic;  -- PIN_B9   ::           :: RED
    I2C_OUT_SCK : out std_logic;  -- PIN_B10  ::           :: ORANGE

    FPGA_INT : out std_logic;   -- PIN_A3   :: BBB P9_12 :: BLACK
    KERNEL_INT : in std_logic;  -- PIN_A4   :: BBB P9_14 :: WHITE

    BUTTON_1 : in std_logic;    -- PIN_H20  :: Reset
    BUTTON_2 : in std_logic;    -- PIN_K19  :: Doesnt Work :: Broken Button or Incorrect Schematic
    BUTTON_3 : in std_logic;    -- PIN_J18
    BUTTON_4 : in std_logic     -- PIN_K18
);
end Platform;

architecture rtl of Platform is

-- Kernel interrupt
signal kernel_interrupt 		: std_logic := '0';

-- Interrupt Pulse Generator
signal interrupt_divider 		: integer := 2;
signal interrupt_period 		: std_logic_vector(25 downto 0) := "10111110101111000001111111";
signal interrupt_length 		: std_logic_vector(3 downto 0) := "1111";
signal interrupt_signal 		: std_logic := '0';

-- SPI
constant DATA					: std_logic_vector(7 downto 0) := "10001000"; -- 0x88

-- SPI Synchronise
signal synced_sclk  			: std_logic := '0';
signal synced_cs    			: std_logic := '0';
signal synced_mosi  			: std_logic := '0';
signal synced_miso  			: std_logic := '0';

-- Reset Button
signal reset_button			: std_logic := '0';

-- i2c clocks
signal i2c_clock : std_logic := '0';
signal i2c_clock_next : std_logic := '0';
signal i2c_clock_delay : std_logic_vector(7 downto 0) := (others => '0');
signal i2c_clock_align : std_logic := '0';
signal i2c_clock_first : std_logic := '0';
signal i2c_clock_last : std_logic := '0';

-- i2c initialise time
signal init_delay : std_logic_vector(26 downto 0) := (others => '0');
signal config_delay : std_logic_vector(26 downto 0) := (others => '0');
signal device_delay : std_logic_vector(26 downto 0) := (others => '0');

--i2c state machine
type TX is (IDLE, INIT, CONFIG, SEND, RECEIVE);
signal tx_current_state, tx_next_state: TX := IDLE;

-- i2c write clock
signal write_clock : std_logic := '1';
signal write_go : std_logic := '0';
signal write_count : std_logic_vector(12 downto 0) := (others => '0');

-- i2c debug
signal sda : std_logic := '0';
signal sck : std_logic := '0';

-- Debug Interrupt
signal diode_check : std_logic := '0';
signal diode_done : std_logic := '0';

-- State Machine Init
signal sm_run : std_logic := '0';
signal sm_delay : std_logic_vector(26 downto 0) := (others => '0');

-----------------------------------
--
-- Ice Debug
--
-- State Machine Flags
--
-----------------------------------
signal isIDLE : std_logic := '0';
signal isINIT : std_logic := '0';
signal isCONFIG : std_logic := '0';
signal isDEVICE : std_logic := '0';


----------------------------
-- COMPONENTS DECLARATION
----------------------------
component SPI_Synchronizer
port
(
    CLK_50MHz : in  std_logic;
    IN_SCLK   : in  std_logic;
    IN_CS     : in  std_logic;
    IN_MOSI   : in  std_logic;
    OUT_SCLK  : out std_logic;
    OUT_CS    : out std_logic;
    OUT_MOSI  : out std_logic
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
    CLOCK 		: in  std_logic;
    DATA 		: in  std_logic_vector(7 downto 0);
    synced_sclk : in std_logic;
    synced_miso : out std_logic
);
end component;

component Interrupt
Port 
(
    CLOCK 				: in  std_logic;
    interrupt_period 	: in  std_logic_vector(25 downto 0);
    interrupt_length 	: in  std_logic_vector(3 downto 0);
    interrupt_signal 	: out std_logic
);
end component;

------------------
-- MAIN ROUTINE
------------------
begin

SPI_Synchronizer_module: SPI_Synchronizer port map 
(
    CLK_50MHz => CLOCK_50MHz,
    IN_SCLK   => KERNEL_SCLK,
    IN_CS     => KERNEL_CS,
    IN_MOSI   => KERNEL_MOSI,
    OUT_SCLK  => synced_sclk,
    OUT_CS    => synced_cs,
    OUT_MOSI  => synced_mosi
);

Debounce_module: Debounce port map 
(
	clock 		=> CLOCK_50MHz,
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
	CLOCK 			=> CLOCK_50MHz,
	DATA 			=> DATA,
	synced_sclk 	=> synced_sclk,
	synced_miso 	=> synced_miso
);

KERNEL_MISO <= synced_miso;

------------------------------------------
-- Interrupt pulse :: 0x2FAF07F/50 MHz
-- (49999999 + 1)/50000000 Hz = 1 sec
--
-- This is adjsted by the divider
-- Currently divider = 2
-- Gives 250ms interrupt
--
-- Interrupt length :: 0xF
-- 16 * 2ns = 32 ns
------------------------------------------
Interrupt_module: Interrupt port map 
(
	CLOCK 				=> CLOCK_50MHz,
	interrupt_period 	=> std_logic_vector(unsigned(interrupt_period) srl interrupt_divider),
	interrupt_length 	=> interrupt_length,
	interrupt_signal 	=> interrupt_signal
);

----------------------
-- DEBUG PROCESS
----------------------

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
clock_free_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
        if i2c_clock_delay = "11111001" then
            i2c_clock <= not i2c_clock;
            i2c_clock_delay <= (others => '0'); -- Reset count
        else
            i2c_clock_delay <= i2c_clock_delay + '1';
        end if;

        -- Assign delayed clocks unconditionally on every clock edge
        i2c_clock_next <= i2c_clock;

    end if;
end process;

state_machine_process:
process(CLOCK_50MHz, reset_button, i2c_clock, i2c_clock_next, i2c_clock_align, write_clock,
	sm_delay, init_delay, config_delay, device_delay, sm_run,
	tx_current_state, tx_next_state, 
	isIDLE, isINIT, isCONFIG, isDEVICE)
begin
    if rising_edge(CLOCK_50MHz) then

    	kernel_interrupt <= KERNEL_INT;

        ----------------------------------------
        -- Reset State Machine
        ----------------------------------------
    	if sm_run = '0' then
		    if sm_delay = "101111101011110000011111111" then -- 2s delay
				sm_delay <= (others => '0');
				sm_run <= '1';

				-- Ice Debug
				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '0';
		    else
		        sm_delay <= sm_delay + '1';
		    end if;
    	elsif sm_run = '1' then

	        ----------------------------------------
	        -- State Machine :: IDLE Process
	        ----------------------------------------
	        if tx_current_state = IDLE then
		        if reset_button = '1' or kernel_interrupt <= '0' then
            		tx_next_state <= INIT;
		        end if;

				isIDLE <= '1';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '0';
		    end if;

	        ----------------------------------------
	        -- State Machine :: INIT Process
	        ----------------------------------------
	        if tx_current_state = INIT then
	            if init_delay = "010111110101111000001111111" then
	                init_delay <= (others => '0');
	            	tx_next_state <= CONFIG;
	            else
	                init_delay <= init_delay + '1';
	            end if;
	            
				isIDLE <= '0';
				isINIT <= '1';
				isCONFIG <= '0';
				isDEVICE <= '0';
		    end if;

	        ----------------------------------------
	        -- State Machine :: CONFIG Process
	        ----------------------------------------
	        if tx_current_state = CONFIG then
	            if config_delay = "010111110101111000001111111" then
	                config_delay <= (others => '0');
	            	tx_next_state <= SEND;
	            else
	                config_delay <= config_delay + '1';
	            end if;

				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '1';
				isDEVICE <= '0';
		    end if;

	        ----------------------------------------
	        -- State Machine :: SEND Process
	        ----------------------------------------
	        if tx_current_state = SEND then
			    if i2c_clock = '1' then 	-- Align @ 1 to achieve cycle from '0'
			        i2c_clock_align <= '1'; -- Stay HI until reset
			    end if;

			    -- Clock is aligned
			    if i2c_clock_align = '1' then
					if write_go = '0' and i2c_clock = '0' then
						write_go <= '1';
					end if;

					if write_go = '1' then
						if write_count = "1110101001100" then
			                write_count <= (others => '0');
			                write_go <= '0';
			            	tx_next_state <= IDLE;

			                i2c_clock_align <= '0'; -- Reset Alignment
							write_clock <= '0'; 	-- Pull up after TX is complete

			                --sm_run <= '0'; 			-- Hold State Machine :: Debug
						else
							write_clock <= i2c_clock_next; 	-----===[ OUT ]===-----
							write_count <= write_count + '1';
						end if;
					end if;
			    end if;

				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '1';
		    end if;

	        ----------------------------------------
	        -- SM :: State update
	        ----------------------------------------
	        tx_current_state <= tx_next_state;

	        ----------------------------------------
	        -- SM :: Current LED State
	        ----------------------------------------
	        LED_1 <= isIDLE;
        	LED_2 <= isINIT;
        	LED_3 <= isCONFIG;
        	LED_4 <= isDEVICE;

		end if;
    end if;
end process;


i2c_process:
process(CLOCK_50MHz, write_clock, sda, sck)
begin
	if rising_edge(CLOCK_50MHz) then
		I2C_OUT_SCK <= write_clock;
		I2C_OUT_SDA <= '0';
		sck <= I2C_IN_SCK;
		sda <= I2C_IN_SDA;
	end if;
end process;

-- --------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------
FPGA_INT <= '0'; -- interrupt_signal;

end rtl;


