library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity SoftwareDefinedRadioWraper is
generic
(
    CTRL : std_logic_vector(3 downto 0) := "0000"
);
Port
(
    CLOCK_250MHz : in  std_logic;
    RESET : in  std_logic;
    -- IN
    ADC_12_BITS_I : in  std_logic_vector(11 downto 0); -- Real
    ADC_12_BITS_Q : in  std_logic_vector(11 downto 0); -- Imaginary
    -- OUT
    COMPLEMENT_XK_RE : out std_logic_vector(15 downto 0);
    COMPLEMENT_XK_IM : out std_logic_vector(15 downto 0);
    FLOAT_XK_RE : out std_logic_vector(31 downto 0);
    FLOAT_XK_IM : out std_logic_vector(31 downto 0)
);
end entity SoftwareDefinedRadioWraper;

architecture rtl of SoftwareDefinedRadioWraper is

----------------------------------------------------------------------------------------------------------------
-- Input Signals
----------------------------------------------------------------------------------------------------------------

signal fft_input_two_complement_real : std_logic_vector(15 downto 0) := (others => '0');
signal fft_input_two_complement_imag : std_logic_vector(15 downto 0) := (others => '0');
signal fft_start : std_logic := '0';
signal fft_unload : std_logic := '0';
signal fft_write_enable : std_logic := '0';
-- Indicating forward inversion
signal fft_forward_inversion : std_logic := '0';
signal fft_forward_inversion_write_enable : std_logic := '0';
-- NFFT = log₂(1024) = 10 ---> Which is "01010"
constant FFT_NFFT : std_logic_vector(4 downto 0) := "01010";
----------------------------------------------------------------------------------------------------------------
--
-- SCALE_SCH=2×ceil(5)=2×5=10 bits
-- An example scaling schedule for N =1024, Radix-4,
--
-- From Datasheet
-- Burst I/O is [1 0 2 3 2] (ordered from last to first stage)
--
----------------------------------------------------------------------------------------------------------------
constant FFT_SCALING_SCHEDULE : std_logic_vector(9 downto 0) := "0100101110";

signal fft_scaling_schedule_write_enable : std_logic := '0';
----------------------------------------------------------------------------------------------------------------
--
-- 1024 64  1/16    Low-delay channel, common
-- 1024 128 1/8     Typical in WiFi, LTE
-- 1024 256 1/4     Long multipath channel
-- 1024 0           No prefix (not recommended for OFDM)
--
-- We are not using FFT for communication ---> So (others => '0')
--
----------------------------------------------------------------------------------------------------------------
signal fft_cyclic_prefix_length : std_logic_vector(9 downto 0) := (others => '0');
signal fft_cyclic_prefix_write_enable : std_logic := '0';
-- Clock and Reet
signal fft_master_synchronous_reset : std_logic := '0';
signal fft_clock_enable : std_logic := '0';
signal fft_clock : std_logic := '0';

type SDR_SM is
(
    SDR_IDLE,
    SDR_INIT,
    SDR_CONFIG,
    SDR_PROCESS,
    SDR_DONE
);
signal sdr_state: SDR_SM := SDR_IDLE;

----------------------------------------------------------------------------------------------------------------
-- COMPONENTS DECLARATION
----------------------------------------------------------------------------------------------------------------

component TwoComplementConverter
Port
(
    CLOCK_250MHz : in  std_logic;
    RESET        : in  std_logic;
    -- IN
    I_12_BITS    : in  std_logic_vector(11 downto 0);
    -- OUT
    O_16_BITS    : out std_logic_vector(15 downto 0)
);
end component;

component SinglePrecisionFloatConverter
Port
(
    CLOCK_250MHz : in  std_logic;
    RESET        : in  std_logic;
    -- IN
    FIXED_IN  : in  std_logic_vector(15 downto 0);  -- 16-bit 2's complement
    -- OUT
    FLOAT_OUT : out std_logic_vector(31 downto 0)   -- 32-bit IEEE 754 float
);
end component;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------

begin

----------------------------------------------------------------------------------------------------------------
-- INSTANTIAIONS
----------------------------------------------------------------------------------------------------------------

TwoComplementConverter_module: TwoComplementConverter
port map
(
    CLOCK_250MHz => CLOCK_250MHz,
    RESET => RESET,
    -- IN
    I_12_BITS => (others => '0'),
    -- OUT
    O_16_BITS => open
);

SinglePrecisionFloatConverter_module: SinglePrecisionFloatConverter
port map
(
    CLOCK_250MHz => CLOCK_250MHz,
    RESET => RESET,
    -- IN
    FIXED_IN => (others => '0'),
    -- OUT
    FLOAT_OUT => open
);

----------------------------------------------------------------------------------------------------------------
-- PROCESSES
----------------------------------------------------------------------------------------------------------------

state_machine_process:
process(CLOCK_250MHz, RESET)
begin
    if RESET = '1' then
        fft_input_two_complement_real <= (others => '0');
        fft_input_two_complement_imag <= (others => '0');
        fft_start <= '0';
        fft_unload <= '0';
        fft_write_enable <= '0';
        fft_forward_inversion <= '0';
        fft_forward_inversion_write_enable <= '0';
        fft_scaling_schedule_write_enable <= '0';
        fft_cyclic_prefix_length <= (others => '0');
        fft_cyclic_prefix_write_enable <= '0';
        fft_master_synchronous_reset <= '0';
        fft_clock_enable <= '0';
        fft_clock <= '0';
        sdr_state <= SDR_IDLE;
    elsif rising_edge(CLOCK_250MHz) then

        case sdr_state is
            when SDR_IDLE =>

            when SDR_INIT =>

            when SDR_CONFIG =>

            when SDR_PROCESS =>

            when SDR_DONE =>

            when others =>
                sdr_state <= SDR_IDLE;
        end case;
    end if;
end process;

end architecture rtl;
