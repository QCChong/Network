#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "color.h"
typedef struct 
{
	int hour;
	int min;
	char name[20];
	char content[256];
}message;

void *recv_msg(void * arg){
	int sockfd = *(int*)arg;
	char buffer[512];
	ssize_t res;

	message msg;
	// Now read server response
	while(1){
		bzero(buffer, 512);
		res = read(sockfd, buffer, sizeof(buffer));
		if ( res < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		else if(res == 0){
			printf(L_RED"No information is obtained from the server, the server may have been shut down."NONE"\n");
			exit(0);
			break;
		}
		else{
			memcpy(&msg,&buffer,sizeof(message));
			printf(L_BLUE"{%02d:%02d} "L_PURPLE"[%s] "YELLOW"%s"NONE"\n",msg.hour, msg.min, msg.name, msg.content);
		}
	}
	pthread_exit(NULL);
}

void *send_msg(void *arg){
	int sockfd = *(int*)arg;
	char buffer[256];

	printf(L_CYAN"Successfully connected to the server, please start the conversation below"NONE"\n");
	while(1){
		bzero(buffer, 256);
		fgets(buffer, 255, stdin);
		buffer[strlen(buffer)-1]='\0';

		if (write(sockfd, buffer, strlen(buffer)) < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}
	}
}

int connect_server(int argc, char *argv[]){
	int sockfd;
	char server_port[20];
	char server_addr[20];
	char client_name[20];
	uint16_t portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// Get server address, port and client name
	if (argc < 3) {
		printf(YELLOW"please input the address of server: "NONE);
		fgets(server_addr, 20, stdin);
		server_addr[strlen(server_addr)-1]='\0';

		printf(YELLOW"please input the port of server: "NONE);
		fgets(server_port, 20, stdin);
		server_port[strlen(server_port)-1]='\0';

		printf(YELLOW"please input your name: "NONE);
		fgets(client_name, 20, stdin);
		client_name[strlen(client_name)-1]='\0';
	}
	else{
		strcpy(argv[1],server_addr);
		strcpy(argv[2],server_port);
		strcpy(argv[3],client_name);
	}

	// Create a socket point 
	portno = (uint16_t) atoi(server_port);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) { 
		perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(server_addr);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
	serv_addr.sin_port = htons(portno);

	// Now connect to the server
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}
	else{  // Successfully connected, send username
		write(sockfd, client_name, strlen(client_name)+1);
	}

	return sockfd;
}

int main(int argc, char *argv[]) {
	int sockfd;
	pthread_t recv, sendmsg;
	
	// Connect to the specified server
	sockfd =  connect_server(argc, argv);

	// Create a thread to receive messages
	pthread_create(&recv, NULL, recv_msg, (void *)&sockfd);

	// Send message to the server 
	pthread_create(&sendmsg, NULL, send_msg, (void *)&sockfd);

	pthread_join(sendmsg,NULL);
	return 0;
}