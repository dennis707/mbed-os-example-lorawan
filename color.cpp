#include "mbed.h"
#include "color.h"

// IR-Filter:
// Der IR-Blockfilter reduziert den Einfluss 
// von Infrarotlicht. Dadurch werden die Messungen genauer und weniger von Umgebungslicht beeinflusst.

// Konstruktor zur Initialisierung der I2C-Referenz und LED-Pin
ColorSensor::ColorSensor(I2C &i2c_instance) : i2c(i2c_instance), redCount(0), greenCount(0), blueCount(0) {
    // Optional: Weitere Initialisierungen
}

// Schreiben Register
void ColorSensor::writeRegister(uint8_t reg, uint8_t value) {
    char data[2] = { (char)(TCS34725_COMMAND_BIT | reg), (char)value }; // spezifiziert registeradresse fürl ese/schreib cmd

    // COMMAND Bit 0x80 im Register von COMMAND muss 7 Bit auf 1 sein --> signalisiert befehl
    i2c.write(TCS34725_ADDRESS, data, 2); // Daten ins Register schreiben
}

// Funktion zum Lesen eines 16-Bit-Werts von einem bestimmten Register
uint16_t ColorSensor::read16(uint8_t reg) {
    char cmd = TCS34725_COMMAND_BIT | reg;
    char data[2];
    i2c.write(TCS34725_ADDRESS, &cmd, 1); // Leseanfrage senden
    i2c.read(TCS34725_ADDRESS, data, 2); // Daten lesen
    return (data[1] << 8) | data[0]; // Bytes zusammenführen
}

// Funktion zum Initialisieren des TCS34725-Sensors
void ColorSensor::init() {
    writeRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON); // PON power On --> 0
    ThisThread::sleep_for(3ms);
    writeRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN); // ADC aktivieren
    writeRegister(TCS34725_ATIME, 0xC0); // Max Integrationszeit  // 0xF6 helles licht, 0x00 für schwaches Licht, 0xC0 normale Bedingungen
    // dunkle Umgebungen --> lange Integrationszeit (0x00), helle Umgebungen genügt eine kurze Zeit (0xF6)
    writeRegister(TCS34725_CONTROL, 0x01); // Gain auf 4x
}

// Funktion zum Lesen der Farbdaten
void ColorSensor::readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue) {
    clear = read16(TCS34725_CDATAL); // without ir filter
    red = read16(TCS34725_RDATAL);
    green = read16(TCS34725_GDATAL);
    blue = read16(TCS34725_BDATAL);
    clear = clear;
    red = red;
    green = green;
    blue = blue;
}






