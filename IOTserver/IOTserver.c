#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <py_IOT_P3.h>

//Defines
#define DEBUG 2			//Debugging messages
#define MAX_MSG 1024	//MAx comunication message size
#define PORT 8080		//UDP port

#define SEND_RATE 10   	//data receiving rate in seconds
#define SAMPLE_RATE	1	//data reading rate of the client in seconds
#define CLC_RATE 60		//rate of calculatin mean max min and std
#define USE_WHATSAPP 0
#define USE_TELEGRAM 1
#define USE_GMAIL  1
#define PY_MAX_MSG 1024
#define PHONE "663860706"
#define TLGRM_ID "1127355522"
#define BLUE_MAX 170
#define RED_MAX 150
#define GREEN_MIN 220

/********************************************************************
 * Name: er_debug													*
 * Input variables: 												*
 * msg: Message for printing.										*
 * cat: if 1 a \n is printed at the end of the message				*
 * 																	*
 * Functionality: prints the indicated message in order to debug the*
 * program.															*
 *******************************************************************/
void er_debug(char * msg, int cat);


/********************************************************************
 * Name:CTRLHandler													*
 * 																	*
 * Functionality: Function executed when receiving the sigint signal* 															*
 *******************************************************************/
void CTRLCHandler (int dummy);

/********************************************************************
 * Name: charcmp													*
 * Input variables: 												*
 * msg1:first message to compare									*
 * msg1:second message to compare									*
 * n: quantity of chars to compare									*
 * 																	*
 * Return: 1 if the messages are equals, 0 if inequals				*
 * 																	*
 * Functionality:Compares the fist n characters of 2 messages	 	*
 *******************************************************************/
int charcmp(char* msg1, char* msg2, int n);

/********************************************************************
 * Name: clc_mean													*
 * Input variables: 												*
 * ant_mean: previous mean											*
 * data_qtty: total of data quantity in the previous mean			*
 * id: id for knowing in witch data has to make the mean			*
 * 																	*
 * Return:new mean value											*
 * 																	*
 * Functionality:Calculates mean of the data indicated by id	 	*
 *******************************************************************/
double clc_mean(double ant_mean, int data_qtty, int id);

/********************************************************************
 * Name: clc_max													*
 * Input variables: 												*
 * ant_max: previous max											*
 * id: id for knowing in witch data has to make the mean			*
 * 																	*
 * Return:new max value												*
 * 																	*
 * Functionality:Calculates maximum value of the data indicated 	*
 * by id	 														*
 *******************************************************************/
double clc_max(double ant_max, int id);

/********************************************************************
 * Name: clc_min													*
 * Input variables: 												*
 * ant_min: previous min											*
 * id: id for knowing in witch data has to make the mean			*
 * 																	*
 * Return:new min value												*
 * 																	*
 * Functionality:Calculates minimun value of the data indicated 	*
 * by id	 														*
 *******************************************************************/
double clc_min(double ant_min, int id);

/********************************************************************
 * Name: clc_std													*
 * Input variables: 												*
 * ant_std: previous std											*
 * mean: mean of the data.											*
 * data_qtty: total of data quantity in the previous mean			*
 * id: id for knowing in witch data has to make the mean			*
 * 																	*
 * Return:new std value												*
 * 																	*
 * Functionality:Calculates std value of the data indicated 		*
 * by id	 														*
 *******************************************************************/
double clc_std(double ant_std, double mean, int data_qtty, int id);


/********************************************************************
 * Name:wr_to_datafile												*
 * 																	*
 * Functionality: writes the data in the "IOTdata.txt* file		*
 *******************************************************************/
void wr_to_datafile(void);

/********************************************************************
 * Name:wr_to_screen												*
 * 																	*
 * Functionality: writes the data in the screen						*
 *******************************************************************/
void wr_to_screen(void);

