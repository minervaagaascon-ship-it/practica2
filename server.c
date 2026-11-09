#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define KTAM 1024
#define KPUERTO 9999
#define KARCHIVO "Google.html"


static int crear_servidor(void)
{
	struct sockaddr_in serverAddress;
	int sock, opcion = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Error. No se pudo abrir el socket");
		return -1;
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_port = htons(KPUERTO);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));

	if (bind(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		perror("Error no se pudo vincular el socket");
		close(sock);
		return -1;
	}
	if (listen(sock, 5) < 0) {	// cola de peticiones hasta 5.
		perror("error en listen");
		close(sock);
		return -1;
	}
	return sock;
}

static void enviar_fichero(int newSock)
{
	char buffer[KTAM];
	ssize_t leidos, enviados, escritos;
	int fichOrigen;

	fichOrigen = open(KARCHIVO, O_RDONLY);
	if (fichOrigen < 0) {
		perror("error al abrir el fichero");
		return;
	}

	leidos = read(fichOrigen, buffer, KTAM);
	while (leidos > 0) {
		printf("he leido del fichero un bloque de %zd\n", leidos);
		enviados = 0;
		while (enviados < leidos) {	
			escritos = write(newSock, buffer + enviados, leidos - enviados);
			if (escritos <= 0) {
				perror("error al enviar al cliente");
				break;
			}
			enviados += escritos;
		}
		if (enviados < leidos) break;
		leidos = read(fichOrigen, buffer, KTAM);
	}
	if (leidos < 0) perror("error al leer el fichero");
	printf("ya he acabado, tamaño del ultimo bloque leido %zd\n", leidos);
	close(fichOrigen);
}

int main(void)
{
	socklen_t tam;
	struct sockaddr_in clientAddress;
	int sock,
	    newSock;	
	pid_t pid;

	sock = crear_servidor();
	if (sock < 0)
		return 1;

	signal(SIGCHLD, SIG_IGN);	
	signal(SIGPIPE, SIG_IGN);

	while (1) {
		tam = sizeof(clientAddress);
		newSock = accept(sock, (struct sockaddr *) &clientAddress, &tam);
		if (newSock < 0) {
			perror("error al aceptar la conexion");
			continue;
		}

		printf("aceptando conexion\n");
		fflush(stdout);		
		pid = fork();

		if (pid == 0) {		
			close(sock);	
			printf("hijo acepta conexion\n");
			enviar_fichero(newSock);
			printf("soy el que atendia la peticion, ya he acabado\n");
			close(newSock);
			fflush(stdout);
			_exit(0);	
		}
		else if (pid > 0) {	
			close(newSock);
		}
		else {
			perror("error en fork");
			close(newSock);
		}
	}
	return 0;
}
