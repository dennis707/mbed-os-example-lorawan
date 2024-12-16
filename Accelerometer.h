#include "mbed.h"
#include <cstdint>

#define MMA8451_I2C_ADDRESS (0x1d << 1)
#define REG_WHO_AM_I        0x0D
#define REG_CTRL_REG_1      0x2A
#define REG_OUT_X_MSB       0x01
#define REG_OUT_Y_MSB       0x03
#define REG_OUT_Z_MSB       0x05
#define UINT14_MAX          16383

class Accelerometer {
private:
    I2C &i2c; // Referenz auf die gemeinsame I2C-Instanz

    float valx,valy,valz;
    void readRegs(int addr, uint8_t *data, int len);
    void writeRegs(uint8_t *data, int len);
    int16_t getAccAxis(uint8_t addr);

public:
    Accelerometer(I2C &i2c_instance); // Konstruktor mit I2C-Referenz
    void initialize();
    uint8_t getWhoAmI();
    float getAccX();
    float getAccY();
    float getAccZ();

};