/********************************************************************
 * Name:wr_to_clcfile												*
 * 																	*
 * Functionality: writes the calculated data (mean, min, max, 		*
 * std) in the "IOTdata.txt" file									*
 *******************************************************************/
void wr_to_clcfile(void);

/********************************************************************
 * Name:wr_clc_to_screen											*
 * 																	*
 * Functionality: writes the calculated data (mean, min, max, 		*
 * std) in the screen												*
 *******************************************************************/
void wr_clc_to_screen(void);

/********************************************************************
 * Name:getTime														*
 * 																	*
 * Functionality: gets the data and the time and puts it in 		*
 * date_time														*
 *******************************************************************/
void getDateTime(void);

/********************************************************************
 * Name:getTime														*
 * 																	*
 * Functionality: initialises the clc variavles						*
 *******************************************************************/
void clc_init(void);


void data_check(void);


int cont_ex = 1;								//continue executing

int send_num=SEND_RATE/SAMPLE_RATE; 			//integer for konwing how much data is in each sent from client

double data [100]; 								//array for saving the data

FILE* file_data;								//File for saving the data;

double mean[7];									//array with the mean values 0-x 1-y 2-z 3-clear 4-red 5-green 6-blue
double max[7];									//array with the max values 0-x 1-y 2-z 3-clear 4-red 5-green 6-blue
double min[7];									//array with the min values 0-x 1-y 2-z 3-clear 4-red 5-green 6-blue
double std[7];									//array with the std values 0-x 1-y 2-z 3-clear 4-red 5-green 6-blue

char py_msg[PY_MAX_MSG];
char py_asntoGM[PY_MAX_MSG];
char py_emailGM[PY_MAX_MSG];

char date_time[128];							//Array for the date an the hour

