/*
 * servidor.c
 *
 *  Created on: May 5, 2025
 *      Author: ubuntu
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PUERTO 12345
#define BUFFER_TAM 1024

int main() {
    int sockfd;
    char buffer[BUFFER_TAM];
    struct sockaddr_in servidor, cliente;
    socklen_t len_cliente = sizeof(cliente);

    // Crear el socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la estructura del servidor
    memset(&servidor, 0, sizeof(servidor));
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las interfaces
    servidor.sin_port = htons(PUERTO);

    // Enlazar el socket al puerto
    if (bind(sockfd, (const struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Error al enlazar");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor UDP escuchando en el puerto %d...\n", PUERTO);

    while (1) {
        // Limpiar buffer
        memset(buffer, 0, BUFFER_TAM);

        // Recibir mensaje del cliente
        int n = recvfrom(sockfd, buffer, BUFFER_TAM, 0, (struct sockaddr *)&cliente, &len_cliente);
        if (n < 0) {
            perror("Error al recibir");
            continue;
        }

        buffer[n] = '\0';
        printf("Mensaje recibido desde %s:%d\n", inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port));
        printf("Contenido: %s\n", buffer);

        // Enviar respuesta opcional
        const char *respuesta = "Mensaje recibido por el servidor.";
        sendto(sockfd, respuesta, strlen(respuesta), 0, (struct sockaddr *)&cliente, len_cliente);
    }

    close(sockfd);
    return 0;
}
