#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>


#define MAX_CLIENT 1024
#define STRING_MAX 1024
#define ID_MAX 64

using namespace std;

typedef struct{
	int  fd = 0;
	char id[ID_MAX] = "\0";
	char status = 'L';//L = login, R = register, M = main, C = chat, F = file
}User_log;

int Recv_Mes(User_log client_log){
	char input[STRING_MAX];
	memset(input,'\0',sizeof(input));
	//client disconnected
	if (recv(client_log.fd,input,sizeof(input),0) <= 0)
		return -1;
	string message = input;

	//login
	if (client_log.status == 'L'){
		if (strcmp(input,"reg") == 0){
			client_log.status = 'R';
			send (client_log.fd,"Reg ID:",7,0);
			return 1;
		}
		//check ID
		fstream file;
		file.open(input,ios::in);
		if (!file){
			send (client_log.fd,"User doesn't exist.",19,0);
			return 0;
		}
		//check password
		memset(input,'\0',sizeof(input));
		if (recv(client_log.fd,input,sizeof(input),0) <= 0)
			return -1;
		char buffer [STRING_MAX];
		char password [STRING_MAX];
		file.getline(buffer,sizeof(buffer),' ');
		if (strcmp(buffer,"hash:") != 0){
			printf ("ERROR, buffer = %s\n",buffer);
			return -2;
		}
		file.getline(password,sizeof(password),' ');
		if (strcmp(input,password) != 0){
			send(client_log.fd,"Wrong Password, please login again.",35,0);
			return 0;
		}
	}
	//Login end
	//reg
	else if (client_log.status == 'R'){
		fstream file;
		file.open(input,ios::in);
		if (file.is_open() == 1){
			send(client_log.fd,"ID already exist.",17,0);
			file.close();
			return 0;
		}
		else if (!file.is_open()){
			file.open(input,ios::out);
			send(client_log.fd,"Please enter password",21,0);
			memset(input,'\0',sizeof(input));
			if (recv(client_log.fd,input,sizeof(input),0) <= 0){
				file.close();
				return -1;
			}
			file.write("hash: ",6);
			file.write(input,strlen(input));
			send(client_log.fd,"Reg success!",12,0);
			client_log.status = 'L';
			return 1;
		}
	}
	//Reg end
	else {
		send (client_log.fd,"ERROR message.",14,0);
		printf ("ERROR message: %s\n",input);
		return -2;
	}
	//printf ("recv from[%s:%d]\n",inet_ntoa(client_info[i].sin_addr),ntohs(client_info[i].sin_port));
	//send(client_log[i].fd,message,strlen(message),0);
	return 0;
}

int main(int argc, char *argv[]){
	//open socket
	int server_fd = 0;
	int port = atoi(argv[1]);
	struct sockaddr_in info;
	
	server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_fd == 0){
		printf("Fail to create a socket.");
	}
	bzero(&info,sizeof(info));

	//set sock info
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(port);
	
	//bind
	if (bind(server_fd, (struct sockaddr *)&info, sizeof(info)) < 0){
		perror("bind failed");   
		exit(0);
	}
	
	if (listen(server_fd,MAX_CLIENT) < 0){
		perror("listen failed");
		exit(0);
	}
	//setup parameters that select() used
	struct timeval tv;
	fd_set rdfds;
	fd_set master;

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	FD_ZERO(&master);
	FD_SET(server_fd, &master);
	int maxfd = server_fd;

	User_log 	client_log[MAX_CLIENT];
	int	 		fd;
	int			client_num = 0;
	char 		input[STRING_MAX];
	struct		sockaddr_in client_info[MAX_CLIENT];
	int 		addrlen = sizeof(info);
	while (1)
	{
		printf ("looping\n");
		FD_ZERO(&rdfds);
		memcpy(&rdfds, &master, sizeof(rdfds));
		select (maxfd+1,&rdfds,NULL,NULL,&tv);
		
		//new client
		if (FD_ISSET(server_fd,&rdfds)){
			printf ("Get new client!\n");
			//find a available client_num
			for (int k = 0; k < MAX_CLIENT; k++)
				if (client_log[k].fd == 0)
					client_num = k;
			if ((client_log[client_num].fd = accept(server_fd,(struct sockaddr *)&client_info[client_num], (socklen_t*)&addrlen))<0){
				perror("accepted failed");
			}
			//printf ("Accepted\n");
			getpeername(server_fd, (struct sockaddr *)&client_info[client_num], (socklen_t*)sizeof(client_info[client_num]));
			FD_SET(client_log[client_num].fd,&master);
			maxfd = max(maxfd,client_log[client_num].fd);
			//set client log
			memset(client_log[client_num].id,'\0',ID_MAX);
			client_log[client_num].status = 'L';
		}

		//check client
		for (int i = 0; i < MAX_CLIENT; i++){
			if (FD_ISSET(client_log[i].fd,&rdfds)){
				int check = Recv_Mes(ref(client_log[i]));
				if (check == -1){
					FD_CLR(client_log[i].fd,&master);
					close(client_log[i].fd);
					client_log[i].fd = 0;
					continue;
				}
				
			}
		}
	}
	return 0;
}