int main (void){

	int cont_ex_2 = 1;								//variable used for sigint gestion
	int data_cnt = 0;									//integer for konwing how much data has received the server
	int clc_it = CLC_RATE/SEND_RATE;										//number of necesary clc itinerations
	int clc_cnt = 0;									//Counter for knowing when to calculate

	uint32_t len, n;								//variables used for comunnication

	signal(SIGINT, CTRLCHandler); 					//Sets the interruption of the SIGINT signal

	//variables of the comunication UDP
	int sock;										//socket identifier
	struct sockaddr_in serv_addr;					//address of the server
	struct sockaddr_in cli_addr;					//address of the client
	char buf[MAX_MSG];								//char array for receiving messages
	char msg[MAX_MSG];								//char array for sending messages


	clc_init();										//Initialisation of the clc values

	//Setup of the addresses
	memset(&serv_addr, 0, sizeof(serv_addr));		//empty the server address
	memset(&cli_addr, 0, sizeof(cli_addr));			//empty the client address

	//Server address
	serv_addr.sin_family = AF_INET;					//IPV4 communication
	serv_addr.sin_addr.s_addr = INADDR_ANY;			//Receive data from any IP
	serv_addr.sin_port = htons(PORT);				//Port for UDP communication

	er_debug("Creating the socket.", 0);
	sock = socket(AF_INET, SOCK_DGRAM, 0);			//Creation of the socket
	if (sock<0){									//Verification of the correct creation of the socket
			printf("Exiting due to error creating the socket.\n");
			exit(0);
		}
	er_debug("Socket successfully created.", 1);

	er_debug("Making bind.", 0);
	bind(sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));								//Binding the socket
	er_debug("Bind done.", 1);

	len = sizeof(cli_addr);																			//length of the client address

	er_debug("Waiting for connection request.", 0);
	n = recvfrom(sock,(char *)buf, MAX_MSG, MSG_WAITALL, (struct sockaddr *)&cli_addr, &len);		//Waiting for receiving a connection request
	er_debug("Connection request received", 1);

	buf[n]='\0';
	if (!charcmp(buf, "Hello server", sizeof("Hello server"))){										//Verification of the connection request message
		printf("Exiting due to wrong connection request message.");
		close(sock);
		exit(0);
	}

	er_debug("Sending connection confirmation.", 0);
	strcpy(msg, "Hello RPI");
	sendto(sock, (char*)msg, strlen(msg), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));		//Sending of the connection confirmation
	er_debug("Connection confirmation sent.", 1);

	cont_ex = 1;

	file_data = fopen("IOTdata.txt","wt");															//Openning the file for saving data															//Openning file for saving the calculated data
	while (cont_ex_2){

		er_debug("Waiting for data.", 0);
		n = recvfrom(sock,(double *)data, MAX_MSG, MSG_WAITALL, (struct sockaddr *)&cli_addr, &len); //Receiving data from the client
		er_debug("Data received.", 1);

		er_debug("Sending data confirmation.", 0);
		if (cont_ex == 1){
			strcpy(msg, "Data received");															//Sending data ACK if the program has to continue running
		}else{
			strcpy(msg, "Bye RPI");																	//Sending data ACK if the program has to stop running
			cont_ex_2 = 0;
			er_debug("Closing connection",0);
		}

		sendto(sock, (char*)msg, strlen(msg), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));	//Sending ACK
		er_debug("Data confirmation sent.", 1);

		wr_to_datafile();																			//Adding data to file
		data_check();
		if (DEBUG > 0){
			wr_to_screen();
		}
		clc_cnt++;
		for (int i = 0; i < 7; i++){
			mean[i] = clc_mean(mean[i], data_cnt*10, i);												//Calculating the mean values
			max[i] = clc_max(max[i], i);																//Calculating the max values
			min[i] = clc_min(min[i], i);																//Calculating the max values
			std[i] = clc_std(std[i],mean[i], data_cnt*10,i);											//Calculating the std values
		}
		if (clc_cnt == clc_it){
			wr_to_clcfile();																			//Adding data to file
			if (DEBUG > 0){
				wr_clc_to_screen();
			}
			clc_cnt = 0;
		}
		data_cnt++;																					//incrementing the data counter
	}


	fclose(file_data);																				//Closing the data saving file

	er_debug("Waiting closing confirmation.", 0);
	n = recvfrom(sock,(char *)buf, MAX_MSG, MSG_WAITALL, (struct sockaddr *)&cli_addr, &len);		//Waiting for closin ACK
	er_debug("Closing confirmation received.", 1);
	buf[n] = '\0';
	if (!charcmp(buf, "Bye server", sizeof("Bye server"))){											//Verification of closing ACK
		printf("Exiting due to wrong connection close response.");
		fclose(file_data);																				//Closing the data saving file
		close(sock);
		exit(0);
	}

	er_debug("Closing socket", 1);
	close(sock);																					//Closing socket

	printf("\nClosing APP\nThanksFor using.\n///////////////////////////////////////////////////////////////////////////////////////\n\n");
	return 0;
}



//wr_to_datafile
void wr_to_datafile(void){
	static int i;
	static int cnt = 0;
	i = 0;

	for (int a = 0; a < send_num; a++){
		cnt++;
		er_debug("Opening file for writing data.", 0);
		getDateTime();
		fprintf(file_data, "Read num: %d ; [%s]\n\nOrientation:\nX: %.2f\nY: %.2f\nZ: %.2f\n", cnt, date_time, data[i], data[i+1], data[i+2]);
		if (data[i+3] == 0){
			fputs("Portrait/landscape: portrait.\n", file_data);
		}else{
			fputs("Portrait/landscape: landscape.\n", file_data);
		}
		if (data[i+4] == 0){
			fputs("Back/front: front.\n\n", file_data);
		}else{
			fputs("Back/front: back.\n\n", file_data);
		}
		fprintf(file_data, "Color:\nClear: %d\nRed: %d\nGreen: %d\nBlue: %d\n\n", (int)data[i+5], (int)data[i+6], (int)data[i+7], (int)data[i+8]);
		fputs("-------------------------------------------------------------------------\n\n", file_data);
		er_debug("File written.", 1);
		i = i + 9;
		}
}

