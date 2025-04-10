#
# For GPIO set in XDC file
#
# PIN_E6 :: HD_GPIO_9 :: Pin 16
# PIN_E5 :: HD_GPIO_10 :: Pin 18
#
set_property IOSTANDARD LVCMOS12 [get_ports {gpio_rtl_tri_io[0]}]
set_property PACKAGE_PIN E6 [get_ports {gpio_rtl_tri_io[0]}]

set_property IOSTANDARD LVCMOS12 [get_ports {gpio_rtl_tri_io[1]}]
set_property PACKAGE_PIN E5 [get_ports {gpio_rtl_tri_io[1]}]




//
// Check AXI bus address specified in Vivado block design for GPIO's
//
cat /sys/class/gpio/gpiochip506/label
a0000000.gpio

/* Enable GPIO's */
echo -n 506 > /sys/class/gpio/export 
echo -n 507 > /sys/class/gpio/export 

/* Set direction */
echo out > /sys/class/gpio/gpio506/direction
echo out > /sys/class/gpio/gpio507/direction

/* io control */
echo 0 > /sys/class/gpio/gpio506/value
echo 0 > /sys/class/gpio/gpio507/value
echo 1 > /sys/class/gpio/gpio506/value
echo 1 > /sys/class/gpio/gpio507/value

