#include "sensores.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

// -------------------- CONFIGURACIÓN --------------------
#define BUFFER_TAM 256
#define ACK_TIMEOUT_SEC 2
#define NUM_MUESTRAS 10

// Archivo I2C
int fd_mpu, fd_tcs;

// Contexto UDP compartido entre hilos
struct udp_context {
    int sockfd;
    struct sockaddr_in servidor;
};

void *read_sensors(void *arg);

void print_usage(const char *program_name) {
    fprintf(stderr, "Uso: %s <IP_SERVIDOR> <PUERTO>\n", program_name);
    fprintf(stderr, "Ejemplo: %s 192.168.0.26 12345\n", program_name);
}

// -------------------- MAIN --------------------
int main(int argc, char *argv[]) {
	if (argc != 3) {
		print_usage(argv[0]);
		return 1;
	}

	const char *server_ip = argv[1];
	int server_port = atoi(argv[2]);

	if (server_port <= 0 || server_port > 65535) {
		fprintf(stderr, "Puerto inválido. Debe estar entre 1 y 65535\n");
		return 1;
	}

	struct udp_context ctx;
    // Inicializar I2C para MPU-6000
    fd_mpu = open("/dev/i2c-1", O_RDWR);
    if (fd_mpu < 0) {
        perror("Error al abrir I2C para MPU-6000");
        return 1;
    }
    if (ioctl(fd_mpu, I2C_SLAVE, MPU6050_ADDR) < 0) {
        perror("Error al conectar con MPU-6000");
        close(fd_mpu);
        return 1;
    }
    init_mpu6050();

    // Inicializar I2C para TCS3472
    fd_tcs = open("/dev/i2c-1", O_RDWR);
    if (fd_tcs < 0) {
        perror("Error al abrir I2C para TCS3472");
        return 1;
    }
    if (ioctl(fd_tcs, I2C_SLAVE, TCS3472_ADDR) < 0) {
        perror("Error al conectar con TCS3472");
        close(fd_tcs);
        return 1;
    }
    init_tcs3472();

    // Crear socket UDP
    //struct udp_context ctx;
    ctx.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx.sockfd < 0) {
        perror("Error al crear socket UDP");
        return 1;
    }


    memset(&ctx.servidor, 0, sizeof(ctx.servidor));
    ctx.servidor.sin_family = AF_INET;
    ctx.servidor.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &ctx.servidor.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(ctx.sockfd);
        return 1;
    }

    //Recive ACK de knowlegment
    /*char buffer_rec[2];
    int n = recvfrom(ctx.sockfd, buffer_rec, 2, 0, (struct sockaddr *)&ctx.servidor, sizeof(ctx.servidor));
	if (n < 0) {
		perror("Error al recibir");
	}
	else{
		buffer_rec[n] = '\0';
		printf("ACK recibido");
	}*/

    // Crear hilos
    pthread_t thread_sensors;

    pthread_create(&thread_sensors, NULL, read_sensors, &ctx);
    // Esperar hilos
    pthread_join(thread_sensors, NULL);


    // Cerrar recursos
    close(fd_mpu);
    close(fd_tcs);
    close(ctx.sockfd);
    return 0;
}

void *read_sensors(void *arg) {
    struct udp_context *ctx = (struct udp_context *)arg;
    uint16_t clear, red, green, blue;
    int16_t ax, ay, az;
    char ack_buffer[3] = {0};
    int16_t buffer[NUM_MUESTRAS][7];
    int contador = 0;
    struct timeval tv;

    tv.tv_sec = 2;
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1) {
    	read_color(&clear, &red, &green, &blue);
        read_acceleration(&ax, &ay, &az);
    	
        buffer[contador][0] = (int16_t)red;
        buffer[contador][1] = (int16_t)green;
        buffer[contador][2] = (int16_t)blue;
        buffer[contador][3] = (int16_t)clear;
        buffer[contador][4] = ax;
        buffer[contador][5] = ay;
        buffer[contador][6] = az;

        contador++;

        if (contador == NUM_MUESTRAS) {
        
            ssize_t sent = sendto(ctx->sockfd, buffer, sizeof(buffer), 0,
                    (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));
            if (sent == sizeof(buffer)) {
            	printf("Datos enviados: %d muestras\n", NUM_MUESTRAS);

            	socklen_t len = sizeof(ctx->servidor);
            	ssize_t ack = recvfrom(ctx->sockfd, ack_buffer, sizeof(ack_buffer) - 1, 0, (struct sockaddr *)&ctx->servidor, &len);

            	if (ack > 0) {
            		ack_buffer[ack] = '\0';
            		printf("ACK recibido: %s\n", ack_buffer);
            	} else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            		printf("Timeout ACK - Continuando...\n");
            	} else {
            		perror("Error recibiendo ACK");
            	}
            } else {
            	perror("Error al enviar");
            }

            contador = 0;
        }
        sleep(1);
	}
    return NULL;
}
