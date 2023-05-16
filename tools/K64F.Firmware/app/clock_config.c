

#include "fsl_smc.h"
#include "clock_config.h"

//
// Definitions
//
#define MCG_PLL_DISABLE 0U             /*!< MCGPLLCLK disabled */
#define OSC_CAP0P 0U                   /*!< Oscillator 0pF capacitor load */
#define OSC_ER_CLK_DISABLE 0U          /*!< Disable external reference clock */
#define SIM_OSC32KSEL_RTC32KCLK_CLK 2U /*!< OSC32KSEL select: RTC32KCLK clock (32.768kHz) */
#define SIM_PLLFLLSEL_IRC48MCLK_CLK 3U /*!< PLLFLL select: IRC48MCLK clock */
#define SIM_PLLFLLSEL_MCGPLLCLK_CLK 1U /*!< PLLFLL select: MCGPLLCLK clock */

//
// Variables
//
// System clock frequency
//
extern uint32_t SystemCoreClock;


//
// FUNCTION
//
static void CLOCK_CONFIG_SetFllExtRefDiv(uint8_t frdiv)
{
    MCG->C1 = ((MCG->C1 & ~MCG_C1_FRDIV_MASK) | MCG_C1_FRDIV(frdiv));
}

//
// Variables for BOARD_BootClockRUN configuration
//
const mcg_config_t mcgConfig_BOARD_BootClockRUN = {
    .mcgMode         = kMCG_ModePEE,      /* PEE - PLL Engaged External */
    .irclkEnableMode = kMCG_IrclkEnable,  /* MCGIRCLK enabled, MCGIRCLK disabled in STOP mode */
    .ircs            = kMCG_IrcSlow,      /* Slow internal reference clock selected */
    .fcrdiv          = 0x0U,              /* Fast IRC divider: divided by 1 */
    .frdiv           = 0x0U,              /* FLL reference clock divider: divided by 32 */
    .drs             = kMCG_DrsLow,       /* Low frequency range */
    .dmx32           = kMCG_Dmx32Default, /* DCO has a default range of 25% */
    .oscsel          = kMCG_OscselOsc,    /* Selects System Oscillator (OSCCLK) */
    .pll0Config =
        {
            .enableMode = MCG_PLL_DISABLE, /* MCGPLLCLK disabled */
            .prdiv      = 0x13U,           /* PLL Reference divider: divided by 20 */
            .vdiv       = 0x18U,           /* VCO divider: multiplied by 48 */
        },
};
const sim_clock_config_t simConfig_BOARD_BootClockRUN = {
    .pllFllSel = SIM_PLLFLLSEL_MCGPLLCLK_CLK, /* PLLFLL select: MCGPLLCLK clock */
    .er32kSrc  = SIM_OSC32KSEL_RTC32KCLK_CLK, /* OSC32KSEL select: RTC32KCLK clock (32.768kHz) */
    .clkdiv1   = 0x1240000U,                  /* SIM_CLKDIV1 - OUTDIV1: /1, OUTDIV2: /2, OUTDIV3: /3, OUTDIV4: /5 */
};
const osc_config_t oscConfig_BOARD_BootClockRUN = {
    .freq        = 50000000U,    /* Oscillator frequency: 50000000Hz */
    .capLoad     = (OSC_CAP0P),  /* Oscillator capacity load: 0pF */
    .workMode    = kOSC_ModeExt, /* Use external clock */
    .oscerConfig = {
        .enableMode =
            kOSC_ErClkEnable, /* Enable external reference clock, disable external reference clock in STOP mode */
    }};

