#include "brightness.h"
#include "mbed.h"
AnalogIn brightness_sensor(A2);



float Brightness::read() 
{    
    bright =  brightness_sensor.read();
    brightness_val = bright * 100.0f;  // Read brightness (0.0 - 1.0)          
    return brightness_val;
}





