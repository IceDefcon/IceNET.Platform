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
    COMPLEMENT_XK_RE out  std_logic_vector(15 downto 0);
    COMPLEMENT_XK_IM out  std_logic_vector(15 downto 0);
    FLOAT_XK_RE out  std_logic_vector(31 downto 0);
    FLOAT_XK_IM out  std_logic_vector(31 downto 0)
);
end entity SoftwareDefinedRadioWraper;

architecture rtl of SoftwareDefinedRadioWraper is

----------------------------------------------------------------------------------------------------------------
-- Synced :: Asynchronic Input Signals
----------------------------------------------------------------------------------------------------------------

signal complement_real : std_logic_vector(15 downto 0) := (others => '0');
signal complement_imag : std_logic_vector(15 downto 0) := (others => '0');

signal fft_start : std_logic := '0';
signal fft_unload : std_logic := '0';
signal fft_size : std_logic_vector(4 downto 0) := "00000";
signal fft_write_enable : std_logic := '0';
signal fft_forward_inversion : std_logic := '0'; -- Indicating forward inversion
signal fft_forward_inversion_write_enable : std_logic := '0';

----------------------------------------------------------------------------------------------------------------
--
-- Required only for the fixed values (two's complement)
--
--
--
----------------------------------------------------------------------------------------------------------------
signal fft_scaling_schedule : std_logic_vector(1 downto 0) := "00";


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

    I_12_BITS    : in  std_logic_vector(11 downto 0);
    O_16_BITS    : out std_logic_vector(15 downto 0)
);
end component;

component SinglePrecisionFloatConverter
Port
(
    CLOCK_250MHz : in  std_logic;
    RESET        : in  std_logic;

    FIXED_IN  : in  std_logic_vector(15 downto 0);  -- 16-bit 2's complement
    FLOAT_OUT : out std_logic_vector(31 downto 0)   -- 32-bit IEEE 754 float
);
end component;

----------------------------------------------------------------------------------------------------------------
-- MAIN ROUTINE
----------------------------------------------------------------------------------------------------------------

begin

    state_machine_process:
    process(CLOCK_50MHz, RESET)
    begin
        if RESET = '1' then
            sdr_state <= SDR_IDLE
        elsif rising_edge(CLOCK_50MHz) then

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
    end process interrupt_process;


end architecture rtl;
