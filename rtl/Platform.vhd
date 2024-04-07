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
    KERNEL_MISO : out std_logic; -- PIN_A6   :: BBB P9_21 :: BROWN   :: SPI0_D0
    KERNEL_MOSI : in std_logic;  -- PIN_A7   :: BBB P9_18 :: BLUE    :: SPI0_D1
    KERNEL_SCLK : in std_logic;  -- PIN_A8   :: BBB P9_22 :: BLACK   :: SPI0_SCLK

    I2C_SDA : inout std_logic; -- PIN_A9   :: BBB P9_20 :: CPU.BLUE <> FPGA.BLUE <> GYRO.WHITE
    I2C_SCK : inout std_logic; -- PIN_A10  :: BBB P9_19 :: CPU.ORANGE <> FPGA.GREEN <> GYRO.PURPLE
 
    FPGA_INT : out std_logic;  -- PIN_A3   :: BBB P9_12 :: BLACK
    KERNEL_INT : in std_logic; -- PIN_A4   :: BBB P9_14 :: WHITE

    BUTTON_1 : in std_logic; -- PIN_H20  :: Reset
    BUTTON_2 : in std_logic; -- PIN_K19  :: Doesnt Work :: Incorrect Schematic or Broken Button
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic  -- PIN_K18
);
end Platform;

architecture rtl of Platform is

----------------------------------------------------------------------------------------------------------------
-- Signals
----------------------------------------------------------------------------------------------------------------

-- SM Init
signal system_start : std_logic := '0';
-- SM Reset
signal reset_button : std_logic := '0';
signal kernel_interrupt : std_logic := '0';
--SM Parameters
constant smStartDelay : std_logic_vector(26 downto 0):= "101111101011110000011111111";
constant smStateDelay : std_logic_vector(24 downto 0):= "1011111010111100000111111";
-- SM Status Register
signal status_sck : std_logic_vector(3 downto 0) := "0000";
signal status_sda : std_logic_vector(3 downto 0) := "0000";

-- Interrupt Pulse Generator
signal interrupt_divider : integer := 2;
signal interrupt_period : std_logic_vector(25 downto 0) := "10111110101111000001111111"; -- [50*10^6 - 1]
signal interrupt_length : std_logic_vector(3 downto 0) := "1111";
signal interrupt_signal : std_logic := '0';

-- I2C & SPA Data
constant address_I2C : std_logic_vector(6 downto 0) := "1001011"; -- 0x69 ---> ID :: GOOD == 1001011 :: BAD == 1001111
constant register_I2C : std_logic_vector(7 downto 0) := "11110000"; -- 0x40 ---> REG
signal index : integer range 0 to 15 := 0;

-- I2C Return Data
signal return_data : std_logic_vector(7 downto 0) := "11100111";

-- SPI Kernel Feedback Data
signal SpiDataFeedback_MISO : std_logic := '0';

-- Delay Timers
signal system_timer : std_logic_vector(26 downto 0) := (others => '0');
signal init_timer : std_logic_vector(24 downto 0) := (others => '0');
signal config_timer : std_logic_vector(24 downto 0) := (others => '0');
signal send_timer : std_logic_vector(24 downto 0) := (others => '0');	
signal done_timer : std_logic_vector(24 downto 0) := (others => '0');

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
	SEND,
	DONE
);
signal state_current, state_next: STATE := IDLE;

-- Status Indicators
signal isIDLE : std_logic := '0';
signal isINIT : std_logic := '0';
signal isCONFIG : std_logic := '0';
signal isDEVICE : std_logic := '0';
signal isDONE : std_logic := '0';

-- L3G4200D Gyroscope registers
signal OUT_X_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_X_H : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Y_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Y_H : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Z_L : std_logic_vector(7 downto 0):= (others => '0');
signal OUT_Z_H : std_logic_vector(7 downto 0):= (others => '0');





----------------------------------------------------------------------------------------------------------------
-- COMPONENTS DECLARATION
----------------------------------------------------------------------------------------------------------------
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

component SpiDataFeedback
Port 
(
    CLOCK : in  std_logic;
    SCLK : in std_logic;
    DATA : in std_logic_vector(7 downto 0);
    synced_miso : out std_logic
);
end component;

component InterruptPulse
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

SpiDataFeedback_module: SpiDataFeedback port map 
(
	CLOCK => CLOCK_50MHz,
	SCLK => KERNEL_SCLK,
	DATA => return_data,
	synced_miso => SpiDataFeedback_MISO
);

KERNEL_MISO <= SpiDataFeedback_MISO;

------------------------------------------------------
-- Interrupt pulse :: 0x2FAF07F/50 MHz
-- (49999999 + 1)/50000000 Hz = 1 sec
--
-- Divide 0 :: 50000000 >> 0 :: 50000000*20ns = 1000ms
-- Divide 1 :: 50000000 >> 1 :: 25000000*20ns = 500ms
-- Divide 2 :: 50000000 >> 2 :: 12500000*20ns = 250ms
-- Divide 3 :: 50000000 >> 3 :: 6250000*20ns = 125ms
-- Divide 4 :: 50000000 >> 4 :: 3125000*20ns = 62.5ms
--
-- Interrupt length :: 0xF
-- 16 * 20ns = 320 ns
------------------------------------------------------
Interrupt_module: InterruptPulse port map 
(
	CLOCK => CLOCK_50MHz,
	interrupt_period => std_logic_vector(unsigned(interrupt_period) srl interrupt_divider),
	interrupt_length => interrupt_length,
	interrupt_signal => interrupt_signal
);

