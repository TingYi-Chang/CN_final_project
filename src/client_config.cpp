#include "client_config.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>

Config::Config(int argc, char *argv[])
	: _host_name("")
	, _port(0)
{
	std::string config_path = "client_data/config/server_info.config";
	std::fstream server_info(config_path.c_str(), std::fstream::in);
	std::string buf1, buf2, buf3;
	server_info >> buf1 >> buf2 >> buf3;
	while(server_info.good()){
		if(buf2 != "="){
				std::cerr << "Client config corrupted.\n";
				return;
		}
		if(buf1 == "HOST_NAME"){
			_host_name = buf3;
		}
		else if(buf1 == "PORT"){
			_port = std::stoi(buf3);
		}
		else{
			std::cerr << "Client config corrupted.\n";
			return;
		}
		server_info >> buf1 >> buf2 >> buf3;
	}

////
	if(argc == 2)
		_port = atoi(argv[1]);
}