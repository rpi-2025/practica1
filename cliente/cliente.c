#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <arpa/inet.h>

// -------------------- CONFIGURACIÓN --------------------
#define SERVER_IP "192.168.1.211"  // Cambia por la IP de tu servidor
#define SERVER_PORT 12345
#define BUFFER_TAM 256

// MPU-6000
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B
#define ACCEL_SCALE  16384.0

// TCS3472
#define TCS3472_ADDR 0x29
#define COMMAND_BIT  0x80
#define ENABLE       0x00
#define ATIME        0x01
#define CONTROL      0x0F
#define CDATA        0x14
#define ENABLE_PON   0x01
#define ENABLE_AEN   0x02


#define NUM_MUESTRAS 10

// Archivo I2C
int fd_mpu, fd_tcs;

// Contexto UDP compartido entre hilos
struct udp_context {
    int sockfd;
    struct sockaddr_in servidor;
};

// -------------------- FUNCIONES I2C --------------------
void i2c_write(int fd, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    write(fd, buffer, 2);
}

void i2c_read(int fd, uint8_t reg, uint8_t *buffer, uint8_t length) {
    write(fd, &reg, 1);
    read(fd, buffer, length);
}

// -------------------- MPU-6000 --------------------
void init_mpu6050() {
    i2c_write(fd_mpu, PWR_MGMT_1, 0x00);
}

void read_acceleration(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];
    i2c_read(fd_mpu, ACCEL_XOUT_H, data, 6);
    *ax = (data[0] << 8) | data[1];
    *ay = (data[2] << 8) | data[3];
    *az = (data[4] << 8) | data[5];
}

void *read_mpu6050(void *arg) {
    struct udp_context *ctx = (struct udp_context *)arg;
    int16_t ax, ay, az;
    //char mensaje[BUFFER_TAM];
    char msg [256];
    char buffer_mpu[NUM_MUESTRAS][BUFFER_TAM];
        int contador = 0;
        sprintf(msg, "Hello WORLD!!!!!!!!!!!!!!");

       sendto(ctx->sockfd, msg, strlen(msg), 0,
              (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));
    while (1) {
      /*  read_acceleration(&ax, &ay, &az);
        double ax_g = ax / ACCEL_SCALE;
        double ay_g = ay / ACCEL_SCALE;
        double az_g = az / ACCEL_SCALE;

        snprintf(mensaje, BUFFER_TAM,
                 "[MPU-6000] X=%d (%.2fg), Y=%d (%.2fg), Z=%d (%.2fg)",
                 ax, ax_g, ay, ay_g, az, az_g);

        sendto(ctx->sockfd, mensaje, strlen(mensaje), 0,
               (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));

        usleep(1000000); // 1 s*/
    	read_acceleration(&ax, &ay, &az);
    	        double ax_g = ax / ACCEL_SCALE;
    	        double ay_g = ay / ACCEL_SCALE;
    	        double az_g = az / ACCEL_SCALE;

    	        snprintf(buffer_mpu[contador], BUFFER_TAM,
    	                 "%.2f %.2f %.2f",
						 	 	ay_g, az, az_g);

    	        contador++;

    	        if (contador == NUM_MUESTRAS) {
    	            // Concatenar mensajes y enviar
    	            char mensaje_total[NUM_MUESTRAS * BUFFER_TAM] = "";
    	            for (int i = 0; i < NUM_MUESTRAS; i++) {
    	                strcat(mensaje_total, buffer_mpu[i]);
    	                strcat(mensaje_total, "\n");
    	            }

    	            sendto(ctx->sockfd, mensaje_total, strlen(mensaje_total), 0,
    	                   (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));

    	            contador = 0; // Reiniciar contador
    	        }

    	        sleep(1);

    }
    return NULL;
}

// -------------------- TCS3472 --------------------
void init_tcs3472() {
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON);
    usleep(3000); // Esperar 3 ms
    i2c_write(fd_tcs, COMMAND_BIT | ENABLE, ENABLE_PON | ENABLE_AEN);
    i2c_write(fd_tcs, COMMAND_BIT | ATIME, 0x00); // Integración 700 ms
    i2c_write(fd_tcs, COMMAND_BIT | CONTROL, 0x01); // Ganancia 1x
}

