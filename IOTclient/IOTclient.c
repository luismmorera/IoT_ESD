//Used libraries
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <MMA8451.h>
#include <ICS34725.h>
#include <pigpio.h>

//General defines
#define DEBUG 2			//Debugging value

//UDP comunication defines
#define PORT 8080				//Port for comunication
#define MAX_MSG 1024			//size of the message
#define MAX_DATA 1024			//Size of the data
#define IP "192.168.43.164"		//Server IP

//Timing defines
#define SAMPLE_RATE 1			//Data reading date in seconds
#define SEND_RATE 10			//Server data sending rate in seconds


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




int main (void){

	double data[MAX_DATA];									//Data array for sending it to the server
	int i;													//Integer used for moving in the data array
	double data_MMA[3];										//Array for saving the orientation data from the acelerometer
	int data_ICS[4];										//Array for saving the color data from the color sensor
	int landsport, bafro;									//Integers for saving the orientation (back/front, landscape/portrait) of the acelerometer
	uint32_t n, len;										//Lengths for the communications

	int cont_ex = 1;											//Continue executing variable for the gestion of sigint
	int cnt = 0;


	int send_num = SEND_RATE / SAMPLE_RATE;						//Calcule of the itineration number for data sending


	//Variables for the UDP comunicaation
	int sock;												//Socket identifier
	struct sockaddr_in serv_addr;							//Server address
	char msg[MAX_MSG];										//array for sending messages
	char buf[MAX_MSG];										//array for receiving messages

	//Specification of the server address
	memset(&serv_addr, 0, sizeof(serv_addr));				//empty the server address

	serv_addr.sin_family = AF_INET;							//IPV4 communication
	serv_addr.sin_addr.s_addr = inet_addr(IP);				//IP of the server
	serv_addr.sin_port = htons(PORT);							//Port for UDP

	//Ceation of the socket
	er_debug("I'm going to create the socket.", 0);
	sock = socket(AF_INET, SOCK_DGRAM, 0);					//Creation of the socket
	if (sock < 0){											//Verification of the socket creation
		printf("Exiting due to an error creating socket.\n");
		exit(0);
	}
	er_debug("Socket created successfully", 1);



	len = sizeof (serv_addr);																				//Size of server address

	er_debug("I'm going to send the \"Hello server\" message.", 0);
	strcpy(msg, "Hello server");
	sendto(sock, (char*)msg, strlen(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));			//Sending the connection request to server
	er_debug("Message sent.", 1);



	er_debug("Waiting for connection confirmation.", 0);
	n = recvfrom(sock,(char *)buf, MAX_MSG, MSG_WAITALL, (struct sockaddr *)&serv_addr, &len);			//Waiting for connection ack
	er_debug("Connection confirmation received", 1);

	buf[n] = '\0';

	if (!charcmp(buf, "Hello RPI", sizeof("Hello RPI"))){												//Verification of the connection ack
		printf("Exiting due to an error in the connection confirmation.\n");
		MMA_close();
		ICS_close();
		close(sock);
		exit(0);
	}
	er_debug("Correct connection confirmation", 1);


	//Initialization of the sensors
	MMAInit();														///Initialization of the acelerometer
	ICSInit();														//Initialization of the color sensor

	while (cont_ex){
		i = 0;
		for (int a=0; a<send_num; a++){
			//lecture of the data from the acelerometer
			MMA_orientation(data_MMA);								//Reading the orientation data from the acelerometer
			er_debug("Orientation data read.", 0);
			landsport = MMA_landsport();							//Reading the landscape/portrait from the acelerometer
			er_debug("Landscape/portrait data read.", 0);
			bafro = MMA_bafro();									//Reading the back front from the acelerometer
			er_debug("Back/front data read.", 0);
			ICS_color(data_ICS);									//Reading the data color from the color sensor
			er_debug("Color data read.", 0);
			data[i] = data_MMA[0];									//Adding data to the data array
			data[i+1] = data_MMA[1];								//Estructure of the data array /0-x/1-y/2-z/3-pl/4-bf/5-c/6-r/7-g/8-b/
			data[i+2] = data_MMA[2];
			data[i+3] = (double)landsport;
			data[i+4] = (double)bafro;
			data[i+5] = (double)data_ICS[0];
			data[i+6] = (double)data_ICS[1];
			data[i+7] = (double)data_ICS[2];
			data[i+8] = (double)data_ICS[3];
			i = i + 9;

			er_debug("Data added to memory.", 0);
			sleep(SAMPLE_RATE);


		}

		if (DEBUG > 1){												//printing the data in the screen for debugging
			i = 0;
			for (int a = 0; a < send_num; a++){
				cnt++;
				printf( "\n\n\nRead num: %d.\nOrientation:\nX: %.2f\nY:%.2f\nZ: %.2f\n", cnt, data[i+0], data[i+1], data[i+2]);
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
						printf("Color:\nClear: %d\nRed: %d\nGreen: %d\nBlue: %d\n\n\n", (int)data[i+5], (int)data[i+6], (int)data[i+7], (int)data[i+8]);
						printf("-------------------------------------------------------------------------");
						i = i + 9;

			}
		}

		er_debug("I'm going to send data to the server", 0);
		sendto(sock, (double*)data, sizeof(data), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));		//Sending data to server
		er_debug("Data sent to the server", 1);

		er_debug("Waiting for data confirmation from the server.", 0);
		n = recvfrom(sock,(char *)buf, MAX_MSG, MSG_WAITALL, (struct sockaddr *)&serv_addr, &len);   		//Waiting for data ACK
		er_debug("Data confirmation received", 1);

		buf[n] = '\0';
		if (charcmp(buf, "Bye RPI", sizeof("Bye RPI"))){
			cont_ex = 0; er_debug("cont ex = 0", 1);														//Checking if the're is a connection close request
		}
		if ((!charcmp(buf, "Data received", sizeof("Data received"))) && (cont_ex == 1)){					//Verifing the data ACK
			printf("Exiting due to a error in data confirmation.");
			MMA_close();
			ICS_close();
			close(sock);
			exit(0);
		}

	}

	er_debug("Connection closed by server.", 0);
	er_debug("Sending close message \"Bye server\".", 0);
	strcpy(msg, "Bye server");
	sendto(sock, (char*)msg, strlen(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));		//Sending connection close ACK
	er_debug("Close message sent.", 1);



	printf("\nClosing APP.\n");
	MMA_close();														//Closing the acelerometer
	ICS_close();														//Closing the color sensor
	close(sock);														//Closing the socket
	printf("APP closed.\n");
	printf("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n");


	return(0);

}


//er_debug
void er_debug(char * msg, int cat){

	if (DEBUG > 0){
		printf(msg);
		printf("\n");
		if (cat == 1){
			printf("\n");
		}
	}

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

