/*
 * ICS34725.c
 *
 *  Created on: May 13, 2021
 *      Author: rpi
 */

#include<ICS34725.h>

#define DEBUG 0

//Registers
#define ENABLE 0x80  	// bit 0 PON bit 1 AEN
#define STATUS 0x93		// bit 0 RGB valid
#define CDATA 0x94 		// Data start
#define SCALE 255		// Scale of the data
#define LED 23			// GPIO port


//slave addres
#define ICS34725_addr 0x29


//lengths of the arrays
#define W_LEN 2
#define R_LEN 8
#define MSG_LEN 2

//Tralation of dato to engienieering format
static double to_eng(unsigned char msb, unsigned char lsb);


//I2cStart
static void I2cStart(int addr);

//I2cWr
static void I2cWr(int fd, int addr_sl, int reg, uint16_t msg_wr, int len);

//I2cRd
static void I2cRd(int fd, int addr_sl, int reg, int len);

//ICSInit
void ICSInit(void);

//ICS_color
void ICS_color(int res[4]);

//ICS_close
void ICS_close(void);

static void er_debug(char * msg, int cat);


static int fd;
static char i2cFile[15];
static int device = 1;

static struct i2c_rdwr_ioctl_data packets;
static struct i2c_msg msg[MSG_LEN];
static unsigned char w_b[W_LEN];
static unsigned char r_b[R_LEN];

//ICSInit
void ICSInit(void){

	I2cStart(ICS34725_addr);
	// First set to 1 the PON bit in ENABLE register, in order to activate the internal oscillator and allow the
	// ADC channels to operate.
	I2cWr(fd, ICS34725_addr, ENABLE, 0x01, 2);
	usleep(2400);// we wait for 2,4ms (2400us) to activate the AEN bit
	// Activation on AEN bit
	I2cWr(fd, ICS34725_addr, ENABLE, 0x03, 2);
	sleep(1);
}

//ICS_color
void ICS_color(int res[4]){

	// We read the STATUS byte in order to know if the data is ready to be read
	I2cRd(fd, ICS34725_addr, STATUS, 1);

	if ((r_b[0]&0x01) == 0x01){ 		// If the data is ready to be read

		//We read 8 data bytes starting from the clear data
		I2cRd(fd, ICS34725_addr, CDATA, 8);

		// We use the To_eng function to pass the data into decimal units and to scale it.
		res[0] = to_eng(r_b[1],r_b[0]);
		res[1] = to_eng(r_b[3],r_b[2]);
		res[2] = to_eng(r_b[5],r_b[4]);
		res[3] = to_eng(r_b[7],r_b[6]);

	}else{
		 printf("Problem reading, the color data isn't valid.\n\n");
	}
}

//ICS_close
void ICS_close(void){
	close(fd);
}


static double to_eng(unsigned char msb, unsigned char lsb){
     static int aux;
     aux = (int)(msb << 8) + lsb;
     aux = (aux / (65535 / SCALE));
     return aux;
}

//I2cStart
static void I2cStart(int addr){

	er_debug("Realizando el I2Cstart",1);
    sprintf(i2cFile, "/dev/i2c-%d", device);
    fd = open(i2cFile, O_RDWR);             //Obtain file descriptor for RW
    ioctl(fd, I2C_SLAVE, addr);                 //Configure the file for I2C communications with slave

}


static void I2cWr(int fd, int addr_sl, int reg, uint16_t msg_wr, int len){

	er_debug("Sending message to the ICS", 1);
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

	er_debug("Reading from the ICS", 1);
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
