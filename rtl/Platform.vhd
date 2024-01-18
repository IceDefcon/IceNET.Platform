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
    CLOCK_50MHz : in std_logic; -- PIN_T2

    -- For debuging
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

    I2C_IN_SDA : in std_logic; -- PIN_A9   :: BBB P9_20 :: CPU.BLUE <> FPGA.BLUE <> GYRO.WHITE
    I2C_IN_SCK : in std_logic; -- PIN_A10  :: BBB P9_19 :: CPU.ORANGE <> FPGA.GREEN <> GYRO.PURPLE
	 
    I2C_OUT_SDA : out std_logic; -- PIN_B9   :: GYRO.RED
    I2C_OUT_SCK : out std_logic; -- PIN_B10  :: GYRO.ORANGE

    FPGA_INT : out std_logic;  -- PIN_A3   :: BBB P9_12 :: BLACK
    KERNEL_INT : in std_logic; -- PIN_A4   :: BBB P9_14 :: WHITE

    BUTTON_1 : in std_logic; -- PIN_H20  :: Reset
    BUTTON_2 : in std_logic; -- PIN_K19  :: Doesnt Work :: Broken Button or Incorrect Schematic
    BUTTON_3 : in std_logic; -- PIN_J18
    BUTTON_4 : in std_logic  -- PIN_K18
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
constant data_SPI : std_logic_vector(7 downto 0) := "10001000"; -- 0x88
-- I2C
constant address_I2C : std_logic_vector(9 downto 0) := "0010011110"; -- 0x69
signal sda_index : integer range 0 to 15 := 0;

-- SPI Synchronise
signal synced_sclk : std_logic := '0';
signal synced_cs : std_logic := '0';
signal synced_mosi : std_logic := '0';
signal synced_miso : std_logic := '0';

-- Reset Button
signal reset_button : std_logic := '0';

-- i2c clocks
signal i2c_clock : std_logic := '0';
signal i2c_clock_next : std_logic := '0';
signal i2c_clock_next01 : std_logic := '0';
signal i2c_clock_next02 : std_logic := '0';
signal i2c_clock_next03 : std_logic := '0';
signal i2c_clock_next04 : std_logic := '0';
signal i2c_clock_next05 : std_logic := '0';
signal i2c_clock_next06 : std_logic := '0';
signal i2c_clock_next07 : std_logic := '0';
signal i2c_clock_next08 : std_logic := '0';
signal i2c_clock_next09 : std_logic := '0';
signal i2c_clock_next10 : std_logic := '0';
signal i2c_clock_next11 : std_logic := '0';
signal i2c_clock_next12 : std_logic := '0';

signal i2c_clock_align : std_logic := '0';
signal i2c_clock_first : std_logic := '0';
signal i2c_clock_last : std_logic := '0';

-- i2c initialise time
signal i2c_clock_timer : std_logic_vector(7 downto 0) := (others => '0');
signal init_timer : std_logic_vector(26 downto 0) := (others => '0');
signal config_timer : std_logic_vector(26 downto 0) := (others => '0');
signal send_timer : std_logic_vector(26 downto 0) := (others => '0');
signal done_timer : std_logic_vector(26 downto 0) := (others => '0');
signal sck_timer : std_logic_vector(8 downto 0) := (others => '0');
signal sda_timer : std_logic_vector(8 downto 0) := (others => '0');
signal sm_timer : std_logic_vector(26 downto 0) := (others => '0');
signal prcess_sda_timer : std_logic_vector(8 downto 0) := "111110011"; -- Initial for tbe start bit

--i2c state machine
type TX is (IDLE, INIT, CONFIG, SEND, DONE, RECEIVE);
signal tx_current_state, tx_next_state: TX := IDLE;

-- i2c write clock
signal write_sda : std_logic := '0';
signal write_sck : std_logic := '0';
signal write_sck_enable : std_logic := '0';
signal write_sck_timer : std_logic_vector(12 downto 0) := (others => '0');
signal write_sda_timer : std_logic_vector(12 downto 0) := (others => '0');

-- i2c debug
signal sda : std_logic := '0';
signal sck : std_logic := '0';

-- Debug Interrupt
signal diode_check : std_logic := '0';
signal diode_done : std_logic := '0';

