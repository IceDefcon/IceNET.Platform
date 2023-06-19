


# Clock constraints

# Automatically constrain PLL and other generated clocks
derive_pll_clocks -create_base_clocks

set_instance_assignment -name CLOCK_NETWORK "50MHz" -to CLOCK

# Automatically calculate clock uncertainty to jitter and other effects.
#derive_clock_uncertainty
# Not supported for family Cyclone II

# tsu/th constraints

# tco constraints

# tpd constraints

