#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "dns_protocol.h"
#include "dns_protocol.h"

#define MAXLINE 256

int  ChName(char *fname,char *tname)  
{  
    int j =0;  
    int i =strlen(fname)-1;  
    int k = i+1;  
    tname[i+2] = 0;  
    for (; i>=0;i--,k--)  {  
        if (fname[i] == '.')  {  
            tname[k] = j;  
            j=0;  
        }  
        else{  
            tname[k] = fname[i];  
            j++;  
        }  
    }  
    tname[k] = j;  
    return strlen(tname)+1;  
}  

int process_send(char *buffer,char *name){
    memset(buffer,0,MAXLINE);
     
	struct DNS_HEADER *dns = (struct DNS_HEADER*)buffer;
    dns->id = htons(1);
    dns->rd = 1;
    dns->q_count = htons(1);

    unsigned char *qname = (unsigned char*)(buffer + sizeof(struct DNS_HEADER));
    int namelen = ChName(name,(char*)qname);

    struct QUESTION *ques = (struct QUESTION *)(qname + namelen);
    ques-> qtype =  htons(1);
    ques-> qclass = htons(1);
    int send_len = sizeof(struct DNS_HEADER)+sizeof(struct QUESTION)+namelen; 
    return send_len;
}

void process_recv(char *buffer,int query_len){
    struct DNS_HEADER *dns = (struct DNS_HEADER*)buffer;//
    int ans_count = ntohs(dns->ans_count);

    struct R_DATA *r_data;
    char *p= buffer + query_len;

    // Parse the returned ipv4 address
    int r_len = sizeof(struct R_DATA);
	char str[INET_ADDRSTRLEN];

    for(int i=0;i<ans_count;i++){
        r_data = (struct R_DATA *)(p+2);
        if (ntohs(r_data->type)==1){  // type A
            printf("\tIP : %s\n",
                            inet_ntop(AF_INET,p + 2 + r_len, str, sizeof(str)));
        }
        p = p + 2 + r_len + ntohs(r_data->data_len);
    }
}


int main(int argc, char const *argv[])
{
	int sockfd,dns_port;
    struct sockaddr_in server_addr;
    char sen_buf[MAXLINE],recv_buf[MAXLINE];
    char dns_addr[30],domain[30];

    socklen_t len = sizeof(server_addr);
    bzero(&server_addr, len);

    if (argc < 3) {
        fprintf(stderr, "usage: %s DNS_server_ip domain\n", argv[0]);
        exit(0);
    }

    strcpy(dns_addr,argv[1]);
    strcpy(domain,argv[2]);
    if (strcmp(argv[1], "127.0.0.1") == 0){
        dns_port = 5300;
    }else{
        dns_port = 53;
    }
        
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(dns_port);
	inet_pton(AF_INET, dns_addr, &server_addr.sin_addr);

	char *s_addr=domain; //"www.baidu.com";

	int send_len = process_send(sen_buf,s_addr);
	if(sendto(sockfd, sen_buf, send_len, 0,
						(struct sockaddr *)&server_addr, len) < 0){
        exit(0);
    }
    
	bzero(&recv_buf, sizeof(recv_buf));
	if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, 
							(struct sockaddr *)&server_addr, &len) < 0){
        exit(0);
    }
    process_recv(recv_buf,send_len);

	return 0;
}