-- State Machine Init
signal sm_run : std_logic := '0';

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
signal isDONE : std_logic := '0';


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
	DATA 			=> data_SPI,
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
        if i2c_clock_timer = "11111001" then
            i2c_clock <= not i2c_clock;
            i2c_clock_timer <= (others => '0'); -- Reset count
        else
            i2c_clock_timer <= i2c_clock_timer + '1';
        end if;

        -- Assign delayed clocks unconditionally on every clock edge
        i2c_clock_next <= i2c_clock;
        i2c_clock_next01 <= i2c_clock_next;
        i2c_clock_next02 <= i2c_clock_next01;
        i2c_clock_next03 <= i2c_clock_next02;
        i2c_clock_next04 <= i2c_clock_next03;
        i2c_clock_next05 <= i2c_clock_next04;
        i2c_clock_next06 <= i2c_clock_next05;
        i2c_clock_next07 <= i2c_clock_next06;
        i2c_clock_next08 <= i2c_clock_next07;
        i2c_clock_next09 <= i2c_clock_next08;
        i2c_clock_next10 <= i2c_clock_next09;
        i2c_clock_next11 <= i2c_clock_next10;
        i2c_clock_next12 <= i2c_clock_next11;

    end if;
end process;

state_machine_process:
process(CLOCK_50MHz, reset_button, kernel_interrupt, sm_run, 
	sm_timer, init_timer, config_timer, send_timer, 
	isIDLE, isINIT, isCONFIG, isDEVICE, isDONE, 
	tx_current_state, tx_next_state,
	write_sda, write_sck_enable, write_sck)
