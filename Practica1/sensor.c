/*
 * sensor.c
 *
 *  Created on: Feb 24, 2025
 *      Author: ubuntu
 */
#include <sensor.h>

/*#define I2C_ADDRESS 0x68
#define WAKEUP_SENSOR 0x6B
#define READ_X 0x3B
#define READ_Y 0x3D
#define READ_Z 0x3F

//I2C
struct i2c_rdwr_ioctl_data packets;
struct i2c_msg messages[2];
char i2cFile[15];
int device = 1;


void initSensor(void){
	sprintf(i2cFile, "/dev/i2c-%d", device);
	int fd = open(i2cFile, O_RDWR);
	ioctl(fd, I2C_SLAVE, I2C_ADDRESS);

	//WAKE UP SENSOR
	char data[2] = {WAKEUP_SENSOR, 0};
	message[0].addr = I2C_ADDRESS;
	message[0].flags = 0;
	message[0].len = sizeof(data);
	message[0].buf = data;

	packets.msgs = messages;
	packets.nmsg = 1;

	ioctl(fd, I2C_RDWR, &packets);
}*/

#define MPU6050_ADDR 0x68  // Dirección I2C del MPU-6000
#define PWR_MGMT_1   0x6B  // Registro de gestión de energía
#define ACCEL_XOUT_H 0x3B  // Registro de aceleración en X (alto)
#define ACCEL_SCALE  16384.0  // Factor de conversión para ±2g

// Función para escribir un byte en el MPU-6000
void i2c_write(int fd, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    write(fd, buffer, 2);
}

// Función para leer múltiples bytes del MPU-6000
void i2c_read(int fd, uint8_t reg, uint8_t *buffer, uint8_t length) {
    write(fd, &reg, 1);
    read(fd, buffer, length);
}

// Función para obtener datos de aceleración
void read_acceleration(int fd, int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];
    i2c_read(fd, ACCEL_XOUT_H, data, 6);
    *ax = (data[0] << 8) | data[1];
    *ay = (data[2] << 8) | data[3];
    *az = (data[4] << 8) | data[5];
}

int main() {
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        perror("Error al abrir I2C");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, MPU6050_ADDR) < 0) {
        perror("Error al conectar con el MPU-6000");
        close(fd);
        return 1;
    }

    // Activar el sensor
    i2c_write(fd, PWR_MGMT_1, 0x00);

    int16_t ax, ay, az;
    while (1) {
        read_acceleration(fd, &ax, &ay, &az);
        double ax_g = ax / ACCEL_SCALE;
        double ay_g = ay / ACCEL_SCALE;
        double az_g = az / ACCEL_SCALE;

        printf("Aceleración: X=%d (%.2fg), Y=%d (%.2fg), Z=%d (%.2fg)\n",
               ax, ax_g, ay, ay_g, az, az_g);

        usleep(500000); // Esperar 500 ms
    }

    close(fd);
    return 0;
}



