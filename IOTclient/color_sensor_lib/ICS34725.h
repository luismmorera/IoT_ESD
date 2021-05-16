/*
 * ICS34725.h
 *
 *  Created on: May 13, 2021
 *      Author: rpi
 */

#ifndef ICS34725_H_
#define ICS34725_H_

//Used libraries
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>


//ICSInit
void ICSInit(void);

//ICS_color
void ICS_color(int res[4]);


//ICS_close
void ICS_close(void);



#endif /* ICS34725_H_ */
