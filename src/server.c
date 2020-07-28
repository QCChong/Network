#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "color.h"
#include <sys/syscall.h>
#define THREAD_NUMBER 5      

typedef struct _client_user
{
	char name[20];
	int client_socket;
}client_user;

typedef struct 
{
	int hour;
	int min;
	char name[20];
	char content[256];
}message;

client_user users[THREAD_NUMBER];
 
int creat_listen(){
	// Bind the host, port and return the socket.
	int sockfd;
	uint16_t portno;   // server port
	struct sockaddr_in serv_addr;   //server address

	// First call to socket() function
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	// Initialize socket structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5001;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);


	// Now bind the host address using bind()
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	// Start listening for the clients

	if (listen(sockfd, THREAD_NUMBER) < 0){ 
		perror("ERROR on listening");
	} 
	else{
		printf("-----------------"L_GREEN"Start listening on port 5001"NONE"-----------------\n"); 
	}
	return sockfd;
}

client_user wait_client(int listen_sockfd){
	/* Establish a connection with the client, 
	   and return the client information (user name, socket) */
	int newsockfd;
	struct sockaddr_in cli_addr;
	client_user user;
	unsigned int clilen;

	// Accept actual connection from the client 
	clilen = sizeof(cli_addr);
	newsockfd = accept(listen_sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) {
		perror("ERROR on accept");
		exit(1);
	}
	else{  // Successfully connected, recieve username
		user.client_socket = newsockfd;
		read(newsockfd, user.name, sizeof(user.name));
		printf(L_CYAN"Successfully connected a client："L_PURPLE"%s (%s)"NONE"\n", user.name,inet_ntoa(cli_addr.sin_addr));		
	}
	return user;
}

void inform(message msg){
	/*  When multiple users connect to the server, 
		the server distributes client messages through broadcast */
	for(int i=0;i<THREAD_NUMBER;i++){
		// Broadcast to clients that have not sent messages
		if (strcmp(users[i].name ,msg.name)!=0 && users[i].client_socket){
			if (write(users[i].client_socket, (char * )&msg, sizeof(msg)) <0) {// send on Windows
				perror("ERROR writing to socket");
				exit(1);
			}
		}
	}
}

void *hanld_client(void *arg){
	char *name = ((client_user *)arg)->name;
	int client_socket = ((client_user *)arg)->client_socket;
	char buffer[256];
	time_t timep;
	struct tm *p;
	ssize_t res;

	// init the structure
	message msg;
	memset(&msg, 0, sizeof(message));
	strcpy(msg.name,name);

	// Start communicating 
	while(1){
		bzero(buffer, 256);
		res = read(client_socket, buffer, 255);
		if ( res < 0) { // recv on Windows
			perror("ERROR reading from socket");
			exit(1);
		}
		else if(res == 0){
			printf(L_RED"No information is obtained from the client(%s), the client may have been closed."NONE"\n",name);
			break;
		}
		else{
			time (&timep);
    		p=gmtime(&timep);
    		// Build information structure
    		msg.hour=8+p->tm_hour;
			msg.min=p->tm_min;
			strcpy(msg.content,buffer);
			printf(L_BLUE"{%02d:%02d} "L_PURPLE"[%s] "YELLOW"%s"NONE"\n",msg.hour, msg.min, msg.name, msg.content);
			inform(msg);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	int listen_socket = creat_listen();
	int thread_count = 0, res;
	client_user user;
	pthread_t thread[THREAD_NUMBER];
	while(1){
		// Listen to the client and add user
		user = wait_client(listen_socket); 
		strcpy(users[thread_count].name,user.name);
		users[thread_count].client_socket = user.client_socket;

		// Create thread to listen to new clients
		res = pthread_create(&thread[thread_count], NULL, hanld_client, &user);
		if (res != 0) {
            	printf(L_RED"Create thread %d failed"NONE"\n", thread_count);
                exit(1);
        }else{
        	thread_count++;
        }
	}
	
	close(listen_socket);
 
	return 0;
}