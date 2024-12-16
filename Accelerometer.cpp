#include "Accelerometer.h"

// resolution 14 bit
Accelerometer::Accelerometer(I2C &i2c_instance) : i2c(i2c_instance)
{
    
}

void Accelerometer::initialize() {
    uint8_t data[2] = {REG_CTRL_REG_1, 0x01};  // control register_1 , 0x01 activate sensor
    writeRegs(data, 2);
}

uint8_t Accelerometer::getWhoAmI() {
    uint8_t who_am_i = 0;
    readRegs(REG_WHO_AM_I, &who_am_i, 1); // id des sensors finden (1 Byte)
    return who_am_i;
}

// Rohwert zB z.B. acc = 1234 // pro g eine digitale Zahl von 4096 [g] LSB also 1g = 4096 // 9,81 [m/s^2]
float Accelerometer::getAccX() {
    valx = static_cast<float>(getAccAxis(REG_OUT_X_MSB)) / 4096.0 * 9.81; // sensitity --> 2g mode  am ende [m/s^2]
    return valx;
}

float Accelerometer::getAccY() {
    valy = static_cast<float>(getAccAxis(REG_OUT_Y_MSB)) / 4096.0 * 9.81;
    return valy;
}

float Accelerometer::getAccZ() {
    valz = static_cast<float>(getAccAxis(REG_OUT_Z_MSB)) / 4096.0 * 9.81;
    return valz;
}


// Lesen und Schreiben von Registern des Sensors über den I2C-Bus.
void Accelerometer::readRegs(int addr, uint8_t *data, int len) {
    char t[1] = {static_cast<char>(addr)}; // addr vom sensor für x,y,z register
    i2c.write(MMA8451_I2C_ADDRESS, t, 1, true); //  1 = (1 Byte) sendet adrrese vom register damit sensor bescheid weis
    //Das true-Argument steht für einen "repeated start" auf dem I2C-Bus. kein stop signal, weiterer datenaustausch möglich
    i2c.read(MMA8451_I2C_ADDRESS, reinterpret_cast<char *>(data), len);
}

// für init wichtig
void Accelerometer::writeRegs(uint8_t *data, int len) {
    i2c.write(MMA8451_I2C_ADDRESS, reinterpret_cast<char *>(data), len);
}

// get rohdata (int)
int16_t Accelerometer::getAccAxis(uint8_t addr) {
    int16_t acc;
    uint8_t res[2]; // 2 byte Puffer (res[0]) die oberen 8 Bits enthält (res[1]) enthält die unteren 6 Bits.
    readRegs(addr, res, 2); // zwei Bytes aus den Registern des Sensors lesen

    acc = (res[0] << 6) | (res[1] >> 2); // um ein 14-Bit-Ergebnis (acc) zu erhalten.
    if (acc > UINT14_MAX / 2) { // if the first bit is positive it means negativ
        acc -= UINT14_MAX;
    }
    return acc;
}



