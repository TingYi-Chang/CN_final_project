#ifndef _CLIENT_USER_IO_HPP_
#define _CLIENT_USER_IO_HPP_

#include <vector>
#include <string>

typedef struct{
	bool is_command;
	std::string topic;
	std::vector<std::string> arg;
}UserLine;

bool try_to_stdin(UserLine &line);

#endif