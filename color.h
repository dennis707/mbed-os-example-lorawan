#include "mbed.h"

// TCS34725 Address
#define TCS34725_ADDRESS 0x29 << 1 // Shifted left for 8-bit format (0x29 becomes 0x52)

// TCS34725 Registers
#define TCS34725_COMMAND_BIT 0x80 // Command bit for the sensor's registers
#define TCS34725_ENABLE 0x00       // Enable register address
#define TCS34725_ENABLE_PON 0x01   // Power ON command
#define TCS34725_ENABLE_AEN 0x02   // Enable the ADC
#define TCS34725_ATIME 0x01        // Integration time register address
#define TCS34725_CONTROL 0x0F      // Control register address
#define TCS34725_CDATAL 0x14       // Clear data register address
#define TCS34725_RDATAL 0x16       // Red data register address
#define TCS34725_GDATAL 0x18       // Green data register address
#define TCS34725_BDATAL 0x1A       // Blue data register address

class ColorSensor {
private:
    I2C &i2c; // Referenz auf die gemeinsame I2C-Instanz
    int redCount, greenCount, blueCount; // Color counters for last hour

    void writeRegister(uint8_t reg, uint8_t value);
    uint16_t read16(uint8_t reg);

    
    uint16_t clear, red, green, blue;
    
public:
    ColorSensor(I2C &i2c_instance); // Konstruktor mit I2C-Referenz
    void init();
    void readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue);
    int getRed(){return red;}
    int getGreen(){return green;}
    int getBlue(){return blue;}
};
