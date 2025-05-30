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

#define BUFFER_TAM 1024
#define MPU6000_SCALE_FACTOR 16384
#define TIME_MEASURE_10S 2
#define TIME_MEASURE_S (TIME_MEASURE_10S*10)

typedef struct {
	float accel_x;
	float accel_y;
	float accel_z;
} t_accel_data;

typedef struct{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
    uint8_t Light;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    float f_accel_x;
    float f_accel_y;
    float f_accel_z;
} data_received;

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	ssize_t nread;
	uint8_t buf[100];
	data_received datos[TIME_MEASURE_S];
	int six_meas = 1;

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


//    int sockfd;
//    char buffer[BUFFER_TAM];
//    struct sockaddr_in servidor, cliente;
//    socklen_t len_cliente = sizeof(cliente);

//    // Crear el socket UDP
//    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//        perror("Error al crear el socket");
//        exit(EXIT_FAILURE);
//    }
//
//    // Configurar la estructura del servidor
//    memset(&servidor, 0, sizeof(servidor));
//    servidor.sin_family = AF_INET;
//    servidor.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las interfaces
//    servidor.sin_port = htons(PUERTO);
//
//    // Enlazar el socket al puerto
//    if (bind(sockfd, (const struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
//        perror("Error al enlazar");
//        close(sockfd);
//        exit(EXIT_FAILURE);
//    }

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
        // Limpiar buffer
//        memset(buffer, 0, BUFFER_TAM);
//
//        // Recibir mensaje del cliente
//        int n = recvfrom(sockfd, buffer, BUFFER_TAM, 0, (struct sockaddr *)&cliente, &len_cliente);
//        if (n == -1)
//        	continue;               /* Ignore failed request */
//
//		if ( sendto( sfd, &nread, 1,0,(struct sockaddr *) &peer_addr,peer_addr_len) != 1)   //sends ACK, the data is the number of bytes received
//				 fprintf(stderr, "Error sending response\n");
//
//		char host[NI_MAXHOST], service[NI_MAXSERV];
//
//		s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV );

    	peer_addr_len = sizeof(struct sockaddr_storage);
    	nread = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *) &peer_addr, &peer_addr_len);  //reads data received
		if (nread == -1)
				continue;               /* Ignore failed request */

		if ( sendto( sfd, &nread, 1,0,(struct sockaddr *) &peer_addr,peer_addr_len) != 1)   //sends ACK, the data is the number of bytes received
				 fprintf(stderr, "Error sending response\n");

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV );
		if (s == 0){
		//data interpretation, and save the diferent measures for the calculation
			for(int i = 0; i < 10; i++){
				datos[i + (six_meas - 1)*10].Red = buf[i*10];
				datos[i + (six_meas - 1)*10].Green = buf[i*10+1];
				datos[i + (six_meas - 1)*10].Blue = buf[i*10+2];
				datos[i + (six_meas - 1)*10].Light = buf[i*10+3];
				datos[i + (six_meas - 1)*10].accel_x = ((buf[i*10+4] << 8) | buf[i*10+5]);
				datos[i + (six_meas - 1)*10].accel_y = ((buf[i*10+6] << 8) | buf[i*10+7]);
				datos[i + (six_meas - 1)*10].accel_z = ((buf[i*10+8] << 8) | buf[i*10+9]);
				datos[i + (six_meas - 1)*10].f_accel_x = (datos[i + (six_meas - 1)*10].accel_x*9.81)/MPU6000_SCALE_FACTOR;
				datos[i + (six_meas - 1)*10].f_accel_y = (datos[i + (six_meas - 1)*10].accel_y*9.81)/MPU6000_SCALE_FACTOR;
				datos[i + (six_meas - 1)*10].f_accel_z = (datos[i + (six_meas - 1)*10].accel_z*9.81)/MPU6000_SCALE_FACTOR;
			}
			if (six_meas == TIME_MEASURE_10S){

			for(int i = 0; i < (TIME_MEASURE_S); i++){

				if(datos[i].Red > red_max){
					red_max = datos[i].Red;
				}
				if(datos[i].Red < red_min){
					red_min = datos[i].Red;
				}
				red_media += datos[i].Red;

				if(datos[i].Green > green_max){
					green_max = datos[i].Green;
				}
				if(datos[i].Green < green_min){
					green_min = datos[i].Green;
				}
				green_media += datos[i].Green;

				if(datos[i].Blue > blue_max){
					blue_max = datos[i].Blue;
				}
				if(datos[i].Blue < blue_min){
					blue_min = datos[i].Blue;
				}
				blue_media += datos[i].Blue;

				if(datos[i].Light > light_max){
					light_max = datos[i].Light;
				}
				if(datos[i].Light < light_min){
					light_min = datos[i].Light;
				}
				light_media += datos[i].Light;

				if(datos[i].f_accel_x > acc_x_max){
					acc_x_max = datos[i].f_accel_x;
				}
				if(datos[i].f_accel_x < acc_x_min){
					acc_x_min = datos[i].f_accel_x;
				}
				acc_x_media += datos[i].f_accel_x;

				if(datos[i].f_accel_y > acc_y_max){
					acc_y_max = datos[i].f_accel_y;
				}
				if(datos[i].f_accel_y < acc_y_min){
					acc_y_min = datos[i].f_accel_y;
				}
				acc_y_media += datos[i].f_accel_y;

				if(datos[i].f_accel_z > acc_z_max){
					acc_z_max = datos[i].f_accel_z;
				}
				if(datos[i].f_accel_z < acc_z_min){
					acc_z_min = datos[i].f_accel_z;
				}
				acc_z_media += datos[i].f_accel_z;
			}

			acc_x_media = acc_x_media/(TIME_MEASURE_S);
			acc_y_media = acc_y_media/(TIME_MEASURE_S);
			acc_z_media = acc_z_media/(TIME_MEASURE_S);
			red_media = red_media/(TIME_MEASURE_S);
			green_media = green_media/(TIME_MEASURE_S);
			blue_media = blue_media/(TIME_MEASURE_S);
			light_media = light_media/(TIME_MEASURE_S);
			//system("clear");
			printf("X axis maximum acceleration: %f\n", acc_x_max);
			printf("X axis minimum acceleration %f\n", acc_x_min);
			printf("X axis mean acceleration: %f\n", acc_x_media);
			printf("Y axis maximum acceleration: %f\n", acc_y_max);
			printf("Y axis minimum acceleration: %f\n", acc_y_min);
			printf("Y axis mean acceleration: %f\n", acc_y_media);
			printf("Z axis maximum acceleration: %f\n", acc_z_max);
			printf("Z axis minimum acceleration: %f\n", acc_z_min);
			printf("Z axis mean acceleration: %f\n", acc_z_media);
			printf("Max Red: %d\n", red_max);
			printf("Min Red: %d\n", red_min);
			printf("Mean Red: %d\n", red_media);
			printf("Max Green: %d\n", green_max);
			printf("Min Green: %d\n", green_min);
			printf("Mean Green: %d\n", green_media);
			printf("Max Blue: %d\n", blue_max);
			printf("Min Blue: %d\n", blue_min);
			printf("Mean Blue: %d\n", blue_media);
			printf("Max Light: %d\n", light_max);
			printf("Min Light: %d\n", light_min);
			printf("Mean Light: %d\n", light_media);


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
			 light_min  = 256;
			 light_max = 0;
			 light_media = 0;
		}
		if(six_meas == TIME_MEASURE_10S){
			six_meas = 1;
		}else six_meas++;

	}else
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

	}

    close(sfd);
    return 0;
}
