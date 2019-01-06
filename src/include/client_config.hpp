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
	std::string ID() {return _ID;}
	void set_ID(std::string s) {_ID = s;}
	std::string chatID() {return _chatID;}
	void set_chatID(std::string s) {_chatID = s;}

private:
	std::string _host_name;
	int _port;
	std::string _ID;
	std::string _chatID;
};

#endif