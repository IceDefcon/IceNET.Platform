library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

------------------------------
-- Author: Ice.Marek 		--
-- IceNET Technology 2024 	--
-- 							--
-- FPGA Chip 				--
-- Cyclone IV 				--
-- EP4CE15F23C8 			--
------------------------------
entity Platform is
port
(
	CLOCK_50MHz 	: in std_logic; 	-- PIN_T2

	-- For debuging
	LED_1 			: out std_logic; 	-- PIN_U7
	LED_2 			: out std_logic; 	-- PIN_U8
	LED_3 			: out std_logic;	-- PIN_R7
	LED_4 			: out std_logic; 	-- PIN_T8
	LED_5 			: out std_logic; 	-- PIN_R8
	LED_6 			: out std_logic;	-- PIN_P8
	LED_7 			: out std_logic; 	-- PIN_M8
	LED_8 			: out std_logic; 	-- PIN_N8

	KERNEL_CS 		: in  std_logic; 	-- PIN_A5 	:: BBB P9_17 :: PULPLE 	:: SPI0_CS0
	KERNEL_MOSI 	: in  std_logic; 	-- PIN_A7 	:: BBB P9_18 :: BLUE 		:: SPI0_D1
	KERNEL_MISO 	: out std_logic; 	-- PIN_A6 	:: BBB P9_21 :: BROWN 	:: SPI0_D0
	KERNEL_SCLK 	: in  std_logic; 	-- PIN_A8 	:: BBB P9_22 :: BLACK 	:: SPI0_SCLK

	I2C_IN_SDA 		: in 	std_logic; 	-- PIN_A9 	:: BBB P9_20 :: BLUE
	I2C_IN_SCK 		: in 	std_logic; 	-- PIN_A10 	:: BBB P9_19 :: GREEN

	I2C_OUT_SDA 	: out 	std_logic; 	-- PIN_B9 	:: BBB P9_20 :: RED
	I2C_OUT_SCK 	: out 	std_logic; 	-- PIN_B10 	:: BBB P9_19 :: ORANGE

	INT_CPU 			: out std_logic; 	-- PIN_A3 	:: BBB P9_12 :: BLACK
	INT_FPGA 		: in 	std_logic; 	-- PIN_A4 	:: BBB P9_14 :: WHITE

	BUTTON_1 		: in  std_logic; 	-- PIN_H20 	:: Reset
	BUTTON_2 		: in  std_logic; 	-- PIN_K19 	:: Doesnt Work :: Broken Button or Incorrect Schematic
	BUTTON_3 		: in  std_logic; 	-- PIN_J18
	BUTTON_4 		: in  std_logic 	-- PIN_K18
);
end Platform;

architecture rtl of Platform is

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
signal idle_button			: std_logic := '0';

-- i2c clocks
signal i2c_half_wave : std_logic_vector(7 downto 0) := (others => '0');
signal i2c_clock_d0 : std_logic := '0';
signal i2c_clock_d1 : std_logic := '0';
signal i2c_clock_d2 : std_logic := '0';
signal i2c_clock_d3 : std_logic := '0';
signal clock_aligned : std_logic := '0';

-- i2c initialise time
signal init_time : std_logic_vector(26 downto 0) := (others => '0');
signal device_time : std_logic_vector(26 downto 0) := (others => '0');

--i2c state machine
type TX is (IDLE, INIT, DEVICE);
signal tx_current_state, tx_next_state: TX := IDLE;

-- i2c state machine flags
signal isIDLEdone : std_logic := '0';
signal isINITdone : std_logic := '0';
signal isDEVICEdone : std_logic := '0';

-- i2c write clock
signal write_clock : std_logic := '0';
signal write_clock_detected : std_logic := '0';
signal write_clock_count : std_logic_vector(3 downto 0) := "0000";

-- i2c debug
signal sda : std_logic := '0';
signal sck : std_logic := '0';

-- Debug Interrupt
signal diode_check : std_logic := '0';
signal diode_done : std_logic := '0';
signal ice_time : std_logic_vector(26 downto 0) := (others => '0');
signal ice_start : std_logic := '0';
signal ice_go : std_logic := '0';
----------------------------
-- COMPONENTS DECLARATION --
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
-- MAIN ROUTINE --
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
	clock => CLOCK_50MHz,
	button_in_1 => BUTTON_1,
	button_in_2 => BUTTON_2,
	button_in_3 => BUTTON_3,
	button_in_4 => BUTTON_4,
	button_out_1 => reset_button,
	button_out_2 => open,
	button_out_3 => idle_button,
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
        if i2c_half_wave = "11111001" then
            i2c_clock_d0 <= not i2c_clock_d0;
            i2c_half_wave <= (others => '0'); -- Reset count
        else
            i2c_half_wave <= i2c_half_wave + '1';
        end if;

        -- Assign delayed clocks unconditionally on every clock edge
        i2c_clock_d1 <= i2c_clock_d0;
        i2c_clock_d2 <= i2c_clock_d1;
        i2c_clock_d3 <= i2c_clock_d2;
    end if;
