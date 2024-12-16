// gps.cpp

#include "GPS.h"

// Konstruktor, der GPS-Komponenten initialisiert
GPS::GPS(PinName tx, PinName rx, PinName enablePin)

    : gpsSerial(tx, rx, 9600), pc_serial(USBTX, USBRX, 115200), gpsEnable(enablePin) {
    num_satellites = 0;
    latitude = 0.0;
    longitude = 0.0;
    altitude = 0.0;
    meridian = ' ';
    parallel = ' ';
    measurement = ' ';
    memset(gps_time, 0, sizeof(gps_time));
    memset(gps_data, 0, sizeof(gps_data));
}

// Initialisiert das GPS-Modul
void GPS::initialize() {
    printf("Starting GPS\n");
    gpsEnable = 1; // GPS-Modul aktivieren
}

// Verarbeitet NMEA-Satz und aktualisiert GPS-Daten
void GPS::parseData(char* nmea_sentence) {
    if (strstr(nmea_sentence, "$GPGGA")) { // Sucht nach GPGGA-Datensatz
        char* token = strtok(nmea_sentence, ","); // Zerlegt die Zeichenkette
        int fieldIndex = 0;
        while (token != NULL) {
            switch (fieldIndex) {
                case 1:
                    snprintf(gps_time, sizeof(gps_time), "%.2s:%.2s:%.2s", token, token + 2, token + 4); // Zeitstempel
                    break;
                case 2:
                    latitude = atof(token) / 100; //  String in einen Float 4916.45 für 49° 16.45'). Um diese in Dezimalgrad zu konvertieren, wird der Wert mit 100
                    break;
                case 3:
                    parallel = token[0];
                    break;
                case 4:
                    longitude = atof(token) / 100;
                    break;
                case 5:
                    meridian = token[0];
                    break;
                case 7:
                    num_satellites = atoi(token);
                    break;
                case 9:
                    altitude = atof(token);
                    break;
                case 10:
                    measurement = token[0] + 32; // A für gültig und V für ungültig 
                    // 32 erhöht, was dem ASCII-Wert von a entspricht, falls A als Zeichen übergeben wird (dies bedeutet, dass der Wert von Großbuchstaben in Kleinbuchstaben geändert wird).
                    break;
            }
            token = strtok(NULL, ",");
            fieldIndex++;
        }
        //sprintf(gps_data, "GPS: #Sats: %d Lat(UTC): %.6f %c Long(UTC): %.6f %c Altitude: %.1f %c GPT time: %s",
        //        num_satellites, latitude, parallel, longitude, meridian, altitude, measurement, gps_time);
        //printf("%s\n\n", gps_data);
        
        //printf("GPS: #Sats: %d Lat(UTC): %.6f %c Long(UTC): %.6f %c Altitude: %.1f %c GPT time: %s\n\n",
       //num_satellites, latitude, parallel, longitude, meridian, altitude, measurement, gps_time);

    }
}

// Methode, um GPS-Daten zu lesen und zu verarbeiten
void GPS::readAndProcessGPSData() {
    char buffer[256];
    //while (true) {
        if (gpsSerial.readable()) {
            int bytesRead = gpsSerial.read(buffer, sizeof(buffer));
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                parseData(buffer); // Verarbeitet den empfangenen GPS-Datenpuffer
            }
        //}
        //ThisThread::sleep_for(2000ms); // Kleiner Schlaf, um die CPU zu entlasten
    }
}

// Getter-Methoden zur Rückgabe der GPS-Daten
int GPS::getNumSatellites()  { return num_satellites; }
float GPS::getLatitude()  { return latitude; }
char GPS::getParallel()  { return parallel; }
float GPS::getLongitude()  { return longitude; }
char GPS::getMeridian()  { return meridian; }
float GPS::getAltitude()  { return altitude; }
char* GPS::getGPSTime()  { return gps_time; }
char GPS::getMeasurement() {return measurement;}

