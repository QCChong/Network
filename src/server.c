#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "dns_protocol.h"
#include<ctype.h>

#define MAXLINE 256
#define UDPPORT 5300

char **getinfo(char *q_address){
	struct hostent *dns_server;
    dns_server  = gethostbyname(q_address);
    if(!dns_server){
    	perror("Get IP address error!");
        exit(1);
    }
    printf("|---------------------------------------|\n");
	printf("\tDomain: %s\n",dns_server->h_name );

	//Alias of host
    char **pptr;
    char str[INET_ADDRSTRLEN];
    for (pptr=dns_server->h_aliases; *pptr!=NULL; pptr++) {
        printf("\tTalias: %s\n\n", *pptr);
    }
    //ipv4 address
	switch (dns_server->h_addrtype) {
	    case AF_INET:
	        pptr = dns_server->h_addr_list;
	        for (; *pptr!=NULL;pptr++) {
	            printf("\tIP: %s\n",
	                    inet_ntop(dns_server->h_addrtype,*pptr, str, sizeof(str)));
	        }
	        break;
	    default:
	        printf("unknown address type\n");
	        break;
	}
	return dns_server->h_addr_list;
}

void process_send(char *buffer,char *name,char **res_ip,int query_len){
    memset(buffer,0,MAXLINE);
	struct DNS_HEADER *dns = (struct DNS_HEADER*)buffer;
	unsigned char *qname = (unsigned char*)(buffer +
								sizeof(struct DNS_HEADER));
	strcpy((char*)qname,(char*)name);
	dns->qr = 1;
	
	//Add ip address to dns protocol
	unsigned char* reader = (unsigned char*)(buffer + query_len);

	int r_len = sizeof(struct R_DATA);
	struct R_DATA *r_data;
	int i=0;
	for (; *res_ip!=NULL;res_ip++,i++) {
		r_data = (struct R_DATA *)(reader+2);
		r_data->data_len = htons(4);
		r_data->type = htons(1);
		strcpy((char*)(reader + 2 + r_len), *res_ip);
		reader = reader + 2 + r_len + 4;
	}
	dns->ans_count = htons(i);	
}
void CHName(char *qname){
	int t = strlen(qname);
	for(int i=0;i<t;i++,qname++){
		if (!isalpha(*qname)){
			*qname = '.';
		}
	}
}
unsigned char *process_recv(char *buffer){
	unsigned char *qname = (unsigned char*)(buffer +
								sizeof(struct DNS_HEADER));
	CHName((char*)qname);
	qname++;
	return qname;
}

int main(int argc, char const *argv[])
{
	int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(server_addr);
    bzero(&server_addr, len);
    
    // Create a socket and bind it
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(UDPPORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	char recv_buf[MAXLINE],send_buf[MAXLINE];

	bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	int query_len ;
	while(1)
	{
		bzero(&recv_buf, sizeof(recv_buf));
		query_len =  recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, 
								(struct sockaddr *)&server_addr, &len);
		if (query_len<0){
			exit(1);
		}
		// Get IP address
		unsigned char *qname = process_recv(recv_buf);
		char **pptr = getinfo((char*)qname);
		process_send(send_buf,(char*)qname,pptr,query_len);

		if (sendto(sockfd, send_buf, sizeof(send_buf), 0,
								(struct sockaddr *)&server_addr, len) <0){
			exit(1);
		}
	}

	return 0;
}