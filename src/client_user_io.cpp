#include "client_user_io.hpp"
#include <sys/select.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

/*
bool try_to_stdin(UserLine &line){
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	select(STDIN_FILENO+1, &rfds, NULL, NULL, NULL);
	if(!FD_ISSET(STDIN_FILENO, &rfds))
		return false;
	else{
		std::string input;
		std::getline(std::cin, input);
		if(input[0] != '\\'){
			line.is_command = false;
			line.topic = input;
			line.arg = std::vector<std::string>();
		}
		else{
			line.is_command = true;
			std::stringstream ss(input);
			std::string token;
			std::getline(ss, token, ' ');
			line.topic = token;
			line.arg = std::vector<std::string>();
			while(std::getline(ss, token, ' '))
				line.arg.push_back(token);
		}

	}
	return true;
}
*/

bool to_stdin(UserLine &line){
    std::string input;
    std::getline(std::cin, input);
    if(input[0] != '\\'){
        line.is_command = false;
        line.topic = input;
        line.arg = std::vector<std::string>();
    }
    else{
        line.is_command = true;
        std::stringstream ss(input);
        std::string token;
        std::getline(ss, token, ' ');
        line.topic = token;
        line.arg = std::vector<std::string>();
        while(std::getline(ss, token, ' '))
            line.arg.push_back(token);
    }
    return true;
}
