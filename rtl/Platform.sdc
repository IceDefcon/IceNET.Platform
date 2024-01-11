##################################################
# Clock constraints
##################################################

# Satisfy constraint of the Quartus clock
create_clock -name {CLOCK_50MHz} -period 20 CLOCK_50MHz

# Set setup time :: Data must be stable before the arrival of the active clock edge
set_input_delay -clock [get_clocks CLOCK_50MHz] -max 5 -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -max 5 -fall [get_nets CLOCK_50MHz]

# Set hold time :: Data must be stable before the arrival of the active clock edge
set_input_delay -clock [get_clocks CLOCK_50MHz] -min 5 -rise [get_nets CLOCK_50MHz]
set_input_delay -clock [get_clocks CLOCK_50MHz] -min 5 -fall [get_nets CLOCK_50MHz]

# Set clock uncertainty constraints for CLOCK_50MHz
set_clock_uncertainty 5 -rise_from [get_clocks {CLOCK_50MHz}]  -to [ get_clocks {*} ]
set_clock_uncertainty 5 -fall_from [get_clocks {CLOCK_50MHz}]  -to [ get_clocks {*} ]