end process;

ice_process:
process(CLOCK_50MHz)
begin
    if rising_edge(CLOCK_50MHz) then
	    if ice_time = "101111101011110000011111111" then -- Transition to DEVIE @ 2s Time
	        ice_time <= (others => '0');
	        ice_go <= '1';
	    else
	        ice_time <= ice_time + '1';
	    end if;
	end if;
end process;

state_machine_process:
process(CLOCK_50MHz, i2c_clock_d0, i2c_clock_d1, i2c_clock_d2, i2c_clock_d3, isIDLEdone, isINITdone, isDEVICEdone, tx_current_state, tx_next_state,
	reset_button, init_time, device_time, clock_aligned, write_clock, write_clock_count, write_clock_detected,
	ice_go, ice_start)
begin
    if rising_edge(CLOCK_50MHz) then

    	if ice_start = '0' then
			LED_1 <= '1';
			LED_2 <= '1';
			LED_3 <= '1';
			LED_4 <= '1';
			LED_5 <= '1';
			LED_6 <= '1';
			LED_7 <= '1';
			LED_8 <= '1';
			ice_start <= '1';
		elsif idle_button <= '1' then
			tx_next_state <= IDLE;
		end if;
		
    	if ice_go = '1' then

    		LED_1 	<= '0';
			tx_current_state <= tx_next_state;  -- Update current state

			case tx_current_state is

			    when IDLE =>
			        if reset_button = '1' then
			            tx_next_state <= INIT;
						LED_5 <= '0';
						LED_6 <= '0';
			        else
			            tx_next_state <= IDLE;
			        end if;

			    when INIT =>
			        if init_time = "101111101011110000011111111" then
			            init_time <= (others => '0');
			            tx_next_state <= DEVICE;
						LED_5 <= '1';
						LED_6 <= '0';
			        else
			            tx_next_state <= INIT;
						LED_5 <= '0';
						LED_6 <= '1';
			            init_time <= init_time + '1';
			        end if;

			    when DEVICE =>
			        if device_time = "101111101011110000011111111" then
			            device_time <= (others => '0');
			            tx_next_state <= IDLE;
			        else
			            tx_next_state <= DEVICE;
						LED_5 <= '1';
						LED_6 <= '1';
			            device_time <= device_time + '1';
			        end if;

			    when others =>
			        tx_next_state <= IDLE;

			end case;

	        ------------------------------------------
	        ---- SM :: DEVICE Process
	        ------------------------------------------
	        --if tx_current_state = DEVICE then
		    --    if i2c_clock_d0 = '0' then
		    --        clock_aligned <= '1';
		    --        LED_8 <= clock_aligned;
		    --    end if;

		    --    -- Clock is aligned
		    --    if clock_aligned = '1' then

		    --    	write_clock <= i2c_clock_d0;

	        --        if i2c_clock_d0 = '0' then  -- First '0' after aligned '1'

	        --            -- End of DEVICE address transmission
	        --            if write_clock_count = "1001" then
	        --                write_clock_count  	<= "0000";
	        --                isDEVICEdone 	<= '1';
	        --            end if;

	        --            -- Write clock cycle is detected
			--			if write_clock_detected = '0' then
		    --                write_clock_count <= write_clock_count + '1';
			--				write_clock_detected <= '1';
			--			end if;
			--		else 
			--			write_clock	<= i2c_clock_d0;
			--			write_clock_detected <= '0';
	        --        end if;
		    --    end if;
		    --end if;
		end if;
    end if;
end process;


i2c_process:
process(CLOCK_50MHz, reset_button, write_clock)
begin
	if rising_edge(CLOCK_50MHz) then
		I2C_OUT_SDA <= reset_button;
		I2C_OUT_SCK <= write_clock;
	end if;
end process;

-- --------------------------------
-- Interrupt is pulled down
-- In order to adjust PID
-- Controler for the gyroscope
-----------------------------------
INT_CPU <= '0'; -- interrupt_signal;

end rtl;