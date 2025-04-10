
//
// Check GPIO
//
cat /sys/kernel/debug/gpio

/**
 *
 * BMI160 Connected
 *
 * SPI0_MOSI :: H40-19 :: SDA :: OUT
 * SPI0_MISO :: H40-21 :: SAO :: IN
 * SPI0_SCLK :: H40-23 :: SCL :: OUT
 * SPI0_CS0  :: H40-24 :: CS  :: OUT
 *
 * SPI0_MOSI :: H40-37
 * SPI0_MISO :: H40-22
 * SPI0_SCLK :: H40-13
 * SPI0_CS0  :: H40-18
 *
 */

//
// GPIO
//

Change state to high:

echo 79 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio79/direction
echo 1 > /sys/class/gpio/gpio79/value

Change state to low:

echo 79 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio79/direction
echo 0 > /sys/class/gpio/gpio79/value

Read the GPIO value:

cat /sys/class/gpio/gpio79/value
