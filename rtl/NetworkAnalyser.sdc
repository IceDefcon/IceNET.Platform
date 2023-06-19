# 
#
#
# Clock constraints
set_instance_assignment -name CLOCK_NETWORK "50MHz" -to CLOCK

# Automatically constrain PLL and other generated clocks
derive_pll_clocks -create_base_clocks





