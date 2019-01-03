#include <cstring>
#include <iostream>
#include <unistd.h>
#include "client_connection.hpp"
#include "common_app_protocol.hpp"

//////////
#include <cstdio>
#include <errno.h>

Connection::Connection(std::string host_name, int port)
	: _is_connected(false)
	, _host_name(host_name)
	, _port(port)
	, _server_info(NULL)
	, _sockfd(-1)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(_host_name.c_str(), std::to_string(port).c_str(), &hints, &_server_info) != 0){
		_server_info = NULL;
		std::cerr << "Warning: Cannot get address infomation of server." << std::endl;
		return;
	}
	for(struct addrinfo *ptr = _server_info; ptr != NULL; ptr = ptr->ai_next){
		_sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(_sockfd == -1)
			continue;
		if(connect(_sockfd, ptr->ai_addr, ptr->ai_addrlen) == 0){
			_is_connected = true;
			break;
		}
		close(_sockfd);
		_sockfd = -1;
	}
}

/*
Connection::~Connection(){
	if(_server_info != NULL)
		freeaddrinfo(_server_info);
	close(_sockfd);
}
*/

bool Connection::try_to_reconnect(){
	if(_sockfd != -1){
		close(_sockfd);
		_sockfd = -1;
	}
	if(_server_info == NULL){
		std::cerr << "Warning: No address infomation of server." << std::endl;
		return false;
	}
	for(struct addrinfo *ptr = _server_info; ptr != NULL; ptr = ptr->ai_next){
		_sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(_sockfd == -1)
			continue;
		if(connect(_sockfd, ptr->ai_addr, ptr->ai_addrlen) == 0){
			_is_connected = true;
			return true;
		}
		close(_sockfd);
		_sockfd = -1;
	}
	return false;
}

bool Connection::try_to_send(int op, std::string &data){
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(_sockfd, &wfds);
	select(_sockfd+1, NULL, &wfds, NULL, NULL);
	if(!FD_ISSET(_sockfd, &wfds))
		return false;
	else{
		int buf_len = 2 * sizeof(int) + data.size();
		char *buf = (char *)malloc(buf_len);
		memset(buf, 0, buf_len);
		int tmp_op = htonl(op), tmp_data_len = htonl(data.size());
		memcpy(buf, &tmp_op, 4);
		memcpy(buf+4, &tmp_data_len, 4);
		memcpy(buf+8, data.c_str(), data.size());
		int ret = send(_sockfd, buf, buf_len, 0);
		free(buf);
		if(ret == -1) return false;
	}
	return true;
}

bool Connection::try_to_recv(int &op, std::string &data){
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_sockfd, &rfds);
	select(_sockfd+1, &rfds, NULL, NULL, NULL);
	if(!FD_ISSET(_sockfd, &rfds))
		return false;
	else{
		int tmp_op, tmp_data_len;
		int ret = recv(_sockfd, &tmp_op, 4, 0);
		if(ret == -1)
			return false;
		else if (ret == 0){
			close(_sockfd);
			_sockfd = -1;
			_is_connected = false;
			return false;
		}
		ret = recv(_sockfd, &tmp_data_len, 4, 0);
		if(ret == -1)
			return false;
		else if (ret == 0){
			close(_sockfd);
			_sockfd = -1;
			_is_connected = false;
			return false;
		}

		op = ntohl(tmp_op);
		tmp_data_len = ntohl(tmp_data_len);
		char *buf2 = (char *)malloc(tmp_data_len+1);
		memset(buf2, 0, tmp_data_len+1);
		ret = recv(_sockfd, buf2, tmp_data_len, 0);
		if(ret == -1) {
			free(buf2);
			return false;
		}
		else if (ret == 0){
			close(_sockfd);
			_sockfd = -1;
			_is_connected = false;
			free(buf2);
			return false;
		}
		data = std::string(buf2, tmp_data_len+1);
		free(buf2);
	}
	return true;
}

bool Connection::to_send(int op, std::string &data){
	int buf_len = 2 * sizeof(int) + data.size();
	char *buf = (char *)malloc(buf_len);
	memset(buf, 0, buf_len);
	int tmp_op = htonl(op), tmp_data_len = htonl(data.size());
	memcpy(buf, &tmp_op, 4);
	memcpy(buf+4, &tmp_data_len, 4);
	memcpy(buf+8, data.c_str(), data.size());
	int ret = send(_sockfd, buf, buf_len, 0);
/////////
	printf("send(op+len+data) fd %d, ret %d\n", _sockfd, ret);
	free(buf);
	if(ret == -1) return false;
	return true;
}

bool Connection::to_recv(int &op, std::string &data){
	int tmp_op, tmp_data_len;
	int ret = recv(_sockfd, &tmp_op, 4, 0);
/////////
	printf("recv(op) fd %d, ret %d, op %d\n", _sockfd, ret, tmp_op);

	if(ret == -1)
		return false;
	else if (ret == 0){
		close(_sockfd);
		_sockfd = -1;
		_is_connected = false;
		return false;
	}

	ret = recv(_sockfd, &tmp_data_len, 4, 0);
/////////
	printf("recv(len) fd %d, ret %d, len %d\n", _sockfd, ret, tmp_data_len);

	if(ret == -1)
		return false;
	else if (ret == 0){
		close(_sockfd);
		_sockfd = -1;
		_is_connected = false;
		return false;
	}
	
	op = ntohl(tmp_op);
	tmp_data_len = ntohl(tmp_data_len);
	char *buf2 = (char *)malloc(tmp_data_len+1);
	memset(buf2, 0, tmp_data_len+1);
	ret = recv(_sockfd, buf2, tmp_data_len, 0);
/////////
	printf("recv(data) fd %d, ret %d, data %s\n", _sockfd, ret, buf2);

	if(ret == -1) {
		free(buf2);
		return false;
	}
	else if (ret == 0){
		close(_sockfd);
		_sockfd = -1;
		_is_connected = false;
		free(buf2);
		return false;
	}
	data = std::string(buf2, tmp_data_len+1);
	free(buf2);
	return true;
}
