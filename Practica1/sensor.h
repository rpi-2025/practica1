/*
 * sensor.h
 *
 *  Created on: Feb 24, 2025
 *      Author: ubuntu
 */

#ifndef SENSOR_H_
#define SENSOR_H_
	#include <stdio.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <fcntl.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <linux/i2c-dev.h>
	#include <linux/i2c.h>
	#include <sys/ioctl.h>

	void initSensor(void);

#endif /* SENSOR_H_ */