---------------------------------------------------------------------------------------
-- i2c communication protocol
---------------------------------------------------------------------------------------
state_machine_process:
process(CLOCK_50MHz, reset_button, kernel_interrupt, system_start, state_current, 
	system_timer, init_timer, config_timer, send_timer, done_timer, status_timer, 
	sck_timer_toggle, sda_offset)
begin
    if rising_edge(CLOCK_50MHz) then

    	kernel_interrupt <= KERNEL_INT;

        --------------------------------------------
        -- State Machine :: Start
        --------------------------------------------
		if system_start = '0' then
			if system_timer = smStartDelay then
				system_start <= '1';
	            state_next <= IDLE;
			else
				system_timer <= system_timer + '1';
			end if;
		else
	        ----------------------------------------
	        -- State Machine :: Reset
	        ----------------------------------------
	        if reset_button = '1' or kernel_interrupt <= '0' then
	        	state_next <= INIT;
	    	else
		        ------------------------------------
		        -- State Machine :: IDLE
		        ------------------------------------
		    	if state_current = IDLE then
					isIDLE <= '1';
					isINIT <= '0';
					isCONFIG <= '0';
					isDEVICE <= '0';
					isDONE <= '0';
					I2C_SCK <= 'Z';
					I2C_SDA <= 'Z';
				end if;
		        ------------------------------------
		        -- State Machine :: INIT
		        ------------------------------------
		        if state_current = INIT then
		            if init_timer = smStateDelay then -- delay for the reset to stabilise
		            	state_next <= CONFIG;
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
		        if state_current = CONFIG then
		            if config_timer = smStateDelay then
		            	state_next <= SEND;
		            	----------------------------
		            	-- Body
		            	----------------------------
		            	sck_timer <= "11111001"; -- Reset timer so SCK is invereted @ 1st clock cycle
		            	sda_timer <= "111110011"; -- Reset timer so data is passed @ 1st clock cycle
		            	sda_offset <= "0000000001100011"; -- [100-1] :: SDA Offset
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
		        if state_current = SEND then
		        	if send_timer = smStateDelay then
			        	state_next <= DONE;
					else
		        		if status_timer = "1111111111111111" then -- Length :: 25k clock cycles :: -----===[ RESET ]===----
				        else
------------------------------------------------------
-- PIPE[0] :: Read SCK Status Registers
------------------------------------------------------
			                if status_timer = "0000000111110011" then -- [500-1] :: Address Clock 1
			                	status_sck <= "0001";
			                end if;

			                if status_timer = "0000111110011111" then -- [4000-1] :: RW 1
			                	status_sck <= "0010";
			                end if;

			                if status_timer = "0001000110010011" then -- [4500-1] :: ACK/NAK 1
			                	status_sck <= "0011";
			                end if;

			                if status_timer = "0001001110000111" then -- [5000-1] :: BARIER 1
			                	status_sck <= "0100";
			                end if;

			                if status_timer = "0001101101010111" then -- [7000-1] :: Data Clock 2
			                	status_sck <= "0101";
			                end if;

					        if status_timer = "0010101011110111" then -- [11000-1] :: ACK/NAK 2
			                	status_sck <= "0110";
			                end if;

					        if status_timer = "0010110011101011" then -- [11500-1] :: BARIER 2
			                	status_sck <= "0111";
			                end if;

					        if status_timer = "0011010010111011" then -- [13500-1] :: INIT SDA RETURN
			                	status_sck <= "1000";
			                end if;

					        if status_timer = "0011011010101111" then -- [14000-1] :: Return Clock 3
			                	status_sck <= "1001";
			                end if;

			 				if status_timer = "0101101111001011" then -- [23500-1] :: BARIER 3
			                	status_sck <= "1010";
			                end if;
