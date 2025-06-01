#include "sensores.h"


// -------------------- FUNCIONES I2C --------------------
void i2c_write(int fd, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    write(fd, buffer, 2);
}

void i2c_read(int fd, uint8_t reg, uint8_t *buffer, uint8_t length) {
    write(fd, &reg, 1);
    read(fd, buffer, length);
}

// -------------------- MPU-6000 --------------------
void init_mpu6050() {
    i2c_write(fd_mpu, PWR_MGMT_1, 0x00);
}

void read_acceleration(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];
    i2c_read(fd_mpu, ACCEL_XOUT_H, data, 6);
    *ax = (data[0] << 8) | data[1];
    *ay = (data[2] << 8) | data[3];
    *az = (data[4] << 8) | data[5];
}

// -------------------- TCS3472 --------------------
void init_tcs3472() {
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON);
    usleep(3000); // Esperar 3 ms
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON | ENABLE_AEN);
    i2c_write(fd_tcs, COMMAND_BIT | ATIME, 0x00); // Integración 700 ms
    i2c_write(fd_tcs, COMMAND_BIT | CONTROL, 0x01); // Ganancia 1x
}

uint16_t i2c_read_word(int fd, uint8_t reg) {
    uint8_t buffer[1] = {COMMAND_BIT | reg};
    write(fd, buffer, 1);

    uint8_t data[2];
    read(fd, data, 2);

    return (data[1] << 8) | data[0];
}

void read_color(uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue) {
    *clear = i2c_read_word(fd_tcs, CDATA);
    *red   = i2c_read_word(fd_tcs, CDATA + 2);
    *green = i2c_read_word(fd_tcs, CDATA + 4);
    *blue  = i2c_read_word(fd_tcs, CDATA + 6);
}
