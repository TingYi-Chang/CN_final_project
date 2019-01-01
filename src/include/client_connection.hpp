#ifndef _CLIENT_CONNECTION_HPP_
#define _CLIENT_CONNECTION_HPP_

#include <string>
#include <netdb.h>
#include "common_app_protocol.hpp"

class Connection{
public:
	Connection()
		: _is_connected(false)
		, _host_name("")
		, _port(-1)
		, _server_info(NULL)
		, _sockfd(-1)
		{}
	Connection(std::string host_name, int port);
	~Connection();
	bool try_to_reconnect();
	bool try_to_send(AppHeader &header, std::string &data);
	bool try_to_recv(AppHeader &header, std::string &data);

private:
	bool _is_connected;
	std::string _host_name;
	int _port;
	struct addrinfo *_server_info;
	int _sockfd;
};

#endif