------------------------------------------------------
-- PIPE[0] :: Read SDA Status Registers
------------------------------------------------------
			                if status_timer = sda_offset then -- [100-1] :: Start bit
			                	status_sda <= "0001";
			                end if;

			                if status_timer = sda_offset + "0000000111110100" then -- [500] :: Address Data 1
			                	status_sda <= "0010";
			                end if;

			                if status_timer = sda_offset + "0000111110100000" then -- [4000] :: RW 1
			                	status_sda <= "0011";
			                end if;

			                if status_timer = sda_offset + "0001000110010100" then -- [4500] :: ACK/NAK 1
			                	status_sda <= "0100";
			                end if;

			                if status_timer = sda_offset + "0001001110001000" then -- [5000] :: BARIER 1
			                	status_sda <= "0101";
			                end if;

			                if status_timer = sda_offset + "0001101101011000" then -- [7000] :: Register Data 2
			                	status_sda <= "0110";
			                end if;

			                if status_timer = sda_offset + "0010101011111000" then -- [11000] :: ACK/NAK 2
			                	status_sda <= "0111";
			                end if;

			                if status_timer = sda_offset + "0010110011101100" then -- [11500] :: BARIER 2
			                	status_sda <= "1000";
			                end if;

					        if status_timer = sda_offset + "0011010110110110" then -- [13750] :: INIT SDA RETURN
			                	status_sda <= "1001";
			                end if;

					        if status_timer = sda_offset + "0011011010110000" then -- [14000] :: Address Data 2
			                	status_sda <= "1010";
			                end if;

					        if status_timer = sda_offset + "0100010001011100" then -- [17500] :: RW 2
			                	status_sda <= "1011";
			                end if;

					        if status_timer = sda_offset + "0100011001010000" then -- [18000] :: ACK/NAK 3
			                	status_sda <= "1100";
			                end if;

					        if status_timer = sda_offset + "0100100001000100" then -- [18500] :: Data From Register
			                	status_sda <= "1101";
			                end if;

					        if status_timer = sda_offset + "0101011111100100" then -- [22500] :: ACK/NAK 4
			                	status_sda <= "1110";
			                end if;

					        if status_timer = sda_offset + "0101100111011000" then -- [23000] :: Stop Bit
			                	status_sda <= "1111";
			                end if;

					        if status_timer = sda_offset + "0101110111000000" then -- [24000] :: BARIER 3
			                	status_sda <= "0000";
			                end if;
------------------------------------------------------
-- PIPE[1] :: Process SCK Status Register
------------------------------------------------------
			                if status_sck = "0001" -- Clock 1
			                or status_sck = "0010" -- RW 1
			                or status_sck = "0011" -- ACK/NAK 1
			                or status_sck = "0101" -- Clock 2
			                or status_sck = "0110" -- ACK/NAK 2
			                or status_sck = "1001" -- Clock 3
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

			                if status_sck = "0100" -- BARIER 1
			                or status_sck = "0111" -- BARIER 2
			                then
			                	I2C_SCK <= '0';
			                end if;

			                if status_sck = "1000" -- INIT SDA
			                or status_sck = "1010" -- BARIER 3
			                then
			                	I2C_SCK <= '1';
			                end if;
------------------------------------------------------
-- PIPE[1] :: Process SDA Status Register
------------------------------------------------------
			                if status_sda = "0001" then -- Start bit
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= '0';
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "0010" then -- Data 
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= address_I2C(index); -- Address Data 1
			                		index <= index + 1;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "0011" then -- RW 1
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= '0';
			                		index <= 0;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "0110" then -- Data 
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= register_I2C(index);
			                		index <= index + 1;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "1001" then --  INIT SDA
			                	I2C_SDA <= '0';
			                end if;

			                if status_sda = "1010" then -- Data
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= address_I2C(index); -- Address Data 2
			                		index <= index + 1;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "1011" then -- RW 2
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		I2C_SDA <= '1';
			                		index <= 0;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "1101" then -- Return Data 
			                	if sda_timer = "111110011" then -- Half bit time
			                		sda_timer <= (others => '0');
			                		return_data(7 - index) <= I2C_SDA; -- Return Data
			                		index <= index + 1;
			                	else
			                		sda_timer <= sda_timer + '1';
			                	end if;
			                end if;

			                if status_sda = "1111" then --  Stop bit 
			                	FPGA_INT <= '1';
			                	I2C_SDA <= '0';
			                end if;

			                if status_sda = "0100" -- ACK/NAK 1
			                or status_sda = "0101" -- BARIER 1
			                or status_sda = "0111" -- ACK/NAK 2
			                or status_sda = "1000" -- BARIER 2
			                or status_sda = "1100" -- ACK/NAK 3
			                or status_sda = "1110" -- ACK/NAK 4
			                then -- BARIER :: 'Z'
			                	I2C_SDA <= 'Z';
			                end if;

			                if status_sda = "0000" -- Stop Bit
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

					isIDLE <= '0';
					isINIT <= '0';
					isCONFIG <= '0';
					isDEVICE <= '1';
					isDONE <= '0';
			    end if;
		        ------------------------------------
		        -- State Machine :: DONE
		        ------------------------------------
		        if state_current = DONE then
		            if done_timer = smStateDelay then
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
			        	state_next <= IDLE;
			        	-- Reset to default
			        	return_data <= "11100111";
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
		        state_current <= state_next;

				------------------------------------
				-- State Machine :: Output
				------------------------------------

				------------------------------------
				-- State Machine :: Status
				------------------------------------
				LED_1 <= isIDLE;
				LED_2 <= isINIT;
				LED_3 <= isCONFIG;
				LED_4 <= isDEVICE;
				LED_5 <= isDONE;
				LED_6 <= '0';
				LED_7 <= status_sda(0) or status_sda(1) or status_sda(2) or status_sda(3);
				LED_8 <= status_sck(0) or status_sck(1) or status_sck(2) or status_sck(3);

	        end if;
    	end if;
    end if;
end process;

-----------------------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------------------
--FPGA_INT <= interrupt_signal;

end rtl;


