/*
 * MMA8541.h
 *
 *  Created on: May 13, 2021
 *      Author: rpi
 */


#ifndef MMA8451_H_
#define MMA8451_H_

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdint.h>

//MMAInit
void MMAInit(void);

//MMARd_orientation
void MMA_orientation(double res[3]);

//MMAlandsport
int MMA_landsport(void);

//MMA_lndsport
int MMA_bafro(void);

//MMA_close
void MMA_close(void);

#endif /* MMA8451_H_ */