void data_check(void){
	static int i;
	i=0;
	for (int a = 0; a < send_num; a++){
		if ((data[i+3] == 1) || (data[i+4] == 1)){
			if(USE_WHATSAPP){
				switch (fork()){
					case -1:
						exit(0);
						break;
					case 0:
						strcpy(py_msg, "IOT ALERT:The plant has fallen.");
						mandarMsgWA(PHONE,py_msg);
						exit(0);
						break;
				}

			}
			if(USE_TELEGRAM){
				strcpy(py_msg, "IOT ALERT: The plant has fallen.");
				mandarMsgTG(TLGRM_ID, py_msg);
			}
			if(USE_GMAIL){
				strcpy(py_msg, "The plant has fallen.");
				strcpy(py_asntoGM, "ESD ALERT");
				strcpy(py_emailGM, "posseluka@gmail.com");
				mandarEmail(py_emailGM, py_asntoGM, py_msg);
			}
			a=send_num;
		}else if ((data[i+6] > data[i+7]) || (data[i+8] > data[i+7])){
			if(USE_WHATSAPP){
				switch (fork()){
					case -1:
						exit(0);
						break;
					case 0:
						strcpy(py_msg, "IOT ALERT:The plant has fallen.");
						mandarMsgWA(PHONE,py_msg);
						exit(0);
						break;
				}
			}
			if(USE_TELEGRAM){
				strcpy(py_msg, "IOT ALERT: Unexpected color.");
				mandarMsgTG(TLGRM_ID, py_msg);
			}
			if(USE_GMAIL){
				strcpy(py_msg, "Unexpected color in your plant.");
				strcpy(py_asntoGM, "ESD ALERT");
				strcpy(py_emailGM, "l.dposse@alumnos.upm.es");
				mandarEmail(py_emailGM, py_asntoGM, py_msg);
			}
			a=send_num;
		}

		i=i+9;
	}

}

void wr_to_screen(void){
	static int i;
	static int cnt = 0;
	i = 0;
	for (int a = 0; a < send_num; a++){
		getDateTime();
		cnt++;
		printf("Read num: %d ; [%s]\nOrientation:\nX: %.2f\nY: %.2f\nZ: %.2f\n", cnt, date_time, data[i], data[i+1], data[i+2]);
		if (data[i+3] == 0){
			printf("Portrait/landscape: portrait.\n");
		}else{
			printf("Portrait/landscape: landscape.\n");
		}
		if (data[i+4] == 0){
			printf("Back/front: front.\n\n");
		}else{
			printf("Back/front: back.\n\n");
		}
		printf("Color:\nClear: %d\nRed: %d\nGreen: %d\nBlue: %d\n\n", (int)data[i+5], (int)data[i+6], (int)data[i+7], (int)data[i+8]);
		printf("-------------------------------------------------------------------------\n\n");
		i = i + 9;
		}
}

