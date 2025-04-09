#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define TCS3472_ADDR 0x29
#define COMMAND_BIT  0x80

// Registros principales
#define ENABLE       0x00
#define ATIME        0x01
#define CONTROL      0x0F
#define CDATA        0x14

// Bits de configuración
#define ENABLE_PON   0x01  // Power ON
#define ENABLE_AEN   0x02  // Activar ADC

// Función para escribir un valor en un registro del TCS3472
void i2c_write(int fd, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {COMMAND_BIT | reg, value};
    write(fd, buffer, 2);
}

// Función para leer datos desde el sensor
uint16_t i2c_read_word(int fd, uint8_t reg) {
    uint8_t buffer[1] = {COMMAND_BIT | reg};
    write(fd, buffer, 1);

    uint8_t data[2];
    read(fd, data, 2);

    return (data[1] << 8) | data[0];
}

// Función para inicializar el sensor
void init_sensor(int fd) {
    // Encender el sensor y habilitar ADC
    i2c_write(fd, ENABLE, ENABLE_PON);
    usleep(3000); // Esperar 3 ms para encender
    i2c_write(fd, ENABLE, ENABLE_PON | ENABLE_AEN);

    // Configurar tiempo de integración (700 ms)
    i2c_write(fd, ATIME, 0x00);

    // Configurar ganancia (1x)
    i2c_write(fd, CONTROL, 0x01);
}

// Función para leer colores
void read_color(int fd, uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue) {
    *clear = i2c_read_word(fd, CDATA);
    *red   = i2c_read_word(fd, CDATA + 2);
    *green = i2c_read_word(fd, CDATA + 4);
    *blue  = i2c_read_word(fd, CDATA + 6);
}

int main() {
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        perror("Error al abrir I2C");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, TCS3472_ADDR) < 0) {
        perror("Error al conectar con el TCS3472");
        close(fd);
        return 1;
    }

    init_sensor(fd);

    uint16_t clear, red, green, blue;
    while (1) {
        read_color(fd, &clear, &red, &green, &blue);
        printf("Rojo: %d, Verde: %d, Azul: %d, Claridad: %d\n", red, green, blue, clear);

        // Normalizar valores de color en porcentaje
        if (clear != 0) {
            float r = (float)red / clear * 100;
            float g = (float)green / clear * 100;
            float b = (float)blue / clear * 100;
            printf("Rojo: %.2f%%, Verde: %.2f%%, Azul: %.2f%%\n", r, g, b);
        }
        usleep(500000); // Leer cada 500 ms
    }

    close(fd);
    return 0;
}


