#ifndef SENSORES_H
#define SENSORES_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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

// Variables compartidas
extern int fd_mpu;
extern int fd_tcs;

//Funciones 

void init_tcs3472();
void read_color(uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue);

void init_mpu6050() ;
void read_acceleration(int16_t *ax, int16_t *ay, int16_t *az);


#endif // SENSORES_H



