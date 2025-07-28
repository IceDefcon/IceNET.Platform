SPI INTERFACE

The AD9361 uses a serial peripheral interface (SPI) to
communicate with the BBP. This interface can be configured
as a 4-wire interface with dedicated receive and transmit ports,
or it can be configured as a 3-wire interface with a bidirectional
data communication port. This bus allows the BBP to set all device
control parameters using a simple address data serial bus protocol.

Write commands follow a 24-bit format. The first six bits are
used to set the bus direction and number of bytes to transfer.
The next 10 bits set the address where data is to be written. The
final eight bits are the data to be transferred to the specified register
address (MSB to LSB). The AD9361 also supports an LSB-first
format that allows the commands to be written in LSB to MSB
format. In this mode, the register addresses are incremented for
multibyte writes.

Read commands follow a similar format with the exception that
the first 16 bits are transferred on the SPI_DI pin and the final
eight bits are read from the AD9361, either on the SPI_DO pin
in 4-wire mode or on the SPI_DI pin in 3-wire mode.

[6-bit Control] [10-bit Address] [8-bit Data]

🔧 WRITE (3 bytes total):

Byte 1: [6-bit control][upper 2 bits of address]
Byte 2: [lower 8 bits of address]
Byte 3: [data to write]

📖 READ (3 bytes total):

Byte 1: [6-bit control][upper 2 bits of address]
Byte 2: [lower 8 bits of address]
Byte 3: dummy byte → replaced by read data from AD9361

✅ Example — Write 0xAB to register 0x003C:

Byte 1: 0x1C   (write, 1 byte)
Byte 2: 0x3C   (address)
Byte 3: 0xAB   (data)

✅ Example — Read from register 0x003C:

Send:    0x0C 0x3C 0x00
Receive: 0x[DATA_FROM_AD9361] on byte 3


//
// Read ID Chip
//

[23:18] Control (read)  Byte 1  0x40
[17:8]  Address (0x037) Byte 2  0x37
[7:0]   Dummy           Byte 3  0x00

//
// config :: AD9361
//

/**
 *
 * Reset AD9361
 *
 */
{ 0x00, 0x00, 0x81 },   // Soft reset
// Wait ~5ms
{ 0x1C, 0x00, 0x01 },   // Clear reset

// 2. Enable REF_CLK input
{ 0x1C, 0x09, 0x0C },   // Assume 40 MHz reference input

// 3. Set BBPLL and ADC clock (example setup)
{ 0x1C, 0x45, 0x24 },   // BBPLL divider = 24 (960 MHz)
{ 0x1C, 0x2B, 0x02 },   // ADC rate divider = 2 → 100 MHz / 2 = 50 MSPS
{ 0x1C, 0x07, 0x00 },   // Enable BBPLL
// (Optional) Poll 0xA5 to confirm PLL lock if read supported

// 4. Enable RX channels
{ 0x1C, 0x14, 0x3F },   // Enable Rx1 & Rx2 channels
{ 0x1C, 0xC3, 0x01 },   // Enable baseband RX chain

// 5. Enable LVDS data output
{ 0x1C, 0x1C, 0x0B },   // LVDS 2R2T, DDR mode
{ 0x1C, 0x1D, 0x0C },   // Enable data out

// 6. Gain mode and manual gain value
{ 0x1C, 0x109, 0x03 },  // Manual gain mode
{ 0x1C, 0x10A, 0x10 },  // Gain value (example)


// 7. Set LO frequency (example: 2400 MHz = 0x249F00 in Hz)
//    You must compute this using ADI's formula or API, here's an example:
{ 0x1C, 0x5E, 0x00 },   // LO LSB
{ 0x1C, 0x5F, 0x9F },   // LO Mid
{ 0x1C, 0x60, 0x24 },   // LO MSB

// 8. Enable RF frontend
{ 0x1C, 0x13, 0x01 },   // Enable RF receive chain

// 9. Enable State Machine (RX + TX)
{ 0x1C, 0x16, 0x03 },   // Enable RX/TX FSM






////////////////////////////////////////////////////////////////////////
//
// Minimum setup
//
////////////////////////////////////////////////////////////////////////

/**
 *
 *
 * 1. Soft Reset (Mandatory)
 *
 *
 */
{ 0x00, 0x00, 0x81 },

/* Wait ~10 ms after this. */


/**
 *
 *
 * 2. Set Clock and Interface Settings
 *
 *
 */

/* These depend on your exact system clock, but here's a general config for 50 MSPS RX using internal PLLs. */

{ 0x00, 0x03, 0x05 }, /* SPI_REG_CLOCK_ENABLE: Enable all clocks */
{ 0x00, 0x0C, 0x1B }, /* BBPLL multiplier: 0x1B (27) × REF_CLK (40 MHz) = 1080 MHz */
{ 0x00, 0x0D, 0x00 }, /* Integer-N PLL mode */
{ 0x00, 0x0E, 0x02 }, /* Enable BBPLL */
{ 0x00, 0x0F, 0x00 }, /* BBPLL control */

/**
 *
 *
 * 3. Set Sampling Rate
 *
 *
 */

/* For 50 MSPS, set the baseband clock and filters accordingly */

/**
 *
 * Assume BBPLL = 1080 MHz, so:
 *
 * Rx path divider = 1080 / 4 = 270 MHz
 *
 * Digital RX path decimation = 270 / 50 = 5.4 → Round to 5 or 6
 *
 * Here's an example assuming 6× decimation
 *
 */
{ 0x00, 0x1C, 0x05 }, /* RX path clock divider (divide by 6) */
{ 0x00, 0x1D, 0x00 }, /* RX FIR filter enable = off */

/**
 *
 *
 * 4. Enable RX Chain
 *
 *
 */
{ 0x00, 0x012, 0x01 }, /* Enable RX channel 1 */
{ 0x00, 0x014, 0x01 }, /* Enable RX synthesizer */
{ 0x00, 0x015, 0x01 }, /* Enable ADC */

/**
 *
 *
 * 5. Set RX Gain Mode and AGC
 *
 *
 */
{ 0x00, 0x109, 0x00 }, /* RX gain mode: manual */
{ 0x00, 0x10A, 0x1F }, /* Manual gain index = mid */

/**
 *
 *
 * 6. Set RF Frequency (RX LO)
 *
 *
 */

/* Assume 2.4 GHz LO for example */
{ 0x00, 0x05, 0x01 }, /* Set LO tuning mode = manual */
/* LO frequency = 2.4 GHz; use registers 0x045 – 0x048 */
{ 0x00, 0x045, 0x24 }, /* Integer part high byte */
{ 0x00, 0x046, 0x00 }, /* Integer part low byte (0x2400 = 0x2400 * 1 MHz = 2.4 GHz) */
{ 0x00, 0x047, 0x00 }, /* Fractional part high */
{ 0x00, 0x048, 0x00 }, /* Fractional part low */

/**
 *
 *
 * 7. Finalize & Start State Machine
 *
 *
 */
{ 0x00, 0x016, 0x01 }, /* Enable ENSM state machine in FDD mode */
{ 0x00, 0x017, 0x02 }, /* Go to ALERT state */
{ 0x00, 0x017, 0x03 }, /* Go to FDD (TX/RX ON) */

/**
 *
 *
 * Optional: Check Chip ID
 *
 *
 */

Send: { 0x40, 0x37, 0x00 } → Receive: 0xAD

You may need to fine-tune FIR filters, ADC clocking, and interface timing depending on your application and FPGA setup.

ADI provides tools like ADI IIO Oscilloscope and MATLAB filter wizard to generate proper FIR coefficients and register dumps.
