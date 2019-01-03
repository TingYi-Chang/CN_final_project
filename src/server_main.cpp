#include <functional>
#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common_app_protocol.hpp"

#define MAX_CLIENT 1024
#define STRING_MAX 4096
#define ID_MAX 64

using namespace std;

typedef struct{
	int  fd = 0;
	char id[ID_MAX] = "\0";
	char status = 'L';//L = login, R = register, M = main, C = chat, F = file
	char dest_id[ID_MAX] = "\0";
}User_log;

User_log 	client_log[MAX_CLIENT];


int server_send(int fd, int op, int data_len, char message[]){
	printf ("Fd = %d, Send op = %d, len = %d, mes = %s, meslen = %d\n",fd,op,data_len,message, strlen(message));
	op = htonl(op);	
	int tmp_data_len = htonl(data_len);
	send (fd,&op,sizeof(int),0);
	send (fd,&data_len,sizeof(int),0);
	send (fd,message,data_len,0);
	//printf ("a,b,c = %d %d %d\n",a,b,c);
	return 0;
}

int Recv_Mes(int client_num){
	int op, data_len;
	//client disconnected
	int test = 0;
	if ((test = recv(client_log[client_num].fd, &op, sizeof(int),0)) <= 0){
		printf ("test = %d\n",test);
		return -1;
	}
	if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0)
		return -1;
	op = ntohl(op);
	data_len = ntohl(data_len);

	char input[data_len+1];
	memset(input,'\0', sizeof(input));
	if (recv(client_log[client_num].fd, input, data_len, 0) <= 0)
		return -1;
	string message = input;
	printf ("strlen = %d\n",strlen(input));
	printf ("op = %d, datalen = %d, Message: %s\n",op,data_len,message.c_str());
	

	//login
	if (client_log[client_num].status == 'L'){
		//printf ("Login page.\n");
		if (op == APP_SIGNUP){
		//	printf ("Signup request\n");
			client_log[client_num].status = 'R';
			server_send(client_log[client_num].fd, APP_SIGNUP, 11, "Sign up ID:");
			return 1;
		}
		else if(op == APP_LOGIN){
			//check ID
			fstream file;
			string info_path = "server_data/";
			info_path = info_path + input + "/" + input + ".txt";
			file.open(info_path,ios::in);
			if (!file){
				server_send(client_log[client_num].fd, APP_ERROR, 19, "User doesn't exist.");
				return 0;
			}
			else if (file)
				server_send(client_log[client_num].fd, APP_LOGIN, 22,"Please enter password.");
			char tmpid[ID_MAX];
			strcpy(tmpid,input);
			//check password
			if (recv(client_log[client_num].fd, &op, sizeof(int),0) <= 0){
				file.close();
				return -1;
			}
			if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0){
				file.close();
				return -1;
			}
			op = ntohl(op);
			data_len = ntohl(data_len);
			memset(input,'\0',sizeof(input));
			if (recv(client_log[client_num].fd,input,data_len,0) <= 0){
				file.close();
				return -1;
			}
			if(op == APP_LOGIN){
				char buffer [STRING_MAX];
				char password [STRING_MAX];
				file.getline(buffer,sizeof(buffer),' ');
				if (strcmp(buffer,"hash:") != 0){
					printf ("ERROR, buffer = %s\n",buffer);
					file.close();
					return -2;
				}
				file.getline(password,sizeof(password),' ');
				if (strcmp(input,password) != 0){
					server_send(client_log[client_num].fd, APP_ERROR, 35,"Wrong Password, please login again.");
					file.close();
					return 0;
				}
				if (strcmp(input,password) == 0){
					server_send(client_log[client_num].fd, APP_MAIN, 8,"Welcome.");
					client_log[client_num].status = 'M';
					strcpy(client_log[client_num].id,tmpid);
					file.close();
					return 0;
				}
			}
		}
	}
	//Login end
	//reg
	else if (client_log[client_num].status == 'R'){
		if(op == APP_SIGNUP){
			char path[256] = "server_data/";
			char id[ID_MAX] = "\0";
			string info_path = "server_data/";
			info_path = info_path + input + "/" + input + ".txt";
			strcat(path,input);
			strcpy(id,input);
			fstream file;
			file.open(info_path,ios::in);//path should be added
			if (file.is_open() == 1){
				server_send(client_log[client_num].fd, APP_ERROR, 17,"ID already exist.");
				file.close();
				return 0;
			}
			else if (!file.is_open()){
				server_send(client_log[client_num].fd, APP_SIGNUP, 21,"Please enter password");
				op = 0;
				data_len = 0;
				if (recv(client_log[client_num].fd, &op, sizeof(int),0) <= 0)
					return -1;
				if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0)
					return -1;
				op = ntohl(op);
				data_len = ntohl(data_len);
				memset(input,'\0',sizeof(input));
				if (recv(client_log[client_num].fd,input,data_len,0) <= 0)
					return -1;
/////
	printf ("strlen = %d\n",strlen(input));
	printf ("op = %d, datalen = %d, Message: %s\n",op,data_len,input);
	

				if(op == APP_SIGNUP){
////
	printf ("Set reg path\n");
	printf ("path = %s\n",path);
					char mes_path[256] = "\0";
					char file_path[256] = "\0";
					strcpy(mes_path,path);
					strcat(mes_path,"/");
					strcat(mes_path,"message");
////
	printf ("mes_path = %s\n",mes_path);
					strcpy(file_path,path);
					strcat(file_path,"/");
					strcat(file_path,"file");
////
	printf ("file_path = %s\n",file_path);
	printf ("info_path = %s\n",info_path.c_str());
					int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					int status_mes = mkdir(mes_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					int status_file = mkdir(file_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					ofstream fout;
					fout.open(info_path,ios::out);
					printf ("status = %d, %d, %d\n",status,status_mes,status_file);
					if (status != 0 || status_mes != 0 || status_file != 0 || !fout){
						printf ("ERROR, cannot reg\n");
						return -2;
					}
					fout.write("hash: ",6);
					fout.write(input,strlen(input));
					fout.close();

					server_send(client_log[client_num].fd, APP_SIGNUP, 16,"Sign up success!");
					client_log[client_num].status = 'L';
					return 1;
				}
				else {
					printf ("ERROR, Wrong mes from client\n");
					return -2;
				}
			}
		}
	}
	//Reg end
	//Main
	else if (client_log[client_num].status == 'M'){
		if (op == APP_CHAT){
			DIR * dir = NULL;
			char dirpath[200] = "server_data/";
			strcat(dirpath,input);
			dir = opendir(dirpath);
			if (dir == NULL){
				server_send(client_log[client_num].fd, APP_ERROR, 19,"User doesn't exist.");
				return 0;
			}
			else {
				closedir(dir);
				ifstream fin;
				string path = "server_data/";
				path = path + client_log[client_num].id + "/message/" + input;
				fin.open(path,ifstream::in);
				if (!fin){
					server_send(client_log[client_num].fd, APP_CHAT, 4, "log ");
					return 0;
				}
				fin.seekg(0,ios::end);
				long long log_len = fin.tellg();
				char history[log_len];
				fin.read(history,log_len);
				char log [log_len + 10];
				strcpy(log,"log ");
				strcat(log,history);
				server_send(client_log[client_num].fd, APP_CHAT, strlen(log), log);
				client_log[client_num].status = 'C';
				strcpy(client_log[client_num].dest_id,input);
				fin.close();
				return 0;
			}
		}
		if (op == APP_FILE){
			DIR * dir = NULL;
			struct dirent * ptr = NULL;
			char path[256] = "server_data/";
			strcat(path,client_log[client_num].id);
			strcat(path,"/file");
			dir = opendir(path);
			if (dir == NULL){
				printf ("ERROR, no dir");
				return -2;
			}
			char output[STRING_MAX] = "\0";
			while((ptr = readdir(dir)) != NULL){
				if (strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0)
					continue;
				strcat(output,ptr->d_name);
				strcat(output,"\n");
			}
			closedir(dir);
			server_send(client_log[client_num].fd, APP_FILE, strlen(output), output);
			client_log[client_num].status = 'F';
			return 0;
		}
	}
	//Main end
	//Chat
	else if (client_log[client_num].status == 'C'){
		if (op == APP_CHAT){
			ofstream fout_dest;
			string path = "server_data/";
			path = path + client_log[client_num].dest_id + "/message/" + client_log[client_num].id;
			fout_dest.open(path,ios::app);
			string mes_to_dest = client_log[client_num].id;
			mes_to_dest = mes_to_dest + ": " + input + "\n";
			fout_dest << mes_to_dest;
			
			ofstream fout_id;
			string path1 = "server_data/";
			path1 = path1 + client_log[client_num].id + "/message/" + client_log[client_num].dest_id;
			fout_id.open(path1,ios::app);
			string mes_to_id = client_log[client_num].dest_id;
			mes_to_id = mes_to_id + ": " + input + "\n";
			fout_id << mes_to_id;
			fout_dest.close();
			fout_id.close();
	
			for (int i = 0; i < MAX_CLIENT; i++){
				if (strcmp(client_log[i].id,client_log[client_num].dest_id) == 0){
					char message[data_len + ID_MAX + 1];
					strcpy(message,client_log[client_num].id);
					strcat(message," ");
					strcat(message,input);
					server_send(client_log[i].fd, APP_CHAT, data_len, input);
				}
			}
			return 0;
		}
		else if (op == APP_MAIN){
			client_log[client_num].status = 'M';
			memset(client_log[client_num].dest_id,'\0',ID_MAX);
			server_send(client_log[client_num].fd, APP_MAIN, 5, "\back");
			return 0;
		}
		else 
			return -2;
	}
	//Chat end
	//File
	else if (client_log[client_num].status == 'F'){
		if (op == APP_FILE){
			if (strcmp(input,"download") == 0){
				server_send(client_log[client_num].fd, APP_FILE, 11,"Which file?");
				if (recv(client_log[client_num].fd, &op, sizeof(int),0) <= 0)
					return -1;
				if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0)
					return -1;
				op = ntohl(op);
				data_len = ntohl(data_len);
				memset(input,'\0',sizeof(input));
				if (recv(client_log[client_num].fd,input,data_len,0) <= 0)
					return -1;
				string file_path = "server_data/";
				file_path = file_path + client_log[client_num].id + "/file/" + input;
				ifstream fin;
				fin.open(file_path,ios::binary);
				fin.seekg(0,ios::end);
				long long log_len = fin.tellg();
				char history[log_len];
				fin.read(history,log_len);
				char log [log_len + 10];
				strcpy(log,"log ");
				strcat(log,history);
				server_send(client_log[client_num].fd, APP_FILE, strlen(log), log);
				fin.close();
				return 0;
			}
			else if (strcmp(input,"send") == 0){
				server_send(client_log[client_num].fd, APP_FILE, 11,"To who?");
				if (recv(client_log[client_num].fd, &op, sizeof(int),0) <= 0)
					return -1;
				if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0)
					return -1;
				op = ntohl(op);
				data_len = ntohl(data_len);
				memset(input,'\0',sizeof(input));
				if (recv(client_log[client_num].fd,input,data_len,0) <= 0)
					return -1;
				
				char *pch;
				pch = strtok(input," ");
				pch = strtok(NULL," ");
				char dest_id[ID_MAX];
				strcpy(dest_id,input);
				char filename[data_len];
				strcpy(filename,client_log[client_num].id);
				strcat(filename,"_");
				strcat(filename,pch);

				DIR * dir = NULL;
				char dir_path[256] = "server_data/";
				strcat(dir_path,dest_id);
				strcat(dir_path,"/file");
				dir = opendir(dir_path);
				if (dir == NULL){
					server_send(client_log[client_num].fd, APP_ERROR, 19,"User doesn't exist.");
					return 0;	
				}
				//printf ("Get file: %s\n",filename);
				else 
					server_send(client_log[client_num].fd, APP_FILE, 6,"Ready.");
				//receive file
				if (recv(client_log[client_num].fd, &op, sizeof(int),0) <= 0)
					return -1;
				if (recv(client_log[client_num].fd, &data_len, sizeof(int),0) <= 0)
					return -1;
				op = ntohl(op);
				data_len = ntohl(data_len);
				memset(input,'\0',sizeof(input));
				if (recv(client_log[client_num].fd,input,data_len,0) <= 0)
					return -1;
				
				
				string path = "server_data/";
				path = path + dest_id + "/message/" + filename; 
				ofstream fout;
				fout.open(path,ios::out|ios::trunc|ios::binary);
				if (!fout){
					printf ("ERROR, cannot open file.\n");
					return -2;
				}
				fout.write(input,data_len);
				fout.close();
				server_send(client_log[client_num].fd, APP_FILE, 8, "Done.");
				return 0;
			}
			else 
				return -2;
		}
		else if (op == APP_MAIN){
			client_log[client_num].status = 'M';
			server_send(client_log[client_num].fd, APP_MAIN, 5, "\back");
		}
		else 
			return -2;
	}
	//File end
	else {
		int tmpOp = APP_ERROR, tmpData_len = 14;
		tmpOp = htonl(tmpOp);
		tmpData_len = htonl(tmpData_len);
		send (client_log[client_num].fd,&tmpOp,sizeof(int),0);
		send (client_log[client_num].fd,&tmpData_len,sizeof(int),0);
		send (client_log[client_num].fd,"ERROR message.",14,0);
		printf ("ERROR message: %s\n",input);
		return -2;
	}
	//printf ("recv from[%s:%d]\n",inet_ntoa(client_info[i].sin_addr),ntohs(client_info[i].sin_port));
	//send(client_log[client_num][i].fd,message,strlen(message),0);
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
	if (::bind(server_fd, (struct sockaddr *)&info, sizeof(info)) < 0){
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
		struct timeval tmp_tv;
		memcpy(&tmp_tv, &tv, sizeof(tmp_tv));
		select (maxfd+1,&rdfds,NULL,NULL,&tmp_tv);
		
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
			if (FD_ISSET(client_log[i].fd,&rdfds)){//add thread
				printf ("Get message from fd %d\n",client_log[i].fd);
				int check = Recv_Mes(i);
				if (check == -1){
					printf ("client %d disconnect\n",client_log[i].fd);
					FD_CLR(client_log[i].fd,&master);
					close(client_log[i].fd);
					client_log[i].fd = 0;
					return 0;
				}
				if (check == -2)//server error
					return 0;
			}
		}
	}
	return 0;
}
