#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define KTAM    1024
#define KPUERTO 9999

static int crear_socket(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("Error. No se pudo crear el socket");
    return sock;
}

static int conectar_servidor(int sock, const char *ip)
{
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(KPUERTO);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) != 1) {
        fprintf(stderr, "Error. IP no valida: %s\n", ip);
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error. No se pudo conectar con el servidor");
        return -1;
    }
    return 0;
}

static int recibir_y_mostrar(int sock)
{
    char buffer[KTAM];
    ssize_t tam;

    while ((tam = read(sock, buffer, sizeof(buffer))) != 0) {
        if (tam < 0) {
            if (errno == EINTR)
                continue;
            perror("Error al leer del socket");
            return -1;
        }
        if (fwrite(buffer, 1, (size_t)tam, stdout) != (size_t)tam) {
            perror("Error al escribir por pantalla");
            return -1;
        }
    }
    fflush(stdout);
    return 0;
}

int main(int argc, char *argv[])
{
    int sock, resultado = 0;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s IP_Servidor\n", argv[0]);
        return EXIT_FAILURE;
    }

    sock = crear_socket();
    if (sock < 0)
        return EXIT_FAILURE;

    if (conectar_servidor(sock, argv[1]) < 0 || recibir_y_mostrar(sock) < 0)
        resultado = EXIT_FAILURE;

    close(sock);
    return resultado;
}
