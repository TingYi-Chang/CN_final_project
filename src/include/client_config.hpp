#ifndef _CLIENT_CONFIG_HPP_
#define _CLIENT_CONFIG_HPP_

#include <string>

class Config{
public:
	Config()
		: _host_name("")
		, _port(0){}
	Config(int argc, char *argv[]);
	std::string host_name() {return _host_name;}
	int port() {return _port;}

private:
	std::string _host_name;
	int _port;
};

#endif