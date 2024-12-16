#include "mbed.h"

class SoilSensor {
private:
    
    float sens_val;
    float soil_help;

public:
    
    // Methode zum Lesen der Bodenfeuchtigkeit in Prozent
    float readMoisture();
    
};

