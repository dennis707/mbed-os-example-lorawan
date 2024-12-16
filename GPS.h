// gps.h
#include "mbed.h"
#include <cstring>
#include <cstdlib>

class GPS {

    private:
    // GPS-Komponenten
    BufferedSerial gpsSerial;
    BufferedSerial pc_serial;
    DigitalOut gpsEnable;

    // GPS-Daten
    int num_satellites;
    float latitude;
    float longitude;
    char meridian;
    char parallel;
    float altitude;
    char measurement;
    char gps_time[10];

    // Puffer für GPS-Daten
    char gps_data[256];

public:
    // Konstruktor
    GPS(PinName tx, PinName rx, PinName enablePin);

    // Initialisiert das GPS-Modul
    void initialize();

    // NMEA-Satz verarbeiten und GPS-Daten aktualisieren
    void parseData(char* nmea_sentence);

    // Getter-Methoden
    int getNumSatellites();
    float getLatitude();
    char getParallel();
    float getLongitude();
    char getMeridian();
    float getAltitude();
    char* getGPSTime();
    char getMeasurement();

    
    // Methode, um GPS-Daten einzulesen und zu verarbeiten (ohne Thread)
    void readAndProcessGPSData();

};

