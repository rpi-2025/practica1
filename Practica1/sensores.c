#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <pthread.h>

// MPU-6000
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B
#define ACCEL_SCALE  16384.0

// TCS3472
#define TCS3472_ADDR 0x29
#define COMMAND_BIT  0x80
#define ENABLE       0x00
#define ATIME        0x01
#define CONTROL      0x0F
#define CDATA        0x14
#define ENABLE_PON   0x01
#define ENABLE_AEN   0x02

// Archivo I2C
int fd_mpu, fd_tcs;

// Función para escribir en I2C
void i2c_write(int fd, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    write(fd, buffer, 2);
}

// Función para leer múltiples bytes desde I2C
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

void *read_mpu6050(void *arg) {
    int16_t ax, ay, az;
    while (1) {
        read_acceleration(&ax, &ay, &az);
        double ax_g = ax / ACCEL_SCALE;
        double ay_g = ay / ACCEL_SCALE;
        double az_g = az / ACCEL_SCALE;
        printf("[MPU-6000] Aceleración: X=%d (%.2fg), Y=%d (%.2fg), Z=%d (%.2fg)\n",
               ax, ax_g, ay, ay_g, az, az_g);
        usleep(500000); // 500 ms
    }
    return NULL;
}

// -------------------- TCS3472 --------------------
void init_tcs3472() {
    // Encender el sensor y habilitar ADC
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON);
    usleep(3000); // Esperar 3 ms para encender
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON | ENABLE_AEN);

    // Configurar tiempo de integración (700 ms)
    i2c_write(fd_tcs, COMMAND_BIT | ATIME, 0x00);

    // Configurar ganancia (1x)
    i2c_write(fd_tcs, COMMAND_BIT | CONTROL, 0x01);
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

void *read_tcs3472(void *arg) {
    uint16_t clear, red, green, blue;
    while (1) {
        read_color(&clear, &red, &green, &blue);
        printf("[TCS3472] Rojo=%d, Verde=%d, Azul=%d, Claridad=%d\n",
               red, green, blue, clear);
        if (clear != 0) {
            float r = (float)red / clear * 100;
            float g = (float)green / clear * 100;
            float b = (float)blue / clear * 100;
            printf("[TCS3472] Rojo=%.2f%%, Verde=%.2f%%, Azul=%.2f%%\n", r, g, b);
        }
        usleep(500000); // 500 ms
    }
    return NULL;
}

// -------------------- MAIN --------------------
int main() {
    // Abrir bus I2C para MPU-6000
    fd_mpu = open("/dev/i2c-1", O_RDWR);
    if (fd_mpu < 0) {
        perror("Error al abrir I2C para MPU-6000");
        return 1;
    }
    if (ioctl(fd_mpu, I2C_SLAVE, MPU6050_ADDR) < 0) {
        perror("Error al conectar con MPU-6000");
        close(fd_mpu);
        return 1;
    }
    init_mpu6050();

    // Abrir bus I2C para TCS3472
    fd_tcs = open("/dev/i2c-1", O_RDWR);
    if (fd_tcs < 0) {
        perror("Error al abrir I2C para TCS3472");
        return 1;
    }
    if (ioctl(fd_tcs, I2C_SLAVE, TCS3472_ADDR) < 0) {
        perror("Error al conectar con TCS3472");
        close(fd_tcs);
        return 1;
    }
    init_tcs3472();

    // Crear hilos
    pthread_t thread_mpu, thread_tcs;
    pthread_create(&thread_mpu, NULL, read_mpu6050, NULL);
    pthread_create(&thread_tcs, NULL, read_tcs3472, NULL);

    // Mantener el programa en ejecución
    pthread_join(thread_mpu, NULL);
    pthread_join(thread_tcs, NULL);

    close(fd_mpu);
    close(fd_tcs);
    return 0;
}



