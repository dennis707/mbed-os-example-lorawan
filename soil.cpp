#include "soil.h"
AnalogIn _sensorPin(A0);



// Liest die Bodenfeuchtigkeit und gibt sie in Prozent zurück
float SoilSensor::readMoisture() {
    soil_help = _sensorPin.read();
    sens_val = soil_help * 100.0f;
    return sens_val;  // Wandelt die analoge Eingabe in Prozent um
}