//
// Code for BOARD_BootClockRUN configuration
//
void BOARD_BootClockRUN(void)
{
    /* Set the system clock dividers in SIM to safe value. */
    CLOCK_SetSimSafeDivs();
    /* Initializes OSC0 according to board configuration. */
    CLOCK_InitOsc0(&oscConfig_BOARD_BootClockRUN);
    CLOCK_SetXtal0Freq(oscConfig_BOARD_BootClockRUN.freq);
    /* Configure the Internal Reference clock (MCGIRCLK). */
    CLOCK_SetInternalRefClkConfig(mcgConfig_BOARD_BootClockRUN.irclkEnableMode, 
                                  mcgConfig_BOARD_BootClockRUN.ircs,
                                  mcgConfig_BOARD_BootClockRUN.fcrdiv);
    /* Configure FLL external reference divider (FRDIV). */
    CLOCK_CONFIG_SetFllExtRefDiv(mcgConfig_BOARD_BootClockRUN.frdiv);
    /* Set MCG to PEE mode. */
    CLOCK_BootToPeeMode(mcgConfig_BOARD_BootClockRUN.oscsel, kMCG_PllClkSelPll0,
                        &mcgConfig_BOARD_BootClockRUN.pll0Config);
    /* Set the clock configuration in SIM module. */
    CLOCK_SetSimConfig(&simConfig_BOARD_BootClockRUN);
    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}


//
// Variables for BOARD_BootClockVLPR configuration
//
const mcg_config_t mcgConfig_BOARD_BootClockVLPR = {
    .mcgMode         = kMCG_ModeBLPI,     /* BLPI - Bypassed Low Power Internal */
    .irclkEnableMode = kMCG_IrclkEnable,  /* MCGIRCLK enabled, MCGIRCLK disabled in STOP mode */
    .ircs            = kMCG_IrcFast,      /* Fast internal reference clock selected */
    .fcrdiv          = 0x0U,              /* Fast IRC divider: divided by 1 */
    .frdiv           = 0x0U,              /* FLL reference clock divider: divided by 32 */
    .drs             = kMCG_DrsLow,       /* Low frequency range */
    .dmx32           = kMCG_Dmx32Default, /* DCO has a default range of 25% */
    .oscsel          = kMCG_OscselOsc,    /* Selects System Oscillator (OSCCLK) */
    .pll0Config =
        {
            .enableMode = MCG_PLL_DISABLE, /* MCGPLLCLK disabled */
            .prdiv      = 0x0U,            /* PLL Reference divider: divided by 1 */
            .vdiv       = 0x0U,            /* VCO divider: multiplied by 24 */
        },
};
const sim_clock_config_t simConfig_BOARD_BootClockVLPR = {
    .pllFllSel = SIM_PLLFLLSEL_IRC48MCLK_CLK, /* PLLFLL select: IRC48MCLK clock */
    .er32kSrc  = SIM_OSC32KSEL_RTC32KCLK_CLK, /* OSC32KSEL select: RTC32KCLK clock (32.768kHz) */
    .clkdiv1   = 0x40000U,                    /* SIM_CLKDIV1 - OUTDIV1: /1, OUTDIV2: /1, OUTDIV3: /1, OUTDIV4: /5 */
};
const osc_config_t oscConfig_BOARD_BootClockVLPR = {
    .freq        = 0U,           /* Oscillator frequency: 0Hz */
    .capLoad     = (OSC_CAP0P),  /* Oscillator capacity load: 0pF */
    .workMode    = kOSC_ModeExt, /* Use external clock */
    .oscerConfig = {
        .enableMode = OSC_ER_CLK_DISABLE, /* Disable external reference clock */
    }};

//
// Code for BOARD_BootClockVLPR configuration
//
void BOARD_BootClockVLPR(void)
{
    /* Set the system clock dividers in SIM to safe value. */
    CLOCK_SetSimSafeDivs();
    /* Set MCG to BLPI mode. */
    CLOCK_BootToBlpiMode(mcgConfig_BOARD_BootClockVLPR.fcrdiv, mcgConfig_BOARD_BootClockVLPR.ircs,
                         mcgConfig_BOARD_BootClockVLPR.irclkEnableMode);
    /* Set the clock configuration in SIM module. */
    CLOCK_SetSimConfig(&simConfig_BOARD_BootClockVLPR);
    /* Set VLPR power mode. */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);
#if (defined(FSL_FEATURE_SMC_HAS_LPWUI) && FSL_FEATURE_SMC_HAS_LPWUI)
    SMC_SetPowerModeVlpr(SMC, false);
#else
    SMC_SetPowerModeVlpr(SMC);
#endif
    while (SMC_GetPowerModeState(SMC) != kSMC_PowerStateVlpr)
    {
    }
    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKVLPR_CORE_CLOCK;
}