void wr_to_clcfile(void){
	static int cnt = 0;
		cnt++;
		getDateTime();
		printf("clctofile");
		fprintf(file_data, "Operation numb: %d ; [%s]\nOrientation:\nX: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", cnt, date_time, mean[0], max[0], min[0], std[0]);
		fprintf(file_data, "Y: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[1], max[1], min[1], std[1]);
		fprintf(file_data, "Z: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n\n", mean[2], max[2], min[2], std[2]);
		fprintf(file_data, "Color:\nClear: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[3], max[3], min[3], std[3]);
		fprintf(file_data, "Red: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[4], max[4], min[4], std[4]);
		fprintf(file_data, "Green: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[5], max[5], min[5], std[5]);
		fprintf(file_data, "Blue: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n\n", mean[6], max[6], min[6], std[6]);
		fputs("-------------------------------------------------------------------------\n\n", file_data);
}

//wr_clc_to_screen
void wr_clc_to_screen(void){
	static int i;
	static int cnt = 0;
	i = 0;

	for (int a = 0; a < send_num; a++){
		cnt++;
		getDateTime();
		printf("Read num: %d ; [%s]\nOrientation:\nX: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", cnt, date_time, mean[0], max[0], min[0], std[0]);
		printf("Y: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[1], max[1], min[1], std[1]);
		printf("Z: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n\n", mean[2], max[2], min[2], std[2]);
		printf("Color:\nClear: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[3], max[3], min[3], std[3]);
		printf("Red: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[4], max[4], min[4], std[4]);
		printf("Green: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n", mean[5], max[5], min[5], std[5]);
		printf("Blue: [Mean: %.3f, Max: %.3f, Min: %.3f, Std: %.3f]\n\n", mean[6], max[6], min[6], std[6]);
		printf("-------------------------------------------------------------------------\n\n");
		i=i+9;
	}
}


//clc_mean
double clc_mean(double ant_mean, int data_qtty, int id){
	static double sum;
	static int i;
	sum = 0;
	i = 0;
	if(id > 2){
		id = id + 2;
	}							//Id adjust for data array
	sum = ant_mean * data_qtty;						//obtaining the sum of the last mean
	for (int a = 0; a < send_num; a++){
		sum = sum + data[i+id];						//adding the new values to sum
		i = i + 9;
	}
	sum = sum / (data_qtty+10);						//calculating the mean value
	return sum;
}


//clc_max
double clc_max(double ant_max, int id){
	static int i;
	i = 0;
	if(id > 2){
		id = id + 2;
	}										//Id adjust
	for (int a = 0; a < send_num; a++){
		if (ant_max < data[i+id]){
			ant_max = data[i+id];
		}		//calcule of the new max
		i = i + 9;
	}
	return ant_max;
}


//clc_min
double clc_min(double ant_min, int id){
	static int i;
	i = 0;
	if(id > 2){
		id = id + 2;
	}										//Id adjust
	for (int a = 0; a < send_num; a++){
		if (ant_min > data[i+id]){
			ant_min = data[i+id];
		}		//calcule of the new min
		i = i + 9;
	}
	return ant_min;
}


//clc_std
double clc_std(double ant_std, double mean, int data_qtty, int id){
	static double sum;
	static int i;
	i = 0;
	sum = sqrt((ant_std*ant_std*data_qtty));					//Obtainning the diferences sum for the last std
	if(id > 2){
		id = id + 2;}
	for (int a=0; a < send_num; a++){
		sum = pow((data[i+id]-mean), 2);						//Calcule of the new sum
		i = i + 9;
	}
	sum = sqrt(sum/(data_qtty+10));							//calcule of the new std
	return sum;
}

void getDateTime(void){
	static time_t time_s;
	static struct tm *tlocal;
	time_s = time(0);
	tlocal = localtime(&time_s);
	strftime(date_time, 128, "%d/%m/%y %H:%M:%S", tlocal);
}

void clc_init(void){
	for (int i = 0; i < 7; i++){
		mean[i] = 0;
		max[i] = -260;
		min[i] = 260;
		std[i] = 0;
	}

}


//er_debug
void er_debug(char * msg, int cat){
	if (DEBUG > 1){							//If debug is 1 (activated) print msg
		printf("%s", msg);
		printf("\n");
		if (cat == 1){
			printf("\n");
		}			//If cat is 1 add an \n
	}
}

//CTRL_Handler
void CTRLCHandler (int dummy){
	cont_ex = 0;								//Continue executing to 0 in order to stop the execution
}

//charcmp
int charcmp(char* msg1, char* msg2, int n){
	int res = 1;

	for (int i = 0; i < n; i++){
		if (msg1[i] != msg2[i]){
			res = 0;
		}
	}

	return res;
}