begin
    if rising_edge(CLOCK_50MHz) then

    	kernel_interrupt <= KERNEL_INT;

        ----------------------------------------
        -- Reset State Machine
        ----------------------------------------
		if sm_run = '0' then
		    if sm_timer = "101111101011110000011111111" then -- 2s delay
		        sm_timer <= (others => '0');
				write_sda <= '1';
				write_sck <= '1';
		        sm_run <= '1';
		    else
		        isIDLE <= '1';
		        isINIT <= '0';
		        isCONFIG <= '0';
		        isDEVICE <= '0';
		        isDONE <= '0';
		        sm_timer <= sm_timer + '1';
		    end if;

    	elsif sm_run = '1' then

	        ----------------------------------------
	        -- State Machine :: INIT Process
	        ----------------------------------------
	        if tx_current_state = IDLE then
				isIDLE <= '1';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '0';
				isDONE <= '0';
		    end if;

    		----------------------------------------
    		-- State Machine :: HW & SW Reset
    		----------------------------------------
	        if reset_button = '1' or kernel_interrupt <= '0' then
        		tx_next_state <= INIT;
	        end if;

	        ----------------------------------------
	        -- State Machine :: INIT Process
	        ----------------------------------------
	        if tx_current_state = INIT then
	            if init_timer = "010111110101111000001111111" then
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

	        ----------------------------------------
	        -- State Machine :: CONFIG Process
	        ----------------------------------------
	        if tx_current_state = CONFIG then
	            if config_timer = "010111110101111000001111111" then
	            	tx_next_state <= SEND;
	            	-----------------------------------
	            	-- Body
	            	-----------------------------------
	            else
	                config_timer <= config_timer + '1';
	            end if;

				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '1';
				isDEVICE <= '0';
				isDONE <= '0';
		    end if;

	        ----------------------------------------
	        -- State Machine :: SEND Process
	        ----------------------------------------
	        if tx_current_state = SEND then
	        	if send_timer = "010111110101111000001111111" then

	            	---> SDA :: Start bit
	            	if sda_timer = "111110100" then
	            		-- Toogle
	            		write_sda <= '1';
	            	else
	            		sda_timer <= sda_timer + '1';
	            		write_sda <= '0';
	            	end if;

	            	---> SCK :: Enable
	            	if sda_timer = "000110001" then
	            		write_sck_enable <= '1';
	            	end if;

	            	---> SCK
	            	if write_sck_enable = '1' then
		            	if sck_timer = "111110100" then
	            			-- Toogle
		            		write_sck <= '1';
		            		-- Change State
		        			tx_next_state <= DONE;
		            	else
		            		sck_timer <= sck_timer + '1';
		            		write_sck <= '0';
		            	end if;
		            end if;

				else
					send_timer <= send_timer + '1';
				end if;

				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '1';
				isDONE <= '0';
		    end if;

	        ----------------------------------------
	        -- State Machine :: DONE Process
	        ----------------------------------------
	        if tx_current_state = DONE then
	            if done_timer = "010111110101111000001111111" then
	            	-- Reset Timers
	            	sda_timer <= (others => '0');
	            	sck_timer <= (others => '0');
	                init_timer <= (others => '0');
	                config_timer <= (others => '0');
	            	send_timer <= (others => '0');
	            	done_timer <= (others => '0');
	            	-- Change State
	        		tx_next_state <= IDLE;
	            	-- Reset Others
	            	write_sck_enable <= '0';
	            else
	                done_timer <= done_timer + '1';
	            end if;
	            
				isIDLE <= '0';
				isINIT <= '0';
				isCONFIG <= '0';
				isDEVICE <= '0';
				isDONE <= '1';
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
        	LED_5 <= isDONE;
        	LED_6 <= '0';
        	LED_7 <= '0';
        	LED_8 <= write_sck_enable;

	        ----------------------------------------
	        -- SM :: Output
	        ----------------------------------------
			I2C_OUT_SCK <= write_sck;
			I2C_OUT_SDA <= write_sda;

	        ----------------------------------------
	        -- State Machine :: SEND Process
	        ----------------------------------------
	        --if tx_current_state = SEND then

	        --	if send_timer = "010111110101111000001111111" then

			--	    if i2c_clock = '1' then 	-- Align @ 1 to achieve cycle from '0'
			--	        i2c_clock_align <= '1'; -- Stay HI until reset
			--	    end if;

			--	    -- Clock is aligned
			--	    if i2c_clock_align = '1' then
			--			if write_go = '0' and i2c_clock = '0' then
			--				write_go <= '1';
			--			end if;

			--			if write_go = '1' then
			--				-------------------------------------
			--				-- SCK
			--				-------------------------------------
			--				if sck_timer = "111111111" then
			--					if write_sck_timer = "1000110010100" then
			--						-------------------------------------
			--						-- Body
			--						-------------------------------------
			--						write_sck <= '0'; -- Kill the clock :: For now '0'
			--					else
			--						write_sck <= i2c_clock_next12; -----===[ OUT ]===-----
			--						write_sck_timer <= write_sck_timer + '1';
			--					end if;
			--				else
			--					sck_timer <= sck_timer + '1';
			--				end if;
			--				-------------------------------------
			--				-- SDA
			--				-------------------------------------
			--				if sda_timer = "001000010" then
			--					if write_sda_timer = "1010101111100" then
			--						-------------------------------------
			--						-- Body
			--						-------------------------------------
			--						write_sda <= '1'; -- Take '1' from 'Z'
			--		            	tx_next_state <= DONE; -- Must change state here :: SDA is longer
			--					else
			--				        if prcess_sda_timer = "111110011" then
			--				        	if sda_index < 9 then
			--				            	write_sda <= address_I2C(sda_index); -----===[ OUT ]===-----
			--				            	sda_index <= sda_index + 1;
			--				            else 
			--				            	ice_debug <= '1';
			--				            	write_sda <= 'Z';
			--				            end if;

			--				            prcess_sda_timer <= (others => '0'); -- Reset count
			--				        else
			--				            prcess_sda_timer <= prcess_sda_timer + '1';
			--				        end if;

			--						write_sda_timer <= write_sda_timer + '1';
			--					end if;
			--				else
			--					sda_timer <= sda_timer + '1';
			--				end if;
			--			end if;
			--	    end if;
			--	else
			--		send_timer <= send_timer + '1';
			--	end if;

			--	isIDLE <= '0';
			--	isINIT <= '0';
			--	isCONFIG <= '0';
			--	isDEVICE <= '1';
			--	isDONE <= '0';
		    --end if;

	        ----------------------------------------
	        -- State Machine :: DONE Process
	        ----------------------------------------
	        --if tx_current_state = DONE then
	        --    if done_timer = "010111110101111000001111111" then
	        --    	sck_timer <= (others => '0');
			--		sda_timer <= (others => '0');
	        --        done_timer <= (others => '0');
	        --        write_sck_timer <= (others => '0');
	        --        write_sda_timer <= (others => '0');
	        --        send_timer <= (others => '0');

	        --        sda_index <= 0;
	        --        write_go <= '0';
	        --    	tx_next_state <= IDLE; -- Back to idle wait for another interrupt
	        --        i2c_clock_align <= '0'; -- Reset Alignment
	        --        --sm_run <= '0'; -- Hold State Machine :: Debug
	        --    else
	        --        done_timer <= done_timer + '1';
	        --    end if;
	            
			--	isIDLE <= '0';
			--	isINIT <= '0';
			--	isCONFIG <= '0';
			--	isDEVICE <= '0';
			--	isDONE <= '1';
		    --end if;

	        ----------------------------------------
	        -- SM :: State update
	        ----------------------------------------
	        --tx_current_state <= tx_next_state;

	        ----------------------------------------
	        -- SM :: Current LED State
	        ----------------------------------------
	        --LED_1 <= isIDLE;
        	--LED_2 <= isINIT;
        	--LED_3 <= isCONFIG;
        	--LED_4 <= isDEVICE;
        	--LED_5 <= isDONE;
        	--LED_6 <= '0';
        	--LED_7 <= '0';
        	--LED_8 <= '0';
		end if;
    end if;
end process;

i2c_process:
process(CLOCK_50MHz, write_sck, write_sda, sck, sda)
begin
	if rising_edge(CLOCK_50MHz) then


		sck <= I2C_IN_SCK;
		sda <= I2C_IN_SDA;
	end if;
end process;

-- --------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------
FPGA_INT <= interrupt_signal;

-----------------------------------------------
--
-- Ice Debug
--
-----------------------------------------------

end rtl;


