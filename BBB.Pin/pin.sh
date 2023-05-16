#!/bin/sh -e
# turn off LEDs 1 - 3, only 0 will be on
echo 0 > /sys/class/leds/beaglebone:green:usr1/brightness
echo 0 > /sys/class/leds/beaglebone:green:usr2/brightness
echo 0 > /sys/class/leds/beaglebone:green:usr3/brightness
 
# configure UART
config-pin p9.11 uart
config-pin p9.13 uart
config-pin p8.37 uart
config-pin p8.38 uart
 
# configure P9 GPIO ports
config-pin p9.12 gpio
config-pin p9.15 gpio
config-pin p9.23 gpio
config-pin p9.27 gpio
config-pin p9.41 gpio
# configure P8 GPIO ports
config-pin p8.11 gpio
config-pin p8.12 gpio
config-pin p8.14 gpio
config-pin p8.16 gpio
config-pin p8.18 gpio
#config-pin p8.21 gpio
#config-pin p8.23 gpio
#config-pin p8.25 gpio
config-pin p8.26 gpio
config-pin p8.27 gpio
config-pin p8.28 gpio
config-pin p8.29 gpio
config-pin p8.30 gpio
config-pin p8.31 gpio
config-pin p8.32 gpio
config-pin p8.33 gpio
config-pin p8.34 gpio
config-pin p8.35 gpio
config-pin p8.36 gpio
config-pin p8.39 gpio
config-pin p8.40 gpio
config-pin p8.41 gpio
config-pin p8.42 gpio
config-pin p8.43 gpio
config-pin p8.44 gpio
config-pin p8.45 gpio
config-pin p8.46 gpio
 
# configure PWM ports
config-pin p9.14 pwm
config-pin p9.16 pwm
config-pin p9.42 pwm
config-pin p8.13 pwm
config-pin p8.19 pwm
 
# configure CAN ports
# P9_19 -> dCAN0 Rx
# P9_20 -> dCAN0 Tx
# P9_24 -> dCAN1 Rx
# P9_26 -> dCAN1 Tx
config-pin p9.19 can
config-pin p9.20 can
config-pin p9.24 can
config-pin p9.26 can
 
#SPI setup
config-pin p9.28 spi_cs
config-pin p9.29 spi
config-pin p9.30 spi
config-pin p9.31 spi_sclk
 
exit 0