/*
 * sensor.c
 *
 *  Created on: Feb 24, 2025
 *      Author: ubuntu
 */
#include <sensor.h>

#define I2C_ADDRESS 0x68
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
}




