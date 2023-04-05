//
//
//
#include "gpio.h"

void gpio_init()
{
    // Enable clock for PORTB
    // This is renabled in the 
    SIM_SCGC5 |= 1 << SIM_SCGC5_PORTB;  // For LED

    // Configure pins PCR21 and PCR22 as GPIO
    PORTB_PCR21 |= 1 << PORTB_PCR21_MUX;
    PORTB_PCR22 |= 1 << PORTB_PCR22_MUX;

    // Configure GPIO pins 21 and 22 as output.
    // It will have a default output value set
    // to 0, so LED will light (negative logic).
    GPIOB_PDDR |= 1 << PIN_21_N;
    GPIOB_PDDR |= 1 << PIN_22_N;

    // Set Blue pin 21 ---> OFF
    // Leave Red pin 22 in default ---> ON
    GPIOB_PDOR ^= 1 << PIN_21_N;

    // Enable clocks
    SIM_SCGC5  |= 1 << SIM_SCGC5_PORTD;
    // Configure pin 1 as GPIO
    PORTD_PCR1 |= 1 << PORTD_PCR1_MUX;
    // Configure GPIO pin 21 as output
    GPIOD_PDDR |= 1 << PIN_SPI_N;
}

void Blinky_task()
{
    // Toogle pin 22 ---> Red LED OFF
    GPIOB_PDOR ^= 1 << PIN_22_N;

    while(1) {
        GPIOB_PDOR ^= 1 << PIN_21_N;            // LED Pin   ---> Toggle with XOR
        GPIOD_PDOR ^= 1 << PIN_SPI_N;           // SPI CLock ---> Toggle with XOR
        vTaskDelay(500 / portTICK_PERIOD_MS);   // 500/10
    }
}