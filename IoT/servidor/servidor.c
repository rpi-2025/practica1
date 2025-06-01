/*
 * servidor.c
 *
 *  Created on: May 5, 2025
 *      Author: ubuntu
 */


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <math.h>

#define MPU6000_SCALE_FACTOR 16384
#define TIME_MEASURE_10S 2
#define TIME_MEASURE_S (TIME_MEASURE_10S*10)
#define NUM_MUESTRAS 10
#define BUFFER_TAM (NUM_MUESTRAS * 7 * sizeof(int16_t))


int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	ssize_t nread;
	uint8_t buf[100];
	
	int six_meas = 1;

    int16_t datos_recibidos[NUM_MUESTRAS][7]; // 10 measurements, 7 values each (R, G, B, Light, Accel_X, Accel_Y, Accel_Z)
	//inicialitation of data

	float acc_x_min = 32767;
	float acc_x_max = -32768;
	float acc_x_media = 0;
	float acc_y_min = 32767;
	float acc_y_max = -32768;
	float acc_y_media = 0;
	float acc_z_min = 32767;
	float acc_z_max  = -32768;
	float acc_z_media = 0;
	float acc_x_stddev = 0;
	float acc_y_stddev = 0;
	float acc_z_stddev = 0;
	float red_stddev = 0;
	float green_stddev = 0;
	float blue_stddev = 0;
	float light_stddev = 0;

	uint16_t red_min = 256;
	uint16_t red_max = 0;
	uint16_t red_media = 0;
	uint16_t green_min = 256;
	uint16_t green_max = 0;
	uint16_t green_media = 0;
	uint16_t blue_min = 256;
	uint16_t blue_max = 0;
	uint16_t blue_media = 0;
	uint16_t light_min  = 256;
	uint16_t light_max = 0;
	uint16_t light_media = 0;
	char c_nread;


	if (argc != 2) { //if not enough arguments the program ends
	  fprintf(stderr, "Usage: %s port\n", argv[0]);
	  exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully bind(2).
	   If socket(2) (or bind(2)) fails, we (close the socket
	   and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Success */

		close(sfd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */

	/* Read datagrams and echo them back to sender */

	//infinite loop

    printf("Servidor UDP escuchando en el puerto %s...\n", argv[1]);

    while (1) {
    	peer_addr_len = sizeof(struct sockaddr_storage);
    	nread = recvfrom(sfd, datos_recibidos, BUFFER_TAM, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);  //reads data received
    	if (nread == -1)
				continue;               /* Ignore failed request */

//		if ( sendto( sfd, &nread, 1,0,(struct sockaddr *) &peer_addr,peer_addr_len) != 1)   //sends ACK, the data is the number of bytes received
//				 fprintf(stderr, "Error sending response\n");

		if (nread == BUFFER_TAM) {
		        // Obtener IP del cliente (sin resolución DNS)
		        char client_ip[INET6_ADDRSTRLEN];
		        inet_ntop(peer_addr.ss_family,
		                 peer_addr.ss_family == AF_INET ?
		                 (void*)&((struct sockaddr_in *)&peer_addr)->sin_addr :
		                 (void*)&((struct sockaddr_in6 *)&peer_addr)->sin6_addr,
		                 client_ip, sizeof(client_ip));

		        printf("Datos recibidos de %s\n", client_ip);

		        sendto(sfd, "OK", 2, 0, (struct sockaddr *)&peer_addr, peer_addr_len);

		        // Enviar ACK
				//sendto(sfd, "OK", 2, 0, (struct sockaddr *)&peer_addr, peer_addr_len);

				//char host[NI_MAXHOST], service[NI_MAXSERV];

				//s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV );
				//if (s == 0){
				//data interpretation, and save the diferent measures for the calculation



					for(int i = 0; i < (NUM_MUESTRAS); i++){
						//ROJO
						if(datos_recibidos[i][0] > red_max){
							red_max = datos_recibidos[i][0];
						}
						if(datos_recibidos[i][0] < red_min){
							red_min = datos_recibidos[i][0];
						}
						red_media += datos_recibidos[i][0];

						//VERDE
						if(datos_recibidos[i][1] > green_max){
							green_max = datos_recibidos[i][1];
						}
						if(datos_recibidos[i][1] < green_min){
							green_min = datos_recibidos[i][1];
						}
						green_media += datos_recibidos[i][1];

						//AZUL
						if(datos_recibidos[i][2] > blue_max){
							blue_max = datos_recibidos[i][2];
						}
						if(datos_recibidos[i][2] < blue_min){
							blue_min = datos_recibidos[i][2];
						}
						blue_media += datos_recibidos[i][2];

						//EJE X
						if(datos_recibidos[i][4] > acc_x_max){
							acc_x_max = datos_recibidos[i][4];
						}
						if(datos_recibidos[i][4] < acc_x_min){
							acc_x_min = datos_recibidos[i][4];
						}
						acc_x_media += datos_recibidos[i][4];

						//EJE Y
						if(datos_recibidos[i][5] > acc_y_max){
							acc_y_max = datos_recibidos[i][5];
						}
						if(datos_recibidos[i][5] < acc_y_min){
							acc_y_min = datos_recibidos[i][5];
						}
						acc_y_media += datos_recibidos[i][5];

						//EJE Z
						if(datos_recibidos[i][6] > acc_z_max){
							acc_z_max = datos_recibidos[i][6];
						}
						if(datos_recibidos[i][6] < acc_z_min){
							acc_z_min = datos_recibidos[i][6];
						}
						acc_z_media += datos_recibidos[i][6];
					}

					acc_x_media = acc_x_media/10;
					acc_y_media = acc_y_media/10;
					acc_z_media = acc_z_media/10;
					red_media   = red_media/10;
					green_media = green_media/10;
					blue_media  = blue_media/10;

					for (int i = 0; i < NUM_MUESTRAS; i++) {
						acc_x_stddev += pow(datos_recibidos[i][4] - acc_x_media, 2);
						acc_y_stddev += pow(datos_recibidos[i][5] - acc_y_media, 2);
						acc_z_stddev += pow(datos_recibidos[i][6] - acc_z_media, 2);
						red_stddev += pow(datos_recibidos[i][0] - red_media, 2);
						green_stddev += pow(datos_recibidos[i][1] - green_media, 2);
						blue_stddev += pow(datos_recibidos[i][2] - blue_media, 2);
					}

					acc_x_stddev = sqrt(acc_x_stddev / NUM_MUESTRAS);
					acc_y_stddev = sqrt(acc_y_stddev / NUM_MUESTRAS);
					acc_z_stddev = sqrt(acc_z_stddev / NUM_MUESTRAS);
					red_stddev = sqrt(red_stddev / NUM_MUESTRAS);
					green_stddev = sqrt(green_stddev / NUM_MUESTRAS);
					blue_stddev = sqrt(blue_stddev / NUM_MUESTRAS);

					system("clear");
//					printf("X axis maximum acceleration: %f\n", acc_x_max);
//					printf("X axis minimum acceleration %f\n", acc_x_min);
//					printf("X axis mean acceleration: %f\n", acc_x_media);
//					printf("Y axis maximum acceleration: %f\n", acc_y_max);
//					printf("Y axis minimum acceleration: %f\n", acc_y_min);
//					printf("Y axis mean acceleration: %f\n", acc_y_media);
//					printf("Z axis maximum acceleration: %f\n", acc_z_max);
//					printf("Z axis minimum acceleration: %f\n", acc_z_min);
//					printf("Z axis mean acceleration: %f\n", acc_z_media);
//					printf("Max Red: %d\n", red_max);
//					printf("Min Red: %d\n", red_min);
//					printf("Mean Red: %d\n", red_media);
//					printf("Max Green: %d\n", green_max);
//					printf("Min Green: %d\n", green_min);
//					printf("Mean Green: %d\n", green_media);
//					printf("Max Blue: %d\n", blue_max);
//					printf("Min Blue: %d\n", blue_min);
//					printf("Mean Blue: %d\n", blue_media);
//					printf("Max Light: %d\n", light_max);
//					printf("Min Light: %d\n", light_min);
//					printf("Mean Light: %d\n", light_media);
					printf("\n--- Estadísticas completas ---\n");
					printf("Aceleración X: Media=%.2f, StdDev=%.2f, Min=%.2f, Max=%.2f\n",
						   acc_x_media, acc_x_stddev, acc_x_min, acc_x_max);
					printf("Aceleración Y: Media=%.2f, StdDev=%.2f, Min=%.2f, Max=%.2f\n",
						   acc_y_media, acc_y_stddev, acc_y_min, acc_y_max);
					printf("Aceleración Z: Media=%.2f, StdDev=%.2f, Min=%.2f, Max=%.2f\n",
						   acc_z_media, acc_z_stddev, acc_z_min, acc_z_max);
					printf("Color Rojo: Media=%.2d, StdDev=%.2f, Min=%d, Max=%d\n",
						   red_media, red_stddev, red_min, red_max);
					printf("Color Verde: Media=%.2d, StdDev=%.2f, Min=%d, Max=%d\n",
						   green_media, green_stddev, green_min, green_max);
					printf("Color Azul: Media=%.2d, StdDev=%.2f, Min=%d, Max=%d\n",
						   blue_media, blue_stddev, blue_min, blue_max);



				//reset of the initial values of the data
					 acc_x_min = 32767;
					 acc_x_max = -32768;
					 acc_x_media = 0;
					 acc_y_min = 32767;
					 acc_y_max = -32768;
					 acc_y_media = 0;
					 acc_z_min = 32767;
					 acc_z_max  = -32768;
					 acc_z_media = 0;

					 red_min = 256;
					 red_max = 0;
					 red_media = 0;
					 green_min = 256;
					 green_max = 0;
					 green_media = 0;
					 blue_min = 256;
					 blue_max = 0;
					 blue_media = 0;

					 acc_x_stddev = 0;
					 acc_y_stddev = 0;
					 acc_z_stddev = 0;
					 red_stddev = 0;
					 green_stddev = 0;
					 blue_stddev = 0;

		}

    }


    close(sfd);
    return 0;
}
