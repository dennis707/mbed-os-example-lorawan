#include "RGB.h"

// Pin definitions for RGB LED (common anode)
DigitalOut red_led(D15);
DigitalOut green_led(D13);
DigitalOut blue_led(D12);

void RGB::set_red()
{
    red_led = 0;   // Red (inverted for common anode)
    green_led = 1;
    blue_led = 1;
}

void RGB::set_green()
{
    red_led = 1;
    green_led = 0; // Green (inverted)
    blue_led = 1;
}


void RGB::turn_off_led()
{
    red_led = 1;   
    green_led = 1;
    blue_led = 1;
}