uint16_t i2c_read_word(int fd, uint8_t reg) {
    uint8_t buffer[1] = {COMMAND_BIT | reg};
    write(fd, buffer, 1);

    uint8_t data[2];
    read(fd, data, 2);

    return (data[1] << 8) | data[0];
}

void read_color(uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue) {
    *clear = i2c_read_word(fd_tcs, CDATA);
    *red   = i2c_read_word(fd_tcs, CDATA + 2);
    *green = i2c_read_word(fd_tcs, CDATA + 4);
    *blue  = i2c_read_word(fd_tcs, CDATA + 6);
}

void *read_tcs3472(void *arg) {
    struct udp_context *ctx = (struct udp_context *)arg;
    uint16_t clear, red, green, blue;
    //char mensaje[BUFFER_TAM];
    char buffer_color[NUM_MUESTRAS][BUFFER_TAM];
    int contador = 0;
    while (1) {
       /* read_color(&clear, &red, &green, &blue);
        if (clear != 0) {
            float r = (float)red / clear * 100;
            float g = (float)green / clear * 100;
            float b = (float)blue / clear * 100;

            snprintf(mensaje, BUFFER_TAM,
                     "[TCS3472] R=%d G=%d B=%d Clear=%d | Porcentaje R=%.2f%% G=%.2f%% B=%.2f%%",
                     red, green, blue, clear, r, g, b);

            sendto(ctx->sockfd, mensaje, strlen(mensaje), 0,
                   (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));
        }

        usleep(1000000); // 1 s*/
    	read_color(&clear, &red, &green, &blue);
    	        if (clear != 0) {
    	            float r = (float)red / clear * 100;
    	            float g = (float)green / clear * 100;
    	            float b = (float)blue / clear * 100;

    	            snprintf(buffer_color[contador], BUFFER_TAM,
    	                     "%d %d %d %.2f %.2f %.2f",
    	                     red, green, blue, clear, r, g, b);

    	            contador++;

    	            if (contador == NUM_MUESTRAS) {
    	                char mensaje_total[NUM_MUESTRAS * BUFFER_TAM] = "";
    	                for (int i = 0; i < NUM_MUESTRAS; i++) {
    	                    strcat(mensaje_total, buffer_color[i]);
    	                    strcat(mensaje_total, "\n");
    	                }

    	                sendto(ctx->sockfd, mensaje_total, strlen(mensaje_total), 0,
    	                       (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));

    	                contador = 0;
    	            }
    	        }

    	        sleep(1);
    }
    return NULL;
}

// -------------------- MAIN --------------------
int main() {
	//struct udp_context *ctx = (struct udp_context *)arg;
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

    ctx.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx.sockfd < 0) {
        perror("Error al crear socket UDP");
        return 1;
    }

    memset(&ctx.servidor, 0, sizeof(ctx.servidor));
    ctx.servidor.sin_family = AF_INET;
    ctx.servidor.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &ctx.servidor.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(ctx.sockfd);
        return 1;
    }

    //Recive ACK de knowlegment
    char buffer_rec[BUFFER_TAM];
    int n = recvfrom(ctx.sockfd, buffer_rec, BUFFER_TAM, 0, (struct sockaddr *)&ctx->servidor, sizeof(ctx->servidor));
		if (n < 0) {
			perror("Error al recibir");

		}
		else{
			buffer_rec[n] = '\0';
					printf("Mensaje recibido desde %s:%d\n", inet_ntoa(ctx.servidor.sin_addr), ntohs(ctx.servidor.sin_port));
					printf("Contenido: %s\n", buffer_rec);
		}

    // Crear hilos
    pthread_t thread_mpu, thread_tcs;
    pthread_create(&thread_mpu, NULL, read_mpu6050, &ctx);
    pthread_create(&thread_tcs, NULL, read_tcs3472, &ctx);

    // Esperar hilos
    pthread_join(thread_mpu, NULL);
    pthread_join(thread_tcs, NULL);

    // Cerrar recursos
    close(fd_mpu);
    close(fd_tcs);
    close(ctx.sockfd);
    return 0;
}
