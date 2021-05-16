/*
 * MMA8451.c
 *
 *  Created on: May 13, 2021
 *      Author: rpi
 */

#include <MMA8451.h>

#define DEBUG 0

//Registers
#define OUT_X_MSB 0x01
#define F_SETUP 0x09
#define PL_STATUS 0x10
#define PL_CFG 0x11
#define XYZ_DATA_CFG 0x0E
#define CTRL_REG1 0x2A

//I2C Address of the acelerometer
#define MMA8451_addr 0x1C

//Lengths of the arrays
#define w_len 2
#define r_len 6
#define msg_len 2

//I2cStart
static void I2cStart(int addr);

//I2cWr
static void I2cWr(int fd, int addr_sl, int reg, uint16_t msg_wr, int len);

//I2cRd
static void I2cRd(int fd, int addr_sl, int reg, int len);

//MMAstart
void MMAInit(void);

void MMA_orientation(double res[3]);

//MMA_lndsport
int MMA_landsport(void);

//MMA_lndsport
int MMA_bafro(void);

//MMA_close
void MMA_close(void);

static void er_debug(char * msg, int cat);

//Traduction to engieneering units of the data from the acelerometer
static double to_eng(signed char msb, signed char lsb);

//globaL variables of i2c
static char i2cFile[15];
static int device = 1;
static int fd;
static struct i2c_rdwr_ioctl_data packets;
static struct i2c_msg msg[msg_len];
static unsigned char w_b[w_len];
static signed char r_b[r_len];



void MMAInit(void){

	er_debug("Comienzo del init.", 1);

	//Setup of I2C
	I2cStart(MMA8451_addr);

    //Inactive mode CTRL_REG1. We set the sensor to inactive mode to configure the registers
    I2cWr(fd, MMA8451_addr,CTRL_REG1, 0x00, 2);

    //Disable FIFO
    I2cWr(fd, MMA8451_addr, F_SETUP, 0x00, 2);

    //Ended initialization of the default values

    //Write XYZ_DATA_CFG message. We set the scale to 4g
    I2cWr(fd, MMA8451_addr, XYZ_DATA_CFG, 0x01, 2);

    //Write PL_CFG message. We enable the landscape and portrait detection
    I2cWr(fd, MMA8451_addr, PL_CFG, 0xC0, 2);

    //Write CTRL_REG1 message. We set the sensor to active mode
    I2cWr(fd, MMA8451_addr, CTRL_REG1, 0x01, 2);
}

void MMA_orientation(double res[3]){

    //Read XYZ
    I2cRd(fd,  MMA8451_addr, OUT_X_MSB, 6);

	//Translation to engienieering units, 2048/g (4g)
	res[0]=to_eng(r_b[0], r_b[1]);
	res[1]=to_eng(r_b[2], r_b[3]);
	res[2]=to_eng(r_b[4], r_b[5]);

}

//MMA_lndsport
//0 if portrait 1 if landscape
int MMA_landsport(void){
	int res;
	u_int8_t pl;

    //Read PL_STATUS
    I2cRd(fd,  MMA8451_addr, PL_STATUS, 1);

    pl = r_b[0] & 0x04; // we get the 3rd bit (landscape/portrait)

    if(pl == 0x00){ //if its on portrait position
    	res = 0;
    }else if (pl == 0x01){ //if its in landscape position
    	res = 1;
    }else{
    	res = -1;
    }
    return res;
}

//MMA_bafro
//0 front 1 back
int MMA_bafro(void){
	int res;
	u_int8_t bf;

    //Read PL_STATUS
	I2cRd(fd,  MMA8451_addr, PL_STATUS, 1);

	bf = r_b[0] & 0x01; // we get the 1st bit (back/front)

    if(bf == 0x00){ //if its on front position
    	res = 0;
    }else if (bf == 0x01){ //if its in back position
    	res = 1;
    }else{
    	res = -1;
    }
    return res;
}

//MMA_close
void MMA_close(void){

	close(fd);

}




double to_eng(signed char msb, signed char lsb){
    static int16_t aux;
    static double res;

    aux = (msb << 6) + (lsb >> 2); //We get the 14 bits we need
    res = (double)aux;
    res = res / 2048;
    return res;
}

//I2cStart
static void I2cStart(int addr){

	er_debug("Comienzo del I2c start", 1);
    sprintf(i2cFile, "/dev/i2c-%d", device);
    fd = open(i2cFile, O_RDWR);             //Obtain file descriptor for RW
    ioctl(fd, I2C_SLAVE, addr);                 //Configure the file for I2C communications with slave


}


static void I2cWr(int fd, int addr_sl, int reg, uint16_t msg_wr, int len){

	er_debug("Enviando mensaje",1);
	w_b[0]             = reg;
	w_b[1]             = msg_wr;
	msg[0].addr        = addr_sl;
	msg[0].flags       = 0;
	msg[0].len         = len;
	msg[0].buf         = w_b;

	//Build CTRL_REG1 packet
	packets.msgs       = msg;
	packets.nmsgs      = 1;

	ioctl(fd, I2C_RDWR, &packets);


}


//I2cRd
static void I2cRd(int fd, int addr_sl, int reg, int len){

	er_debug("Leyendo datos",1);

	//Read XYZ
	w_b[0]             = reg;
	msg[0].addr        = addr_sl;
	msg[0].flags       = 0;
	msg[0].len         = 1;
	msg[0].buf         = w_b;

	msg[1].addr        = addr_sl;
	msg[1].flags       = I2C_M_RD;
	msg[1].len         = len;
	msg[1].buf         = r_b;

	//Build read packet
	packets.msgs       = msg;
	packets.nmsgs      = 2;

	ioctl(fd, I2C_RDWR, &packets);

}



static void er_debug(char * msg, int cat){

	if (DEBUG == 1){
		if (cat == 1){
			printf("\n");
		}
		printf(msg);
		printf("\n");
		if (cat == 1){
			printf("\n");
		}
	}

}
