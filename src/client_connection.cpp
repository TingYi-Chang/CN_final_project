#include <cstring>
#include <iostream>
#include <unistd.h>
#include "client_connection.hpp"
#include "common_app_protocol.hpp"

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

Connection::~Connection(){
	freeaddrinfo(_server_info);
	close(_sockfd);
}

bool Connection::try_to_reconnect(){
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

bool Connection::try_to_send(AppHeader &header, std::string &data){
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(_sockfd, &wfds);
	select(_sockfd+1, NULL, &wfds, NULL, NULL);
	if(!FD_ISSET(_sockfd, &wfds))
		return false;
	else{
		int buf_len = 2 * sizeof(int) + data.size() + 5;
		char *buf = (char *)malloc(buf_len);
		memset(buf, 0, buf_len);
		int tmp_op = htonl(header.op), tmp_data_len = htonl(header.data_len);
		memcpy(buf, &tmp_op, 4);
		memcpy(buf+4, &tmp_data_len, 4);
		memcpy(buf+8, data.c_str(), data.size());
		int ret = send(_sockfd, buf, buf_len, 0);
		free(buf);
		if(ret == -1) return false;
	}
	return true;
}

bool Connection::try_to_recv(AppHeader &header, std::string &data){
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_sockfd, &rfds);
	select(_sockfd+1, &rfds, NULL, NULL, NULL);
	if(!FD_ISSET(_sockfd, &rfds))
		return false;
	else{
		char buf[16];
		int ret = recv(_sockfd, buf, 8, 0);
		if(ret == -1)
			return false;
		else if (ret == 0){
			close(_sockfd);
			_sockfd = -1;
			_is_connected = false;
			return false;
		}
		memcpy(&header.op, buf, 4);
		memcpy(&header.data_len, buf+4, 4);
		header.op = ntohl(header.op);
		header.data_len = ntohl(header.data_len);
		char *buf2 = (char *)malloc(header.data_len);
		ret = recv(_sockfd, buf2, header.data_len, 0);
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
		data = std::string(buf2, header.data_len);
		free(buf2);
	}
	return true;
}

