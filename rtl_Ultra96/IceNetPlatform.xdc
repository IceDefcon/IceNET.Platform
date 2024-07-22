# Author: Ice.Marek
# IceNET Technology 2024
#

# GPIO
# PIN_E6 :: HD_GPIO_9 :: Pin 16
# PIN_E5 :: HD_GPIO_10 :: Pin 18
#
set_property IOSTANDARD LVCMOS12 [get_ports {gpio_rtl_tri_io[0]}]
set_property PACKAGE_PIN E6 [get_ports {gpio_rtl_tri_io[0]}]

set_property IOSTANDARD LVCMOS12 [get_ports {gpio_rtl_tri_io[1]}]
set_property PACKAGE_PIN E5 [get_ports {gpio_rtl_tri_io[1]}]
