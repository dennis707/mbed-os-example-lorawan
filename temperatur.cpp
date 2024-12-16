#include "temperatur.h"
// temp 14 bit resolution humid 12?

// Konstruktor: speichert die I2C-Referenz
TemperatureSensor::TemperatureSensor(I2C &i2c_instance) : i2c(i2c_instance)
{
    
}

// Methode zur Messung der Luftfeuchtigkeit
float TemperatureSensor::readHumidity() {                  
    char cmd[1] = { CMD_MEASURE_HUMIDITY }; // i2c cmd Measure temperature register
    char data[2] = { 0 };
    
    i2c.write(SI7021_ADDRESS, cmd, 1); // Befehl zum Messen der Luftfeuchtigkeit senden 1--> Anzahl der Bytes
    ThisThread::sleep_for(20ms);       // Warten auf die Messung
    i2c.read(SI7021_ADDRESS, data, 2); // 2 Bytes für die Luftfeuchtigkeit lesen

    // verbindet die beiden gelesenen Bytes zu einem 16-Bit-Rohwert.
    int humidity_raw = (data[0] << 8) | data[1];
    humidity = ((125.0 * humidity_raw) / 65536) - 6.0; // Umwandlung in %

    return humidity;
}

// Methode zur Messung der Temperatur
float TemperatureSensor::readTemperature() {
    char cmd[1] = { CMD_MEASURE_TEMPERATURE };
    char data[2] = { 0 };
    
    i2c.write(SI7021_ADDRESS, cmd, 1); // Befehl zum Messen der Temperatur senden
    ThisThread::sleep_for(20ms);       // Warten auf die Messung
    i2c.read(SI7021_ADDRESS, data, 2); // 2 Bytes für die Temperatur lesen

    int temperature_raw = (data[0] << 8) | data[1]; // sechzehn
    temperature = ((175.72 * temperature_raw) / 65536) - 46.85; // Umwandlung in °C
    
    return temperature;
}







